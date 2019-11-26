#ifndef CANOPEN_PDO_H
#define CANOPEN_PDO_H

#include <iostream>
#include <atomic>           // for atomic_bool
#include <unordered_map>    // for unordered_map
#include <memory>           // for shared_ptr
#include <deque>
#include "SDO.hpp"

class SYNC;

using Pdo_Ele = std::shared_ptr<int64_t>;

class RPDO
{
    friend class SYNC;
private:
    std::atomic_bool rpdo_need_update_;
    std::deque<Pdo_Ele> rpdos_;
    SDO& sdo_;
public:
    RPDO(decltype(sdo_) sdo)
        : sdo_(sdo)
    {
        //
    }

    void disenable()
    {
        sdo_(0x1600, 0).write(0);
        sdo_(0x1601, 0).write(0);
        sdo_(0x1602, 0).write(0);
        sdo_(0x1603, 0).write(0);
    }

    void enable()
    {
        sdo_(0x1600, 0).write(rpdos_.size());
    }

    Pdo_Ele add(uint32_t obj)
    {
        Pdo_Ele ele = std::make_shared<int64_t>(0);
        rpdos_.push_back(ele);
        return ele;
    }
};

class TPDO
{
    friend class SYNC;
private:
    std::unordered_map<uint32_t, Pdo_Ele> tpdos_;
    SDO& sdo_;
public:
    TPDO(decltype(sdo_) sdo)
        : sdo_(sdo)
    {
        //
    }

    void disenable()
    {
        sdo_(0x1A00, 0).write(0);
        sdo_(0x1A01, 0).write(0);
        sdo_(0x1A02, 0).write(0);
        sdo_(0x1A03, 0).write(0);
    }

    void enable()
    {
        sdo_(0x1A00, 0).write(tpdos_.size());
    }

    Pdo_Ele add(uint32_t obj)
    {
        Pdo_Ele ele = std::make_shared<int64_t>(0);
        tpdos_[obj] = ele;
        return ele;
    }
};

#endif