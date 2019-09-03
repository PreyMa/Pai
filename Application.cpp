//
// Created by Matthias Preymann on 30.07.2019.
//

#include "Application.h"

Application::Application(unsigned int ws)
        : m_workes( m_eventLoop, ws ), m_timer( m_eventLoop ), m_taskPool( T_taskInitCount ),
          m_alloc( m_taskPool ), m_eventLoop(m_alloc) {

    m_eventLoop.setWorkers( m_workes );
    m_eventLoop.setTimer( m_timer );
}

Application::~Application() {
    m_workes.stopAndJoin();
    m_timer.stop();
}

void Application::start() {
    this->init();

    m_eventLoop.run();

    this->exit();
}

