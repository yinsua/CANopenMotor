#ifndef CANOPEN_H
#define CANOPEN_H

#include "CAN.hpp"
#include <memory>   // for shard_ptr
#include "SDO.hpp"
#include "PDO.hpp"
#include "OD.hpp"
#include "SYNC.hpp"
#include "NMT.hpp"

class CANopen{
protected:
    CAN::CAN_Ptr can_;
    std::shared_ptr<OD> od_;
    uint8_t addr_;
public:
    CANopen(decltype(can_) can, uint8_t addr, OD::OD_Ptr od)
        : can_(can), od_(od), addr_(addr), 
          sdo(can, od, addr),
          nmt(can, addr),
          rpdo(sdo, addr, od, 0x1600, 0x200),
          tpdo(sdo, addr, od, 0x1A00, 0x180),
          sync(can, rpdo, tpdo)
    {
        od_->init();
    }

    SDO sdo;
    NMT nmt;

    void set_interpolation_time(/*default set to 10ms*/)
    {
        sdo(0x60C2, 1).write(10);
        sdo(0x60C2, 2).write(-3);
    }

    PDO rpdo;
    PDO tpdo;

    SYNC sync;
};

#endif