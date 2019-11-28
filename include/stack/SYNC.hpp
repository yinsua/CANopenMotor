#ifndef CANOPEN_SYNC_H
#define CANOPEN_SYNC_H

#include "CAN.hpp"
#include "PDO.hpp"
#include <chrono>
#include <thread>
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

    std::mutex rpdos_sync_mutex;
    std::mutex tpdos_sync_mutex;
    std::condition_variable rpdos_sync_cv;
    std::condition_variable tpdos_sync_cv;

private:
    void rpdos_scan_and_send()
    {
        {
            std::lock_guard<std::mutex> lg(rpdos_sync_mutex);
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
                            frame[0].data[_index] = (y->value() & (0xFF << i*8) ) >> (i*8);
                            ++_index;
                        }
                    }
                    for(int i=0; i<(8-total_byte); i++)
                    {
                        frame[0].data[7-i] = 0;
                    }
                    can_->send(frame, 1);
                    x.need_update.store(false);
                }
            }
        }
        rpdos_sync_cv.notify_all();
    }

    void tpdos_scan_and_assign()
    {
        {
            std::unique_lock<std::mutex> ul(can_->pdo_mutex);
            std::lock_guard<std::mutex> lg(tpdos_sync_mutex);
            if(can_->pdo_deque.empty()) return;
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
                                elem->value() += (frame.data[_index] << i*8);
                            }
                            elem->convert();
                            ++_index;
                        }
                        break;
                    }
                }
            }
        }
        tpdos_sync_cv.notify_all();
    }
};

void* sync_thread(void* arg)
{
    auto sync = static_cast<SYNC*>(arg);
    auto can = sync->can_;
    auto& rpdo = sync->rpdo_;
    auto& tpdo = sync->tpdo_;

    can_frame sync_frame[1];
    sync_frame[0].can_id = 0x80;
    sync_frame[0].can_dlc = 0;
    for(int i=0; i<8; i++)
    {
        sync_frame[0].data[i] = 0;
    }

    while(sync->thread_need_run_.load())
    {
        sync->rpdos_scan_and_send();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        can->send(sync_frame, 1);
        sync->tpdos_scan_and_assign();
    }
}

#endif