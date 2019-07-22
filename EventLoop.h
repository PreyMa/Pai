//
// Created by Matthias Preymann on 20.07.2019.
//

#ifndef PROMISE_EVENTLOOP_H
#define PROMISE_EVENTLOOP_H


#include <iostream>
#include "Event.h"
#include "EventQueue.h"
#include "WorkerPool.h"

/**
 * Event Loop Class
 * Contains the event queue which messages are executed on the main thread
 */
class EventLoop {
private:
    EventQueue<Event> m_queue;
    WorkerPool* m_pool;

    bool m_enable;

public:
    EventLoop()
            : m_pool( nullptr ), m_enable(true) {}

    inline void setWorkers( WorkerPool& p ) {
        m_pool= &p;
    }

    void stop() {
        m_enable= false;
    }

    void sendEvent( std::unique_ptr<Event> ev );

    void run();
};


#endif //PROMISE_EVENTLOOP_H
