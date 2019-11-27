#ifndef CANOPEN_PDO_H
#define CANOPEN_PDO_H

#include <iostream>
#include <atomic>           // for atomic_bool
#include <unordered_map>    // for unordered_map
#include <memory>           // for shared_ptr
#include <deque>
#include <optional>
#include <linux/can.h>
#include "SDO.hpp"
#include "OD.hpp"

class SYNC;

typedef struct PdoStruct
{
    int64_t value;
    uint8_t byte_size;
    uint8_t od_type;
    bool is_signed;
    PdoStruct(decltype(value) v, decltype(byte_size) s, decltype(is_signed) b)
        : value(v), byte_size(s), is_signed(b)
    {}
    // TODO: need function to execute : value = (TYPE)value;
    void convert()
    {
        if(is_signed)
        {
            switch(byte_size)
            {
                case 1: value = (int8_t)value;break;
                case 2: value = (int16_t)value;break;
                case 4: value = (int32_t)value;break;
                default: std::cerr<<"except PDO value convert"<<std::endl;break;
            }
        }
    }
}PdoStruct;
using Pdo_Ele = std::shared_ptr<PdoStruct>;

/**
 *  A class for RPDO1 RPDO2 RPDO3 RPDO4
 *              TPDO1 TPDO2 TPDO3 TPDO4
 */ 
class PDO_X
{
    friend class SYNC;
private:
    uint32_t index_;
    uint32_t can_id_;
    uint8_t size_byte_left_;
    std::deque<Pdo_Ele> elements_;
    
public:
    // TODO: use it
    std::atomic_bool need_update;
    PDO_X(decltype(index_)index, decltype(can_id_) can_id, decltype(size_byte_left_) size=8)
        : index_(index), can_id_(can_id), size_byte_left_(size), need_update(true)
    {
        //
    }

    std::optional<Pdo_Ele> add(decltype(size_byte_left_) size, bool is_signed)
    {
        // std::cout<<"left:"<<std::to_string(size_byte_left_)<<":"<<std::to_string(size)<<std::endl;
        if(size <= size_byte_left_)
        {
            Pdo_Ele ele = std::make_shared<PdoStruct>(0, size, is_signed);
            elements_.push_back(ele);
            size_byte_left_ -= size;
            return std::optional<Pdo_Ele>(ele);
        }
        else return std::nullopt;
    }

    /**     get this pdo contain the number of object
     */ 
    uint8_t nums()
    {
        return elements_.size();
    }

    uint32_t index()
    {
        return index_;
    }
};

class PDO
{
    friend class SYNC;
private:
    std::deque<PDO_X> pdos_;
    SDO& sdo_;
    uint8_t addr_;
    OD::OD_Ptr od_;
public:
    PDO(decltype(sdo_) sdo, decltype(addr_) addr, decltype(od_) od, uint32_t begin_index, uint32_t begin_can_id)
        : sdo_(sdo), addr_(addr), od_(od)
    {
        for(int i=0; i<4; i++)
        {
            pdos_.emplace_back(begin_index + i, begin_can_id + (i*0x100) + addr_);
        }
    }

    void disenable()
    {
        for(auto& x : pdos_)
        {
            sdo_(x.index(), 0).write(0);
        }
    }

    void enable()
    {
        for(auto& x : pdos_)
        {
            if(x.nums())
                sdo_(x.index(), 0).write(x.nums());
        }
    }

    /**
     *      search left space in pdos, if found, add it and send relate CAN
     *      frame command.
     */ 
    std::optional<Pdo_Ele> search_space_and_send_frame(uint32_t obj, uint8_t size_byte, bool is_signed)
    {
        // std::cout<<std::hex<<std::uppercase<<obj<<std::dec<<std::endl;
        for(auto& x : pdos_)
        {
            if(auto result = x.add(size_byte, is_signed))
            {
                sdo_(x.index(), x.nums()).write(obj);
                return result;
            }
        }
        return std::nullopt;
    }

    auto add(uint32_t obj)
    {
        /**
         *      0020 -> four byte
         *      0010 -> two byte
         *      0008 -> one byte
         */
        std::optional<Pdo_Ele> result;
        bool is_signed = od_->get_signed(obj);
        switch(od_->get_byte(obj))
        {
            case 1: /*one byte*/ result = search_space_and_send_frame((obj<<16)+0x08, 1, is_signed);break;
            case 2: /*one byte*/ result = search_space_and_send_frame((obj<<16)+0x10, 2, is_signed);break;
            case 4: /*one byte*/ result = search_space_and_send_frame((obj<<16)+0x20, 4, is_signed);break;
            default: result = std::nullopt;break;
        }
        return result;
    }
};

#endif