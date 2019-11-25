#ifndef CANOPEN_SDO_H
#define CANOPEN_SDO_H

#include "CAN.hpp"
#include "OD.hpp"
#include <memory>   // for shard_ptr
#include <mutex>    // for mutex
#include <condition_variable>   // for condition variable
#include <variant>

class SDO{
    using Variant_Type = std::variant<int8_t,int16_t,int32_t,uint8_t,uint16_t,uint32_t>;
private:
    CAN::CAN_Ptr can_;
    OD::OD_Ptr od_;
    uint8_t addr_;
    uint32_t current_index_;
    int8_t current_sub_index_;
    uint8_t od_t_;
public:
    SDO(decltype(can_) can, decltype(od_) od, decltype(addr_) addr)
        : can_(can), od_(od), addr_(addr)
    {}

    SDO operator ()(decltype(current_index_) index, decltype(current_sub_index_) sub_index=-1){
        od_t_ = od_->get_type(index, sub_index);
        current_index_ = index;
        current_sub_index_ = sub_index < 0 ? 0 : sub_index;
        return *this;
    }

    int64_t read(){
        can_frame frame[1];
        frame[0].can_id = 0x600+addr_;
        frame[0].can_dlc = 8;
        frame[0].data[0] = 0x40;
        frame[0].data[1] = current_index_ & 0xFF;
        frame[0].data[2] = (current_index_ & 0xFF00) >> 8;
        frame[0].data[3] = current_sub_index_;
        frame[0].data[4] = 0;
        frame[0].data[5] = 0;
        frame[0].data[6] = 0;
        frame[0].data[7] = 0;
        can_->send(frame, 1);

        std::cout<<"10"<<std::endl;

        can_frame recv_the_frame;
        recv_the_frame.can_dlc = 0;
        std::cout<<"11"<<std::endl;
        {
            std::unique_lock<std::mutex> ul(can_->sdo_mutex);
            can_->sdo_cv.wait_for(
                ul, 
                std::chrono::seconds(3), 
                [&](){
                    if(!can_->sdo_deque.empty()){
                        auto& frame = can_->sdo_deque.back();
                        if(frame.can_id == (0x580+addr_)
                        ){
                            recv_the_frame = frame;
                            can_->sdo_deque.pop_back();
                            return true;
                        }
                        return false;
                    }
                    return false;
                }
            );
        }
        if(recv_the_frame.data[0] == 0x80){
            std::cerr<<"read sdo exception"<<std::endl;
            return 0;
        }
        // if( recv_the_frame.can_dlc == 8 &&
        //     recv_the_frame.data[1] == (current_index_ & 0xFF) &&
        //     recv_the_frame.data[2] == ((current_index_ & 0xFF00)>>8) &&
        //     recv_the_frame.data[3] == current_sub_index_)
        std::cout<<"12"<<std::endl;
        int64_t return_value;
        if(recv_the_frame.can_dlc){
            switch(recv_the_frame.data[0]){
                case 0x4F/*one byte*/: {
                    auto a = recv_the_frame.data[4];
                    if(od_t_ == 2)// INT8
                    {
                        return_value = (int8_t)a;
                    }
                    else if(od_t_ == 5)// UINT8
                    {
                        return_value = a;
                    }
                    else 
                        std::cerr<<"SDO ("<<current_index_<<" "<<current_sub_index_<<") type check failed"<<std::endl;
                }break;
                case 0x4B/*two byte*/: {
                    uint16_t a = recv_the_frame.data[5] * 256;
                    auto b = (uint16_t)(a+recv_the_frame.data[4]);
                    if(od_t_ == 3)// INT16
                    {
                        return_value = (int16_t)b;
                    }
                    else if(od_t_ == 6)//UINT16
                    {
                        return_value = b;
                    }
                    else
                        std::cerr<<"SDO ("<<current_index_<<" "<<current_sub_index_<<") type check failed"<<std::endl;
                }break;
                case 0x47/*three byte*/: break;
                case 0x43/*four byte*/: {
                    uint32_t a = recv_the_frame.data[7] << 24;
                    uint32_t b = recv_the_frame.data[6] << 16;
                    uint32_t c = recv_the_frame.data[5] << 8;
                    uint32_t d = recv_the_frame.data[4];
                    uint32_t e = a+b+c+d;
                    if(od_t_ == 4)//INT32
                    {
                        return_value = (int32_t)e;
                    }
                    else if(od_t_ == 7)//UINT32
                    {
                        return_value = e;
                    }
                    else
                        std::cerr<<"SDO ("<<current_index_<<" "<<current_sub_index_<<") type check failed"<<std::endl;
                }break;
                case 0x80/*exception*/:
                    std::cerr<<"read sdo ("<<current_index_<<" "<<current_sub_index_<<") exception"<<std::endl;
                    break;
                default: std::cerr<<"SDO ("<<current_index_<<" "<<current_sub_index_<<
                    ") byte size error"<<std::endl;break;
            }
        }
    }

    bool write(const uint64_t& data){
        can_frame frame[1];
        frame[0].can_id = 0x600+addr_;
        frame[0].can_dlc = 8;
        // frame[0].data[0] = 0x40;
        frame[0].data[1] = current_index_ & 0xFF;
        frame[0].data[2] = (current_index_ & 0xFF00) >> 8;
        frame[0].data[3] = current_sub_index_;
        frame[0].data[4] = (data & 0xFF) >> 0;
        frame[0].data[5] = (data & 0xFF00) >> 8;
        frame[0].data[6] = (data & 0xFF0000) >> 16;
        frame[0].data[7] = (data & 0xFF000000) >> 24;
        
        if(od_t_ == 2 || od_t_ == 5)//int8
        {
            frame[0].data[0] = 0x2F;
            frame[0].data[5] = 0;
            frame[0].data[6] = 0;
            frame[0].data[7] = 0;
        }
        else if(od_t_ == 3 || od_t_ == 6)//int16
        {
            frame[0].data[0] = 0x2B;
            frame[0].data[6] = 0;
            frame[0].data[7] = 0;
        }
        else if(od_t_ == 4 || od_t_ == 7)//int32
        {
            frame[0].data[0] = 0x23;
        }
        can_->send(frame, 1);

        can_frame recv_the_frame;
        recv_the_frame.can_dlc = 0;
        {
            std::unique_lock<std::mutex> ul(can_->sdo_mutex);
            can_->sdo_cv.wait_for(
                ul, 
                std::chrono::seconds(3), 
                [&](){
                    auto& frame = can_->sdo_deque.back();
                    if(frame.can_id == (0x580+addr_) &&
                        frame.can_dlc == 8 &&
                        frame.data[1] == (current_index_ & 0xFF) &&
                        frame.data[2] == ((current_index_ & 0xFF00)>>8) &&
                        frame.data[3] == current_sub_index_
                    ){
                        if(frame.data[1] == 0x60)
                            recv_the_frame = frame;
                        else if(frame.data[1] == 0x80)
                            std::cerr<<"write sdo ("<<current_index_<<" "<<current_sub_index_<<") exception"<<std::endl;
                        else
                            std::cerr<<"receive unknown sdo data while write sdo"<<std::endl;
                        can_->sdo_deque.pop_back();
                        return true;
                    }
                    return false;
                }
            );
        }

        if(recv_the_frame.can_dlc){
            return true;
        }
        return false;
    }
};

#endif