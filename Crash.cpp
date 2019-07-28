//
// Created by Matthias Preymann on 27.07.2019.
//

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

    // Iterate through frame addresses and load each symbol
    for( unsigned int i= 0; i!= numberOfFrames; i++ ) {
        auto address= (DWORD64)symbolAddresses[i];
        SymFromAddr( curProcess, address, nullptr, symbolPtr );

        // Print symbol name if one was found
        if( symbolPtr->Name[0] == '\0' ) {
            Console::error( "\tat '???" );
        } else {
            Console::error( "\tat '", symbolPtr->Name );
        }

        // Print file and line if information debug info was found
        DWORD displacement;
        if( SymGetLineFromAddr64( curProcess, address, &displacement, &line ) ) {
            Console::errorln( "' in '", line.FileName, "' at line ", line.LineNumber );
            Console::errorln( " at address 0x", std::hex, address, std::dec );

        } else {
            Console::errorln( "' at address 0x", std::hex, address, std::dec );
            Console::errorln( "\t\t...Resolving of line from address failed (SymGetLineFromAddr64) with code: ", GetLastError() );
        }
    }

    Console::flushError();
}

#endif