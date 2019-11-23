#ifndef CANOPEN_CIA402_H
#define CANOPEN_CIA402_H

#include "CANopen.hpp"

class CiA402 : public CANopen{
public:
    CiA402(const std::shared_ptr<HardwareInterface>& device, uint8_t addr, const std::shared_ptr<OD>& od)
        : CANopen(device, addr, od)
    {
        //
    }
};

#endif
