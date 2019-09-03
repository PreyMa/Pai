//
// Created by Matthias Preymann on 27.07.2019.
//

#include <cstdlib>

#include "Crash.h"
#include "Console.h"

CrashManager CrashManager::m_manager;

CrashManager::CrashManager() {
    signal(SIGSEGV, CrashManager::signalHandler);
    std::set_terminate(CrashManager::terminateHandler);
}

void CrashManager::terminateHandler() {
    // Acquire full control of the console for the whole scope of this method
    auto lock= Console::acquire();

    // Get uncaught exception
    std::exception_ptr exptr = std::current_exception();
    if (exptr != 0)
    {
        // Rethrow the uncaught exception and get its type
        try
        {
            std::rethrow_exception(exptr);
        }
        catch (std::exception &ex)
        {
            Console::errorln( "Terminated due to uncaught exception: ", ex.what() );
        }
        catch (...)
        {
            Console::errorln( "Terminated due to uncaught unknown exception." );
        }
    }
    else
    {
        Console::errorln( "Terminated due to unknown reason." );
    }

    Console::flushError();

    // Print stacktrace
    printStacktrace();
    std::abort();
}

void CrashManager::signalHandler(int sig) {
    auto lock= Console::acquire();

    Console::errorln("Caught singal: ", sig);

    // Print stacktrace
    printStacktrace();
    std::abort();
}


#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#include <sstream>


template< typename T_Type, int bufferSize >
class LocalContainer {
private:
    unsigned char m_memory[sizeof(T_Type) + bufferSize ];

public:
    inline T_Type* get() { return reinterpret_cast<T_Type*>(m_memory); }
};

void CrashManager::printStacktrace() {

    // Initialize the symbol handler for the current process
    auto curProcess= GetCurrentProcess();
    SymInitialize( curProcess, nullptr, true );

    // Load the stack trace symbol addresses
    void *symbolAddresses[CrashManager::traceBufferLength];
    auto numberOfFrames= CaptureStackBackTrace( 0, CrashManager::traceBufferLength, symbolAddresses, nullptr );

    // Create Symbol Info Object locally
    LocalContainer<SYMBOL_INFO, CrashManager::traceSymbolNameLength> symbolContainer;
    auto symbolPtr= symbolContainer.get();
    symbolPtr->MaxNameLen = CrashManager::traceSymbolNameLength;
    symbolPtr->SizeOfStruct = sizeof(SYMBOL_INFO);

    IMAGEHLP_LINE64 line;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    // Get the name of the symbols based on their addresses
    resolveSymbols( symbolAddresses, numberOfFrames );

    Console::flushError();
}

void CrashManager::resolveSymbols( void* symbolAddresses[], const unsigned int numberOfFrames ) {

    // Retrieve path to the executable of the current process
    char path[CrashManager::maxPathLength];
    if( !GetModuleFileNameA(nullptr, path, CrashManager::maxPathLength) ) {
        Console::errorln("Could not retrieve executable path");

        printAddresses( symbolAddresses, numberOfFrames );
        return;
    }

    // Build shell command
    std::stringstream stream;
    stream << std::hex << "addr2line -f -C -e \"" << path << '\"';

    for( unsigned int i=0; i!= numberOfFrames; i++ ) {
        auto address= (DWORD64)symbolAddresses[i];
        stream << " 0x" << address;
    }

    // Call addr2line and pipe back its output
    auto pipe= popen( stream.str().c_str(), "r" );
    if( !pipe ) {
        Console::println("Could not run addr2line");
        printAddresses(symbolAddresses, numberOfFrames);
        return;
    }

    // Iterate over each line and format the output
    unsigned int i= 0;
    while( !feof( pipe ) ) {
        // Get symbol name
        if( fgets(path, CrashManager::maxPathLength, pipe ) ) {

            path[ strlen(path)-1 ]= '\0';
            Console::error("at '", path, "' (0x", std::hex, symbolAddresses[i], ") ");

            // Only print the path if it can be found
            if( fgets(path, CrashManager::maxPathLength, pipe ) ) {
                path[ strlen(path)-1 ]= '\0';
                if( *path != '?' ) {
                    Console::error( "at '", path, '\'' );
                }
            }

            Console::error("\n\n");

            i++;
        }

    }
}


void CrashManager::printAddresses( void* symbolAddresses[], const unsigned int numberOfFrames ) {
    // Iterate through frame addresses
    for( unsigned int i=0; i!= numberOfFrames; i++ ) {
        auto address= (DWORD64)symbolAddresses[i];
        Console::errorln( "at '\?\?' at address 0x", std::hex, address, std::dec );
    }
}


#endif

