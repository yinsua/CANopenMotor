#ifndef CANOPEN_SYNC_H
#define CANOPEN_SYNC_H

#include "CAN.hpp"
#include "PDO.hpp"
#include <pthread.h>
#include <atomic>

void* sync_thread(void*);

class SYNC
{
    friend void* sync_thread(void*);
private:
    CAN::CAN_Ptr can_;
    PDO& rpdo_;
    PDO& tpdo_;
    pthread_t thread_;
    std::atomic_bool thread_need_run_;
public:
    SYNC(decltype(can_) can, decltype(rpdo_) rpdo, decltype(tpdo_) tpdo)
        : can_(can), rpdo_(rpdo), tpdo_(tpdo), thread_need_run_(true)
    {
    }

    ~SYNC()
    {
        thread_need_run_.store(false);
    }

    void start(){
        pthread_create(&thread_, NULL, sync_thread, this);
    }

private:
    void rpdos_scan_and_send()
    {
        for(auto& x : rpdo_.pdos_)
        {
            if(x.need_update.load() && x.nums())
            {
                can_frame frame[1];
                frame[0].can_id = x.can_id_;
                frame[0].can_dlc = 8;
                uint8_t _index = 0;
                uint8_t total_byte = 0;
                for(auto& y : x.elements_)
                {
                    total_byte += y->byte_size;
                    for(int i=0; i<y->byte_size; i++)
                    {
                        frame[0].data[_index] = (y->value & (0xFF << i*8) ) >> (i*8);
                        ++_index;
                    }
                }
                for(int i=0; i<(8-total_byte); i++)
                {
                    frame[0].data[7-i] = 0;
                }
                can_->send(frame, 1);
            }
        }
    }

    void tpdos_scan_and_assign()
    {
        {
            std::unique_lock<std::mutex> ul(can_->pdo_mutex);
            if(!can_->pdo_deque.empty())
            {
                for(auto& frame : can_->pdo_deque)
                {
                    for(auto& tpdo : tpdo_.pdos_)
                    {
                        if(frame.can_id == tpdo.can_id_)
                        {
                            uint8_t _index=0;
                            for(auto& elem : tpdo.elements_)
                            {
                                for(int i=0; i<elem->byte_size; i++)
                                {
                                    elem->value += (frame.data[_index] << i*8);
                                }
                                elem->convert();
                                ++_index;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
};

void* sync_thread(void* arg)
{
    auto sync = static_cast<SYNC*>(arg);
    auto can = sync->can_;
    auto& rpdo = sync->rpdo_;
    auto& tpdo = sync->tpdo_;

    while(sync->thread_need_run_.load())
    {
        //
    }
}

#endif