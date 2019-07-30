//
// Created by Matthias Preymann on 20.07.2019.
//

#ifndef PROMISE_EVENTLOOP_H
#define PROMISE_EVENTLOOP_H


#include "Event.h"
#include "EventQueue.h"

class WorkerPool;
class Timer;

/**
 * Event Loop Class
 * Contains the event queue which messages are executed on the main thread
 */
class EventLoop {
private:
    EventQueue<Event> m_queue;
    WorkerPool* m_poolPtr;
    Timer *m_timerPtr;

    bool m_enable;

    void checkSetup();

public:
    EventLoop()
            : m_poolPtr(nullptr), m_timerPtr(nullptr), m_enable(true) {}

    inline void setWorkers( WorkerPool& p ) { m_poolPtr= &p; }

    inline void setTimer( Timer& t ) { m_timerPtr= &t; }

    inline WorkerPool& getWorkers() { return *m_poolPtr; }

    inline Timer& getTimer() { return *m_timerPtr;
    }

    void stop() {
        m_enable= false;
    }

    void sendEvent( std::unique_ptr<Event> ev );

    void run();
};


#endif //PROMISE_EVENTLOOP_H
