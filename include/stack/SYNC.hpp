#ifndef CANOPEN_SYNC_H
#define CANOPEN_SYNC_H

#include "PDO.hpp"

class SYNC
{
private:
    RPDO& rpdo_;
    TPDO& tpdo_;
public:
    SYNC(decltype(rpdo_) rpdo, decltype(tpdo_) tpdo)
        : rpdo_(rpdo), tpdo_(tpdo)
    {
        //
    }

    void start(){
        //
    }
};

#endif