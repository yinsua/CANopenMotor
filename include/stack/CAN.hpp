#ifndef CANOPEN_CAN_H
#define CANOPEN_CAN_H

#include <thread>
#include "../hardware_interface.hpp"

class CAN : public std::thread {
private:
    std::shared_ptr<HardwareInterface> hw_interface_;
public:
    CAN(decltype(hw_interface_) hw_interface)
        : hw_interface_(hw_interface)
    {
        hw_interface_->init();
    }
};

#endif