//
// Created by Matthias Preymann on 28.07.2019.
//

#include "Timer.h"
#include "EventLoop.h"
#include "Console.h"

Timer::Timer( EventLoop &el )
        : m_enable(true), m_eventList(T_listBucketSize), m_eventLoop(el), m_thread( Timer::run, this ) {}

void Timer::run() {
    std::unique_lock<std::mutex> m_lock(m_mutex);

    while( true ) {
        // If the list has an event, wait until it is ready
        if( m_eventList.isEmpty() ) {
            //Console::println("Tmr: No events to wait for...");
            m_cvar.wait( m_lock );

        } else {
            //Console::println("Tmr: wait for event to be ready...");
            m_cvar.wait_until( m_lock, m_eventList.front().getTime() );
        }

        // Stop
        if( !m_enable ) {
            break;
        }

        // Dispatch any ready events
        dispatchEvents();
    }

    Console::println("Timer stopped...");
}

void Timer::addTimedEvent( std::chrono::milliseconds ms, PoolPointer<Event> e ) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Calculate absolute time stamp from current time and the provided offset
    T_TimePoint t= std::chrono::system_clock::now() + ms;
    m_eventList.insert( PendingEvent( std::move(e), t ) );

    // Notify the thread to wake up
    m_cvar.notify_all();
}

void Timer::dispatchEvents() {
    // Dispatch all events that are ready now
    while( !m_eventList.isEmpty() ) {
        auto& pending= m_eventList.front();

        // If the next event is not ready yet, go back waiting
        if( !pending.isReady() ) {
            //Console::println("Tmr: No events to dispatch!");
            return;
        }

        // Send the event and remove it from the list
        //Console::println( "Tmr: Dispatch" );
        pending.send( m_eventLoop );
        m_eventList.popFront();
    }
}

void Timer::stop() {
    // Set the enable flag to false and notify the thread
    m_enable= false;
    m_cvar.notify_all();
    m_thread.join();
}

void Timer::PendingEvent::send(EventLoop &el) {
    el.sendEvent( std::move( m_event ) );
}
