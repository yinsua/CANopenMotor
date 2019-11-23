#ifndef HARDWARE_INTERFACE_H
#define HARDWARE_INTERFACE_H

#include <memory>       //for shared_ptr
#include <linux/can.h>  //for can_frame
#include <array>        //for array

class HardwareInterface
{
public:
    virtual bool init()=0;
    virtual uint32_t send(const can_frame[], uint32_t)=0;
    virtual uint32_t recv(can_frame[])=0;
};

#endif