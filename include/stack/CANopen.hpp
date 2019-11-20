#ifndef CANOPEN_H
#define CANOPEN_H

#include "../hardware_interface.hpp"
#include <memory>   // for shard_ptr
#include "SDO.hpp"
#include "PDO.hpp"
#include "SYNC.hpp"
#include "NMT.hpp"

class CANopen{
private:
    std::shared_ptr<HardwareInterface> device_;
    uint8_t addr_;
public:
    CANopen(const std::shared_ptr<HardwareInterface>& device, uint8_t addr)
        : device_(device), addr_(addr), sdo(device)
    {

    }

    SDO sdo;
};

#endif