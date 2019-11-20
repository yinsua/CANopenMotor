#ifndef ZLG_USBCAN2_CAN
#define ZLG_USBCAN2_CAN

#include "../hardware_interface.hpp"
#include "controlcan.h"
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

class CAN : public HardwareInterface
{
private:
    bool device_opened_;
    DeviceStruct device_;

public:
    CAN()
        : device_opened_(false)
    {}

    ~CAN(){
        if(device_opened_)
            VCI_CloseDevice(device_.DeviceType, device_.DeviceIndex);
    }

    template <typename T>
    bool init(const T& device)
    {
        if(std::is_same<T, DeviceStruct>::value){
            auto dev = static_cast<DeviceStruct>(device);
            device_ = dev;
            // open device
            if(VCI_OpenDevice(dev.DeviceType, dev.DeviceIndex, 0) != STATUS_OK){
                return false;
            }
            device_opened_ = true;

            VCI_INIT_CONFIG config;
            config.AccCode = 0;
            config.AccMask = 0xffffffff;
            config.Filter = 1;
            config.Mode = 0;
            config.Timing0 = dev.CanBaudrate & 0xff;
            config.Timing1 = dev.CanBaudrate >> 8;
            if(VCI_InitCAN(dev.DeviceType, dev.DeviceIndex, dev.CanChannel, &config) != STATUS_OK){
                std::cerr<<"Init can failed"<<std::endl;
                return false;
            }
            if(VCI_ClearBuffer(dev.DeviceType, dev.DeviceIndex, dev.CanChannel) != STATUS_OK){
                std::cerr<<"Clear can failed"<<std::endl;
                return false;
            }
            if(VCI_StartCAN(dev.DeviceType, dev.DeviceIndex, dev.CanChannel) != STATUS_OK){
                std::cerr<<"Start can failed"<<std::endl;
                return false;
            }
            return true;
        }
        else return false;
    }

    template <int N>
    uint32_t send(const std::array<can_frame, N>& frames)
    {
        if(device_opened_) return 0;
        if(frames.size() == 0) return 0;
        const uint32_t size = frames.size();
        VCI_CAN_OBJ can[size];
        
        for(int i=0; i<size; i++){
            can[i].ID = frames.at(i).can_id;
            can[i].SendType = device_.TxType;
            can[i].DataLen = frames.at(i).can_dlc;
            can[i].Data[i] = frames.at(i).data[i];
            can[i].ExternFlag = 0;
            can[i].RemoteFlag = 0;
        }

        if(VCI_Transmit(device_.DeviceType, device_.DeviceIndex, device_.CanChannel, &can, size) != STATUS_OK){
            return 0;
        }
        return size;
    }

    template <int N>
    uint32_t recv(std::array<can_frame, N>& frames)
    {
        VCI_CAN_OBJ can[device_.RxMaxBuf];
        uint32_t recv_count = VCI_Receive(
            device_.DeviceType, device_.DeviceIndex, device_.CanChannel, 
            can, device_.RxMaxBuf, device_.RxTimeout);
        for(int i=0; i<recv_count; i++){
            frames.at(i).can_id = can[i].ID;
            frames.at(i).can_dlc = can[i].DataLen;
            for(int j=0; j<can[i].DataLen; j++)
                frames.at(i).data[j] = can[i].Data[j];
        }
        return recv_count;
    }
};

#endif