//
// Created by Matthias Preymann on 20.07.2019.
//

#include "EventLoop.h"

void EventLoop::sendEvent(std::unique_ptr<Event> ev) {
    m_queue.push( std::move(ev) );
}

void EventLoop::run() {
    while( m_enable ) {
        std::cout << "Waiting for events... \n";
        std::unique_ptr<Event> ev= m_queue.waitForPop();
        ev->execute( *this );
    }
}
