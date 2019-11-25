#include <iostream>
#include <thread>
#include <future>
#include <chrono>

#define PRINT_DEBUG

#include "ZLG_USBCAN2/driver.hpp"
#include "stack/OD.hpp"
#include "stack/CiA402.hpp"
#include "stack/CAN.hpp"
#include "stack/SDO.hpp"

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

void can_test(){
    DeviceStruct device({4, 0, 0, 0x1400, 0, 0, 3, 1000, 100, 64});
    auto driver = std::make_shared<Driver>(device);
    auto can = std::make_shared<CAN>(driver);
    auto can1 = std::async(std::launch::async, CAN::run, can);
}

void sdo_test(){
    //
}

void cia402_test(){
    DeviceStruct device({4, 0, 0, 0x1400, 0, 0, 3, 1000, 100, 64});
    auto driver = std::make_shared<Driver>(device);
    auto can = std::make_shared<CAN>(driver);
    auto od = std::make_shared<OD>("/home/waterjet/iPOS.v1.09.eds");
    CiA402 motor0(can, 0, od);
}

int main(int argc, char** argv){
#if 0
    // can_init_test();
    // od_test();
    // can_test();
    // cia402_test();
#else
    DeviceStruct device({4, 0, 0, 0x1400, 0, 0, 3, 1000, 100, 64});
    auto driver = std::make_shared<Driver>(device);
    auto can0 = std::make_shared<CAN>(driver);
    auto can0_thread = std::async(std::launch::async, CAN::run, can0);
    auto od = std::make_shared<OD>("/home/waterjet/iPOS.v1.09.eds");
    CiA402 motor0(can0, 1, od);

    auto a = motor0.sdo(0x1400,1).read();
    std::cout<<std::to_string(a)<<std::endl;

    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<"^"<<std::flush;
    }
#endif
    return 0;
}