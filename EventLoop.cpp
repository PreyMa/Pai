//
// Created by Matthias Preymann on 20.07.2019.
//

#include "EventLoop.h"
#include "Console.h"

void EventLoop::sendEvent(std::unique_ptr<Event> ev) {
    m_queue.push( std::move(ev) );
}

void EventLoop::run() {
    // Sanity check that the pointers are set
    checkSetup();

    while( m_enable ) {
        Console::println( "Waiting for events... " );
        std::unique_ptr<Event> ev= m_queue.waitForPop();
        ev->execute( *this );
    }
}

void EventLoop::checkSetup() {
    if( !m_timerPtr ) {
        throw std::runtime_error( "Timer is not set (nullptr)" );
    }
    if( !m_poolPtr ) {
        throw std::runtime_error( "Worker Pool is not set (nullptr)" );
    }
}
