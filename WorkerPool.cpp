//
// Created by Matthias Preymann on 17.07.2019.
//

#include "WorkerPool.h"
#include "Worker.h"

WorkerPool::WorkerPool(EventLoop &l, unsigned int n)  {
    m_workers.reserve( n );
    for( ; n; n-- ) {
        spawnWorker( l );
    }
}

void WorkerPool::stopAndJoin() {
    m_queue.replace<StopTask>( size() );
    for( auto& w : m_workers ) {
        w->join();
    }
}

void WorkerPool::spawnWorker(EventLoop &l)  {
    m_workers.emplace_back( std::make_unique<Worker>( m_queue, l, size() ) );
}

void WorkerPool::submitTask(std::unique_ptr<Task> e) {
    m_queue.push( std::move( e ) );
}