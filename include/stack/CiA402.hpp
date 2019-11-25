#ifndef CANOPEN_CIA402_H
#define CANOPEN_CIA402_H

#include "CANopen.hpp"

class CiA402 : public CANopen{
public:
    CiA402(CAN::CAN_Ptr can, uint8_t addr, OD::OD_Ptr od)
        : CANopen(can, addr, od)
    {
        //
    }
};

#endif
