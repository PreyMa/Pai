//
// Created by Matthias Preymann on 28.07.2019.
//

#ifndef PROMISE_CONSOLE_H
#define PROMISE_CONSOLE_H


#include <mutex>
#include <iostream>

/**
 * Static Console Class
 * Interface to print synchronized to the standard
 * out / error stream
 *
 */
class Console {
private:
    static std::recursive_mutex m_mutex;

    Console() {}

    static void printInternal(std::ostream&) {}

    template<typename T_Arg >
    static void printInternal(std::ostream& os, T_Arg val ) {
        os << val;
    }

    template<typename T_Arg, typename ...T_Args >
    static void printInternal(std::ostream& os, T_Arg val, T_Args... args) {
        os << val;
        printInternal( os, args... );
    }

public:
    /**
     * Flush the standard out stram
     */
    static void flush() {
        std::cout.flush();
    }

    /**
     * Flush the standard error stream
     */
    static void flushError() {
        std::cerr.flush();
    }

    /**
     * Flush all streams
     */
    static void flushAll() {
        flush();
        flushError();
    }

    /**
     * Acquire the recursive mutex for printing continuously with multiple
     * calls to print / error methods
     *
     * @return unique_lock to hold
     */
    static auto acquire() {
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        flushAll();

        return std::move( lock );
    }

    /**
     * Templated method to print to the standard out stream synchronized
     */
    template< typename ...T_Args >
    static void print( T_Args ... args ) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        printInternal( std::cout, args... );
    }

    /**
     * Templated method to print to the standard out stream synchronized
     * Adds a new-line character at the end
     */
    template< typename ...T_Args >
    static void println( T_Args ... args ) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        printInternal( std::cout, args... );
        std::cout << "\n";
    }

    /**
     * Templated method to print to the standard error stream synchronized
     */
    template< typename ...T_Args >
    static void error( T_Args ... args ) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        printInternal( std::cerr, args... );
    }

    /**
     * Templated method to print to the standard error stream synchronized
     * Adds a new-line character at the end
     */
    template< typename ...T_Args >
    static void errorln( T_Args ... args ) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        printInternal( std::cerr, args... );
        std::cerr << "\n";
    }

};


#endif //PROMISE_CONSOLE_H
