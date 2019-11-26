#ifndef CANOPEN_SDO_H
#define CANOPEN_SDO_H

#include "CAN.hpp"
#include "OD.hpp"
#include <memory>   // for shard_ptr
#include <mutex>    // for mutex
#include <condition_variable>   // for condition variable
#include <variant>
#include <map>
#include <string>
#include <optional>

const static std::map<uint32_t, std::string> SdoAbortCodes = {
    {0x05030000, "Toggle bit not alternated"},
    {0x05040001, "Client/server command specifier not valid or unknown"},
    {0x06010000, "Unsupported access to an object"},
    {0x06020000, "Object does not exist in the object dictionary"},
    {0x06040041, "Object cannot be mapped to the PDO"},
    {0x06040042, "The number and length of the objects to be mapped would exceed PDO length"},
    {0x06040043, "General parameter incompatibility reason"},
    {0x06040047, "General internal incompatibility error in the device"},
    {0x06070010, "Data type does not match, length of service parameter does not match"},
    {0x06070012, "Data type does not match, length of service parameter too high"},
    {0x06070013, "Data type does not match, length of service parameter too low"},
    {0x06090011, "Sub-index does not exist"},
    {0x06090030, "Value range of parameter exceeded (only for write access)"},
    {0x06090031, "Value of parameter written too high"},
    {0x06090032, "Value of parameter written too low"},
    {0x08000000, "General error"},
    {0x08000020, "Data cannot be transferred or stored to the application"},
    {0x08000021, "Data cannot be transferred or stored to the application because of local control"},
    {0x08000022, "Data cannot be transferred or stored to the application because of the present device state"}
};

class SDO{
    using Variant_Type = std::variant<int8_t,int16_t,int32_t,uint8_t,uint16_t,uint32_t>;
private:
    CAN::CAN_Ptr can_;
    OD::OD_Ptr od_;
    uint8_t addr_;
    uint32_t current_index_;
    int8_t current_sub_index_;
    uint8_t od_t_;

    

    int64_t get_value4frame(const can_frame& frame, uint8_t od_t){
        /*
            ID [DLC] XX XX XX XX A B C D
        */
        int64_t a=0, b=0, c=0, d=0;
        switch(od_t){
            case 4:
            case 7: {d = (frame.data[7]<<24); c = (frame.data[6]<<16);}
            case 3:
            case 6: {b = (frame.data[5]<<8);}
            case 2:
            case 5: {a = (frame.data[4]<<0);}break;
            default: std::cerr<<"od value error"<<std::endl;break;
        }
        int64_t result=a+b+c+d;
        switch(od_t){
            case 2: result = (int8_t)result;break;
            case 3: result = (int16_t)result;break;
            case 4: result = (int32_t)result;break;
            default: break;
        }
        return result;
    }

public:
    SDO(decltype(can_) can, decltype(od_) od, decltype(addr_) addr)
        : can_(can), od_(od), addr_(addr)
    {}

    SDO operator ()(decltype(current_index_) index, decltype(current_sub_index_) sub_index=-1){
        od_t_ = od_->get_type(index, sub_index);
        current_index_ = index;
        current_sub_index_ = sub_index < 0 ? 0 : sub_index;
        return *this;
    }

    int64_t read(){
        can_frame frame[1];
        frame[0].can_id = 0x600+addr_;
        frame[0].can_dlc = 8;
        frame[0].data[0] = 0x40;
        frame[0].data[1] = current_index_ & 0xFF;
        frame[0].data[2] = (current_index_ & 0xFF00) >> 8;
        frame[0].data[3] = current_sub_index_;
        frame[0].data[4] = 0;
        frame[0].data[5] = 0;
        frame[0].data[6] = 0;
        frame[0].data[7] = 0;
        can_->send(frame, 1);

        can_frame recv_the_frame;
        recv_the_frame.can_dlc = 0;
        {
            std::unique_lock<std::mutex> ul(can_->sdo_mutex);
            can_->sdo_cv.wait_for(
                ul, 
                std::chrono::seconds(3), 
                [&](){
                    if(!can_->sdo_deque.empty()){
                        auto& frame = can_->sdo_deque.back();
                        if(frame.can_id == (0x580+addr_)
                        ){
                            recv_the_frame = frame;
                            can_->sdo_deque.pop_back();
                            return true;
                        }
                        return false;
                    }
                    return false;
                }
            );
        }
        if(recv_the_frame.data[0] == 0x80){
            std::cout<<"SDO read exception: "<<std::hex<<std::uppercase<<
                        "(0x"<<current_index_<<","<<std::to_string(current_sub_index_)<<") "<<
                        SdoAbortCodes.at(get_value4frame(recv_the_frame, 7))<<std::dec<<std::endl;
            return 0;
        }
        if(!recv_the_frame.can_dlc){
            std::cerr<<"sdo read timeout"<<std::endl;
            return 0;
        }
        // if( recv_the_frame.can_dlc == 8 &&
        //     recv_the_frame.data[1] == (current_index_ & 0xFF) &&
        //     recv_the_frame.data[2] == ((current_index_ & 0xFF00)>>8) &&
        //     recv_the_frame.data[3] == current_sub_index_)
        return get_value4frame(recv_the_frame, od_t_);
    }

    bool write(const uint64_t& data){
        can_frame frame[1];
        frame[0].can_id = 0x600+addr_;
        frame[0].can_dlc = 8;
        // frame[0].data[0] = 0x40;
        frame[0].data[1] = current_index_ & 0xFF;
        frame[0].data[2] = (current_index_ & 0xFF00) >> 8;
        frame[0].data[3] = current_sub_index_;
        frame[0].data[4] = (data & 0xFF) >> 0;
        frame[0].data[5] = (data & 0xFF00) >> 8;
        frame[0].data[6] = (data & 0xFF0000) >> 16;
        frame[0].data[7] = (data & 0xFF000000) >> 24;
        
        if(od_t_ == 2 || od_t_ == 5)//one byte
        {
            frame[0].data[0] = 0x2F;
            frame[0].data[5] = 0;
            frame[0].data[6] = 0;
            frame[0].data[7] = 0;
        }
        else if(od_t_ == 3 || od_t_ == 6)//two byte
        {
            frame[0].data[0] = 0x2B;
            frame[0].data[6] = 0;
            frame[0].data[7] = 0;
        }
        else if(od_t_ == 4 || od_t_ == 7)//four byte
        {
            frame[0].data[0] = 0x23;
        }
        can_->send(frame, 1);

        can_frame recv_the_frame;
        recv_the_frame.can_dlc = 0;
        {
            std::unique_lock<std::mutex> ul(can_->sdo_mutex);
            can_->sdo_cv.wait_for(
                ul, 
                std::chrono::seconds(1), 
                [&](){
                    auto& frame = can_->sdo_deque.back();
                    if(frame.can_id == (0x580+addr_) &&
                        frame.can_dlc == 8 
                    ){
                        recv_the_frame = frame;
                        can_->sdo_deque.pop_back();
                        return true;
                    }
                    return false;
                }
            );
        }
        if(!recv_the_frame.can_dlc){
            std::cout<<"SDO read timeout"<<std::endl;
            return false;
        }
        if(recv_the_frame.data[0] == 0x80){
            std::cout<<"SDO read exception: "<<std::hex<<std::uppercase<<
                        "(0x"<<current_index_<<","<<std::to_string(current_sub_index_)<<") "<<
                        SdoAbortCodes.at(get_value4frame(recv_the_frame, 7))<<std::dec<<std::endl;
            return false;
        }

        if( recv_the_frame.can_dlc && 
            recv_the_frame.data[0] == 0x60 &&
            recv_the_frame.data[1] == (current_index_ & 0xFF) &&
            recv_the_frame.data[2] == ((current_index_ & 0xFF00)>>8) &&
            recv_the_frame.data[3] == current_sub_index_
            ){
            return true;
        }
        return false;
    }
};

#endif