#ifndef ZLG_USBCAN2_H
#define ZLG_USBCAN2_H

#include "../hardware_interface.hpp"
#include "controlcan.h"
#include <iostream>
#include <type_traits>  // for std::is_same
#include <linux/can.h>  // for can_frame

typedef struct{
    DWORD DeviceType;
    DWORD DeviceIndex;
    DWORD CanChannel;
    DWORD CanBaudrate;
    DWORD TxType;
    DWORD TxSleep;
    DWORD TxFrames;
    DWORD TxCount;
    DWORD RxTimeout;
    DWORD RxMaxBuf;
}DeviceStruct;

class Driver : public HardwareInterface
{
private:
    bool device_opened_;
    DeviceStruct device_;

public:
    Driver(const DeviceStruct& device)
        : device_(device), device_opened_(false)
    {}

    ~Driver(){
        if(device_opened_)
            VCI_CloseDevice(device_.DeviceType, device_.DeviceIndex);
    }

    bool init() override
    {
            // open device
            if(VCI_OpenDevice(device_.DeviceType, device_.DeviceIndex, 0) != STATUS_OK){
                return false;
            }
            device_opened_ = true;

            VCI_INIT_CONFIG config;
            config.AccCode = 0;
            config.AccMask = 0xffffffff;
            config.Filter = 1;
            config.Mode = 0;
            config.Timing0 = device_.CanBaudrate & 0xff;
            config.Timing1 = device_.CanBaudrate >> 8;
            if(VCI_InitCAN(device_.DeviceType, device_.DeviceIndex, device_.CanChannel, &config) != STATUS_OK){
                std::cerr<<"Init can failed"<<std::endl;
                return false;
            }
            if(VCI_ClearBuffer(device_.DeviceType, device_.DeviceIndex, device_.CanChannel) != STATUS_OK){
                std::cerr<<"Clear can failed"<<std::endl;
                return false;
            }
            if(VCI_StartCAN(device_.DeviceType, device_.DeviceIndex, device_.CanChannel) != STATUS_OK){
                std::cerr<<"Start can failed"<<std::endl;
                return false;
            }
            std::cout<<"ZLG USBCAN2 init success"<<std::endl;
            return true;
    }

    uint32_t send(const can_frame frames[], uint32_t len) override
    {
        if(!device_opened_) return 0;
        if(len == 0) return 0;
        VCI_CAN_OBJ can[len];
        
        for(int i=0; i<len; i++){
            can[i].ID = frames[i].can_id;
            can[i].SendType = device_.TxType;
            can[i].DataLen = frames[i].can_dlc;
            can[i].Data[i] = frames[i].data[i];
            can[i].ExternFlag = 0;
            can[i].RemoteFlag = 0;
        }

        if(VCI_Transmit(device_.DeviceType, device_.DeviceIndex, device_.CanChannel, can, len) != STATUS_OK){
            return 0;
        }
        return len;
    }

    uint32_t recv(can_frame frames[]) override
    {
        VCI_CAN_OBJ can[device_.RxMaxBuf];
        uint32_t recv_count = VCI_Receive(
            device_.DeviceType, device_.DeviceIndex, device_.CanChannel, 
            can, device_.RxMaxBuf, device_.RxTimeout);
        for(int i=0; i<recv_count; i++){
            frames[i].can_id = can[i].ID;
            frames[i].can_dlc = can[i].DataLen;
            for(int j=0; j<can[i].DataLen; j++)
                frames[i].data[j] = can[i].Data[j];
        }
        return recv_count;
    }
};

#endif