#include <iostream>

#include "WorkerPool.h"
#include "EventLoop.h"
#include "FileLoader.h"
#include "ForEach.h"

void myFileError( EventLoop& l, const std::string& p, const int err ) {
    std::cout << "File '" << p << "' could not be loaded with code: " << err << std::endl;
    l.stop();
}

int main() {
    std::cout << "Hello, World! This is a demo!\n";

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
            std::cout << "Read a file: '" << data << "'\n";

            // Read another file
            readFile( "myOtherFile.txt", p ).then( [](EventLoop& ctrl, std::string& data) {
                std::cout << "Read another file: '" << data << "'\n";

                // Stop the event loop
                ctrl.stop();

            } ).except( myFileError );

        } ).except( [](EventLoop&, std::string& p, int err) {
            std::cout << "Oh no something went wrong!\n";
            std::cout << "Could not load file: '" << p << "' with error code: " << err << std::endl;
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