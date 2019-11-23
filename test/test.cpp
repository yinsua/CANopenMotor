#include <iostream>
#include "ZLG_USBCAN2/driver.hpp"
#include "stack/OD.hpp"
#include "stack/CiA402.hpp"

void can_init_test(){
    DeviceStruct device({4, 0, 0, 0x1400, 0, 0, 3, 1000, 100, 64});
    auto can = std::make_shared<Driver>(device);
    if(can->init())
        std::cout<<"init success"<<std::endl;
    else
        std::cerr<<"init failed"<<std::endl;
}

void od_test(){
    OD od("/home/waterjet/iPOS.v1.09.eds");
    od.init();
    std::cout<<std::to_string(od.get_type(0x1010, 0))<<std::endl;
}

void cia402_test(){
    DeviceStruct device({4, 0, 0, 0x1400, 0, 0, 3, 1000, 100, 64});
    auto can = std::make_shared<Driver>(device);
    auto od = std::make_shared<OD>("/home/waterjet/iPOS.v1.09.eds");
    CiA402 motor0(can, 0, od);
}

int main(int argc, char** argv){
    // can_init_test();
    // od_test();
    cia402_test();
    return 0;
}