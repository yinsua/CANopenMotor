#ifndef HARDWARE_INTERFACE_H
#define HARDWARE_INTERFACE_H

#include <memory>       //for shared_ptr
#include <linux/can.h>  //for can_frame
#include <array>        //for array

class HardwareInterface
{
public:
    template <typename T>
    bool init(const T&){}
    template <int N>
    uint32_t send(const std::array<can_frame, N>&){};
    template <int N>
    uint32_t recv(std::array<can_frame, N>&){};
};

#endif