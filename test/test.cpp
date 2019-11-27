#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <optional>

#define PRINT_DEBUG

#include "ZLG_USBCAN2/driver.hpp"
#include "stack/OD.hpp"
#include "stack/CiA402.hpp"
#include "stack/CAN.hpp"
#include "stack/SDO.hpp"
#include "stack/PDO.hpp"
#include "stack/SYNC.hpp"
#include "stack/NMT.hpp"

void can_init_test(){
    DeviceStruct device({4, 0, 0, 0x1400, 0, 0, 3, 1000, 500, 64});
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
    int64_t a = 0xFD;
    uint8_t b = a;
    std::cout<<std::hex<<std::to_string(b)<<std::endl;
    int8_t c = a;
    std::cout<<std::to_string(c)<<std::endl;
    int64_t d = c;
    std::cout<<std::to_string(d)<<std::endl;
    return 0;
#endif

#if 0
    // can_init_test();
    // od_test();
    // can_test();
    // cia402_test();
#else
    DeviceStruct device({4, 0, 0, 0x1400, 0, 0, 3, 1000, 100, 64});
    auto driver = std::make_shared<Driver>(device);
    auto can0 = std::make_shared<CAN>(driver);
    // NOTE: this line must be put into main function !
    auto can0_thread = std::async(std::launch::async, CAN::run, can0);
    auto od = std::make_shared<OD>("/home/waterjet/iPOS.v1.09.eds");
    CiA402 motor0(can0, 1, od);
    motor0.nmt.init_node();
    motor0.nmt.start_node();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    motor0.set_interpolation_time();

    auto cout_hex_num = [](int64_t v)
    {std::cout<<std::hex<<std::uppercase<<v<<std::dec<<std::endl;};

    auto v = motor0.sdo(0x1400,1).read();
    cout_hex_num(v);

    Pdo_Ele ctl_word;
    Pdo_Ele target_pos;
    motor0.rpdo.disenable();
    // TODO: motor0.rpdo[0x6040]
    if( auto x = motor0.rpdo.add(0x6040))
        ctl_word = x.value();
    if( auto x = motor0.rpdo.add(0x607A))
        target_pos = x.value();
    motor0.rpdo.enable();

    Pdo_Ele act_status;
    Pdo_Ele act_pos;
    motor0.tpdo.disenable();
    if( auto x = motor0.tpdo.add(0x6041))
        act_status = x.value();
    if( auto x = motor0.tpdo.add(0x6064))
        act_pos = x.value();
    motor0.tpdo.enable();
    // motor0.sync.start();
    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<"^"<<std::flush;
    }
#endif
    return 0;
}