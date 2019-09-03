//
// Created by Matthias Preymann on 17.07.2019.
//

#ifndef PROMISE_WORKER_H
#define PROMISE_WORKER_H

#include <thread>
#include <iostream>
#include "ObjectPool.h"

class Task;

template < typename T >
class EventQueue;

class EventLoop;
class Event;

/**
 * Worker Class
 * Thread that waits for messages from the event (task) queue
 * Received tasks are executed
 */
class Worker {
private:
    bool m_enable;
    const unsigned int m_id;

    EventLoop& m_eventLoop;

    EventQueue<Task>& m_queue;
    std::thread m_thread;


    inline void stop() {
        m_enable= false;
    }

    inline unsigned int getID() const {
        return m_id;
    }

    void sendEvent( PoolPointer<Event> ev );

public:

    class WorkerInterface {
    private:
        Worker& m_worker;

    public:
        WorkerInterface( Worker* w )
                : m_worker(*w) {}

        inline void stop() {
            m_worker.stop();
        }

        inline unsigned int getID() const {
            return m_worker.getID();
        }

        void sendEvent( PoolPointer<Event> ev );
    };

    friend WorkerInterface;

    Worker( EventQueue<Task>& q, EventLoop& l, const unsigned int i );

    Worker( Worker& w ) = delete;

    void join() {
        m_thread.join();
    }

    void run();
};



#endif //PROMISE_WORKER_H
