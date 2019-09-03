//
// Created by Matthias Preymann on 26.07.2019.
//

#ifndef PROMISE_CRASH_H
#define PROMISE_CRASH_H

/**
 * Static Crash Manager Class
 * Automatically sets up handlers for uncaught exceptions and
 * OS signals
 */
class CrashManager {
private:
    static constexpr unsigned int traceBufferLength= 1024;
    static constexpr unsigned int traceSymbolNameLength= 1024;
    static constexpr unsigned int maxPathLength= 1024;

    // Internal single instance
    static CrashManager m_manager;

    // private static constructor
    CrashManager();


    static void resolveSymbols(void **, unsigned int);
    static void printAddresses(void **, unsigned int);
public:

    /**
     * Print the call stack frames as stacktrace
     * Addresses are tried to be resolved to symbol names based
     * on available debug information
     */
    static void printStacktrace();

    /**
     * Handler called if the application terminates due to an
     * uncaught exception
     */
    static void terminateHandler();

    /**
     * Handler called if the application receives a signal (SIGSEGV)
     */
    static void signalHandler(int);
};



#endif //PROMISE_CRASH_H

