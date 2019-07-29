#include <iostream>

#include "WorkerPool.h"
#include "EventLoop.h"
#include "FileLoader.h"
#include "ForEach.h"
#include "Crash.h"
#include "Console.h"
#include "SmallStack.h"
#include "ObjectPool.h"

void myFileError( EventLoop& l, const std::string& p, const int err ) {
    Console::errorln( "File '", p, "' could not be loaded with code: ", err );
    l.stop();
}

int main() {
    Console::println( "Hello, World! This is a demo!" );


    // Create new event loop and worker pool
    EventLoop loop;
    WorkerPool p(loop, 3);

    // Attach the worker pool to the event loop
    loop.setWorkers( p );

    // Keep the promise builder scoped, so that it submits the promise on destruction
    {
        // Read a text file
        readFile( "myFile.txt", p ).then( [&p](EventLoop&, std::string& data ) {
            // Output the contents of the file
            Console::println( "Read a file: '", data, '\'' );

            // Read another file
            readFile( "myOtherFile.txt", p ).then( [](EventLoop& ctrl, std::string& data) {
                Console::println( "Read another file: '", data, '\'' );

                // Stop the event loop
                ctrl.stop();

            } ).except( myFileError );

        } ).except( [](EventLoop&, std::string& p, int err) {
            Console::errorln( "Oh no something went wrong!" );
            Console::errorln( "Could not load file: '", p, "' with error code: ", err );
        } ) ;
    }


    // Create ownership guarded vector of ints
    Owner<std::vector<int>> myVector(std::initializer_list<int>{0, 1, 2, 3, 4, 5, 6});

    {
        // Iterate over the vector and multiply every int by two
        forEach( myVector.move(), p, []( int& v ) { v*= 2; } ).then( [&myVector](EventLoop&, BorrowPointer<std::vector<int>>& ptr) {
            // Output the array
            for( auto& v : *ptr ) {
                std::cout << v << std::endl;
            }
            // Return the array back to the event loop thread
            myVector= std::move( ptr );

        } );
    }

    // Run the event loop and process ingoing events
    loop.run();

    // Stop all worker threads
    p.stopAndJoin();

    return 0;

}
