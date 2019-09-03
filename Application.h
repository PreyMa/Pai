//
// Created by Matthias Preymann on 30.07.2019.
//

#ifndef PROMISE_APPLICATION_H
#define PROMISE_APPLICATION_H


#include "EventLoop.h"
#include "Timer.h"
#include "WorkerPool.h"
#include "PoolDefs.h"

/**
 * Abstract Application Class
 * Manages the life cycle of all threads and provides an interface
 * to the actual application
 *
 */
class Application {
protected:
    WorkerPool m_workes;
    Timer m_timer;

    using T_Pool= PoolDefs::T_EventPool;

    static constexpr size_t T_taskInitCount= 100;
    T_Pool m_taskPool;

    PoolAllocator< T_Pool > m_alloc;

    EventLoop m_eventLoop;

public:
    static constexpr unsigned int T_defaultWorkerCount= 3;

    explicit Application( unsigned int ws= T_defaultWorkerCount );

    ~Application();

    void start();


protected:


    /**
     * Client methods
     */
    virtual void init()= 0;
    virtual void exit()= 0;
};


#endif //PROMISE_APPLICATION_H
