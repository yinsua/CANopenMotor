#ifndef CANOPEN_SDO_H
#define CANOPEN_SDO_H

#include "CAN.hpp"
#include "OD.hpp"
#include <memory>   // for shard_ptr

class SDO{
private:
    std::shared_ptr<HardwareInterface> device_;
    std::shared_ptr<OD> od_;
    uint8_t addr_;
    uint32_t current_index_;
    int current_sub_index_;
public:
    SDO(const std::shared_ptr<HardwareInterface>& device, const std::shared_ptr<OD>& od, uint8_t addr)
        : device_(device), od_(od), addr_(addr)
    {}

    SDO operator ()(decltype(current_index_) index, decltype(current_sub_index_) sub_index=-1){
        current_index_ = index;
        current_sub_index_ = sub_index;
        return *this;
    }

    template <typename T>
    T read(){
        can_frame frame[1];
        frame[0].can_id = 0x600+addr_;
        frame[0].can_dlc = 8;
        frame[0].data[0] = 0x40;
        frame[0].data[1] = current_index_ & 0xFF;
        frame[0].data[2] = (current_index_ & 0xFF00) >> 8;
        frame[0].data[3] = current_sub_index_ < 0 ? 0 : current_sub_index_;
        frame[0].data[4] = 0;
        frame[0].data[5] = 0;
        frame[0].data[6] = 0;
        frame[0].data[7] = 0;
        device_->send(frame, 1);
    }

    template <typename T>
    bool write(const T&){
        return true;
    }
};

#endif