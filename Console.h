//
// Created by Matthias Preymann on 28.07.2019.
//

#ifndef PROMISE_CONSOLE_H
#define PROMISE_CONSOLE_H


#include <mutex>
#include <iostream>

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
    static void flush() {
        std::cout.flush();
    }

    static void flushError() {
        std::cerr.flush();
    }

    static void flushAll() {
        flush();
        flushError();
    }

    static auto acquire() {
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        flushAll();

        return std::move( lock );
    }

    template< typename ...T_Args >
    static void print( T_Args ... args ) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        printInternal( std::cout, args... );
    }

    template< typename ...T_Args >
    static void println( T_Args ... args ) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        printInternal( std::cout, args... );
        std::cout << "\n";
    }

    template< typename ...T_Args >
    static void error( T_Args ... args ) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        printInternal( std::cerr, args... );
    }

    template< typename ...T_Args >
    static void errorln( T_Args ... args ) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        printInternal( std::cerr, args... );
        std::cerr << "\n";
    }

};


#endif //PROMISE_CONSOLE_H
