#ifndef CANOPEN_NMT_H
#define CANOPEN_NMT_H

#include "OD.hpp"
#include "CAN.hpp"
#include <chrono>

class NMT{
private:
    CAN::CAN_Ptr can_;
    uint8_t addr_;

    void send_nmt_frame(uint8_t command){
        can_frame frame[1];
        frame[0].can_id = 0;
        frame[0].can_dlc = 2;
        frame[0].data[0] = command;
        frame[0].data[1] = addr_;
        frame[0].data[2] = 0;
        frame[0].data[3] = 0;
        frame[0].data[4] = 0;
        frame[0].data[5] = 0;
        frame[0].data[6] = 0;
        frame[0].data[7] = 0;
        can_->send(frame, 1);
        std::this_thread::sleep_for(std::chrono::microseconds(300));
    }

public:
    NMT(decltype(can_) can, decltype(addr_) addr)
        : can_(can), addr_(addr)
    {
        //
    }

    void init_node()
    {
        send_nmt_frame(0x81);
        send_nmt_frame(0x80);
    }

    void start_node()
    {
        send_nmt_frame(0x01);
    }
};

#endif