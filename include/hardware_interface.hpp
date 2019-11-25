#ifndef HARDWARE_INTERFACE_H
#define HARDWARE_INTERFACE_H

#include <linux/can.h>  //for can_frame

class HardwareInterface
{
public:
    enum {MAX_FRAMES = 1024};
    virtual bool init()=0;
    virtual uint32_t send(const can_frame[], uint32_t)=0;
    virtual uint32_t recv(can_frame[])=0;
};

#endif