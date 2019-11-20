#ifndef CANOPEN_SDO_H
#define CANOPEN_SDO_H

#include "../hardware_interface.hpp"
#include <memory>   // for shard_ptr

class SDO{
private:
    std::shared_ptr<HardwareInterface> device_;
    uint32_t current_index_;
    uint8_t current_sub_index_;
public:
    SDO(const std::shared_ptr<HardwareInterface>& device)
        : device_(device)
    {}

    SDO operator ()(uint32_t index, uint8_t sub_index){
        current_index_ = index;
        current_sub_index_ = sub_index;
        return *this;
    }

    template <typename T>
    bool write(const T&){
        return true;
    }

    template <typename T>
    T read(){
        //
    }
};

#endif