#include <iostream>
#include "ZLG_USBCAN2/can.hpp"

int main(int argc, char** argv){
    DeviceStruct device({4, 0, 0, 0x1400, 0, 0, 3, 1000, 100, 64});
    auto can = std::make_shared<CAN>();
    if(can->init(device))
        std::cout<<"init success"<<std::endl;
    else
        std::cerr<<"init failed"<<std::endl;
    return 0;
}