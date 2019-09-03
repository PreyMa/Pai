//
// Created by Matthias Preymann on 17.07.2019.
//

#ifndef PROMISE_WORKERPOOL_H
#define PROMISE_WORKERPOOL_H

#include "EventQueue.h"
#include "Task.h"
class EventLoop;

/**
 * Worker Pool Class
 * Holds an array of worker threads and the event (task) queue
 */
class WorkerPool {
private:
    EventQueue<Task> m_queue;
    std::vector<std::unique_ptr<Worker>> m_workers;

public:
    WorkerPool( EventLoop& l, unsigned int n );

    inline std::size_t size() const {
        return m_workers.size();
    }

    void spawnWorker( EventLoop& l );

    void stopAndJoin();

    void submitTask(PoolPointer<Task> e);
};


#endif //PROMISE_WORKERPOOL_H
