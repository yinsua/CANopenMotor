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
    std::shared_ptr<HardwareInterface> device_;
    std::shared_ptr<OD> od_;
    uint8_t addr_;
public:
    CANopen(const std::shared_ptr<HardwareInterface>& device, uint8_t addr, const std::shared_ptr<OD>& od)
        : device_(device), od_(od), addr_(addr), sdo(device, od, addr)
    {
        od_->init();
    }

    SDO sdo;
};

#endif