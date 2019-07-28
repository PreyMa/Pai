//
// Created by Matthias Preymann on 26.07.2019.
//

#ifndef PROMISE_CRASH_H
#define PROMISE_CRASH_H

#include <iostream>
class CrashManager {
private:
    static constexpr unsigned int traceBufferLength= 1024;
    static constexpr unsigned int traceSymbolNameLength= 1024;



    static CrashManager m_manager;

    CrashManager();
public:

    static void printStacktrace();
    static void terminateHandler();
    static void signalHandler(int);

};



#endif //PROMISE_CRASH_H
