//
// Created by Matthias Preymann on 17.07.2019.
//

#ifndef PROMISE_EVENTQUEUE_H
#define PROMISE_EVENTQUEUE_H

#include <mutex>
#include <queue>


/**
 * Templated Atomic Event Queue Class
 *
 * Allows atomically queueing of unique pointers to objects of type T_Event
 * Synchronization is achieved through a single mutex
 * Threads are set to sleep with a condition variable if the queue is currently
 * empty
 *
 * @tparam T_Event - Type of event to be referenced
 */
template < typename T_Event >
class EventQueue {
private:
    std::mutex m_mutex;
    std::condition_variable m_cvar;

    std::queue< std::unique_ptr< T_Event > > m_queue;

    std::unique_ptr<T_Event> unsafePop()  {
        if( m_queue.empty() ) {
            return nullptr;
        }

        auto p= std::move( m_queue.front() );
        m_queue.pop();
        return p;
    }

    void unsafePush(std::unique_ptr<T_Event> p) {
        m_queue.emplace( std::move( p ) );
        m_cvar.notify_one();
    }

public:
    EventQueue() = default;

    void push(std::unique_ptr<T_Event> p)  {
        std::lock_guard<std::mutex> lock( m_mutex );

        unsafePush( std::move(p) );
    }

    std::unique_ptr<T_Event> pop()  {
        std::lock_guard<std::mutex> lock( m_mutex );

        return unsafePop();
    }

    std::unique_ptr<T_Event> waitForPop() {
        std::unique_lock<std::mutex> lock( m_mutex );
        std::unique_ptr<T_Event> p;

        while( (p= this->unsafePop()) == nullptr ) {
            m_cvar.wait( lock );
        }

        return p;
    }

    template< typename T >
    void replace( unsigned int num ) {
        std::lock_guard<std::mutex> lock( m_mutex );
        std::queue<std::unique_ptr<T_Event>>().swap( m_queue );

        for( ; num; num-- ) {
            unsafePush( std::make_unique<T>() );
        }
    }
};


#endif //PROMISE_EVENTQUEUE_H
