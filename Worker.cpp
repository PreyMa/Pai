//
// Created by Matthias Preymann on 17.07.2019.
//

#include "Worker.h"
#include "EventQueue.h"
#include "EventLoop.h"
#include "Task.h"

Worker::Worker(EventQueue<Task> &q, EventLoop& l, const unsigned int i)
        : m_enable( true ), m_id(i), m_queue(q), m_eventLoop(l), m_thread(Worker::run, this) {}

void Worker::run()  {
    while( m_enable ) {std::cout << "waiting for work "<< m_id << "... \n";
        WorkerInterface intf(this);

        auto ev= m_queue.waitForPop();
        ev->execute( intf );
    }std::cout << "stopping " << m_id << "... \n";
}

void Worker::sendEvent(std::unique_ptr<Event> ev) {
    m_eventLoop.sendEvent( std::move(ev) );
}

void Worker::WorkerInterface::sendEvent(std::unique_ptr<Event> ev) {
    m_worker.sendEvent( std::move(ev) );
}