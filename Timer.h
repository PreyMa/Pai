//
// Created by Matthias Preymann on 28.07.2019.
//

#ifndef PROMISE_TIMINGTHREAD_H
#define PROMISE_TIMINGTHREAD_H


#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <chrono>
#include "Event.h"
#include "PooledSortedList.h"

class EventLoop;

/**
 * Timer Class
 * Runs a sleeping thread that either wakes up when new events
 * are added to its queue or an event is ready to be sent back
 * to the event loop
 */
class Timer {
private:
    static constexpr size_t T_listBucketSize= 32;
    using T_TimePoint= std::chrono::system_clock::time_point;

    /**
     * Internal Pending Event Class
     * Holds the event pointer and the time stamp when its ready
     */
    class PendingEvent {
    private:
        PoolPointer<Event> m_event;
        T_TimePoint m_timePoint;

    public:
        PendingEvent( PoolPointer<Event> e, T_TimePoint t )
                : m_event( std::move(e) ), m_timePoint( t ) {}

        inline const T_TimePoint& getTime() const { return m_timePoint; }

        inline bool isReady() const {
            return (std::chrono::system_clock::now() >= m_timePoint );
        }

        // Operator '>' needed for sorting
        inline bool operator>( const PendingEvent& x ) {
            return (this->m_timePoint > x.m_timePoint);
        }

        void send( EventLoop& el );
    };

    bool m_enable;

    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cvar;

    PooledSortedList< PendingEvent > m_eventList;

    EventLoop& m_eventLoop;

    void dispatchEvents();

public:
    explicit Timer( EventLoop& el );

    void addTimedEvent( std::chrono::milliseconds ms, PoolPointer<Event> e );

    void run();

    void stop();
};


#endif //PROMISE_TIMINGTHREAD_H
