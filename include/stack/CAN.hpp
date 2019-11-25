#ifndef CANOPEN_CAN_H
#define CANOPEN_CAN_H

#include <iostream>
#include <iomanip>              // for std::setw std::setfill
#include <chrono>               // for chrono::seconds chrono::microseconds
#include <mutex>                // for mutex
#include <deque>                // for deque
#include <condition_variable>   // for condition variable
#include <linux/can.h>          //for can_frame
#include "../hardware_interface.hpp"

class CAN{
private:
    std::shared_ptr<HardwareInterface> hw_interface_;
    bool ready_run_;
    bool need_run_;

public:
    std::mutex sdo_mutex;
    std::mutex pdo_mutex;
    std::condition_variable sdo_cv;
    std::condition_variable pdo_cv;
    std::deque<can_frame> sdo_deque;
    std::deque<can_frame> pdo_deque;
public:
    CAN(decltype(hw_interface_) hw_interface)
        : hw_interface_(hw_interface),
          ready_run_(false),
          need_run_(true)
    {
        if(!hw_interface_->init())
            std::cerr<<"Hardware init failed"<<std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        ready_run_ = true;
    }

    using CAN_Ptr = std::shared_ptr<CAN>;

    void print_can_frames(std::string prefix, const can_frame frames[], uint32_t len){
#ifdef PRINT_DEBUG
        for(int i=0; i<len; i++){
            std::cout<<prefix<<std::hex<<frames[i].can_id<<
                " ["<<int(frames[i].can_dlc)<<"] ";
            for(int j=0; j<8; j++){
                std::cout<<std::setw(2)<<std::setfill('0')<<int(frames[i].data[j])<<" ";
            }
            std::cout<<std::dec<<std::endl;
        }
#endif
    }

    static void run(CAN_Ptr can)
    {
        while(!can->ready_run_){
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        std::cout<<"Start Recv"<<std::endl;
        while(can->need_run_)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            can_frame recv_frames[HardwareInterface::MAX_FRAMES];
            uint32_t recv_count=0;
            if(recv_count = can->hw_interface_->recv(recv_frames)){
                for(int i=0; i<recv_count; i++){
                    if(recv_frames[i].can_id & 0x580 == 0x580){
                        // SDO response
                        {
                            std::lock_guard<std::mutex> lg(can->sdo_mutex);
                            can->sdo_deque.push_back(recv_frames[i]);
                        }
                        can->sdo_cv.notify_all();
                    }
                }
                can->print_can_frames("RECV ", recv_frames, recv_count);
            }
            else{   // no frame
                // std::cout<<"?"<<std::endl;
            }
        }
    }

    bool send(const can_frame frames[], uint32_t len){
        print_can_frames("SEND ", frames, len);
        uint32_t send_count = hw_interface_->send(frames, len);
        if(send_count == len)
            return true;
        return false;
    }
};

#endif