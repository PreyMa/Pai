//
// Created by Matthias Preymann on 17.07.2019.
//

#ifndef PROMISE_TASK_H
#define PROMISE_TASK_H

#include "Worker.h"


/**
 * Abstract Task Class
 * Interface for code to be run on a worker thread
 */
class Task {
private:

public:
    Task() = default;
    virtual ~Task() = default;
    virtual void execute( Worker::WorkerInterface& ) = 0;
};


/**
 * Specialised Stop Task
 * Task to stop the thread that executes it
 */
class StopTask : public Task {
public:

    StopTask()= default;

    void execute( Worker::WorkerInterface& w ) override {
        w.stop();
    }
};


#endif //PROMISE_TASK_H
