//
// Created by Matthias Preymann on 17.07.2019.
//

#ifndef PROMISE_WORKERPOOL_H
#define PROMISE_WORKERPOOL_H

#include "EventQueue.h"
class Worker;
class Task;
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

    inline unsigned int size() const {
        m_workers.size();
    }

    void spawnWorker( EventLoop& l );

    void stopAndJoin();

    void submitTask(std::unique_ptr<Task> e);
};


#endif //PROMISE_WORKERPOOL_H
