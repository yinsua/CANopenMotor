#ifndef CANOPEN_CIA402_H
#define CANOPEN_CIA402_H

#include "CANopen.hpp"

class StateMachine
{
private:
    PDO& tpdo_;
    PDO& rpdo_;
    SYNC& sync_;

    void state_switch(uint16_t command, uint16_t dest_state, uint32_t max_wait_seconds=10)
    {
        auto func_start_timepoint = std::chrono::system_clock::now();
        while(1)
        {
            rpdo_[0x6040] = command;
            {
                std::unique_lock<std::mutex> ul(sync_.tpdos_sync_mutex);
                sync_.tpdos_sync_cv.wait_for(
                    ul,
                    std::chrono::seconds(1),
                    [&](){
                        return !(tpdo_[0x6041]&0xFF == dest_state);
                    }
                );
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if(
                std::chrono::system_clock::now() < 
                func_start_timepoint + 
                std::chrono::seconds(max_wait_seconds)
            )
            {
                std::cerr<<"state switch failed - "<<std::hex<<command<<std::dec<<std::endl;
                break;
            }
        }
    }

public:
    StateMachine(decltype(tpdo_) tpdo, decltype(rpdo_) rpdo, decltype(sync_) sync)
        : tpdo_(tpdo), rpdo_(rpdo), sync_(sync)
    {
        //
    }

    void ready_to_switch_on()
    {
        // 6 -> 31
        state_switch(0x06, 0x31);
    }

    void switch_on()
    {
        // 7 -> 33
        state_switch(0x07, 0x33);
    }

    void operation_enable()
    {
        // F -> 37
        state_switch(0x0F, 0x37);
    }
};

class CiA402 : public CANopen{
public:
    CiA402(CAN::CAN_Ptr can, uint8_t addr, OD::OD_Ptr od)
        : CANopen(can, addr, od),
          state(tpdo, rpdo, sync)
    {
        //
    }

    enum Mode:uint8_t{
        CSP=8
    };

    void set_operation(Mode m)
    {
        if(m == CSP)
        {
            sdo(0x6060).write(m);
            return;
        }
    }
    
    StateMachine state;
};

#endif
