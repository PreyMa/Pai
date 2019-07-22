#include <iostream>

#include "WorkerPool.h"
#include "EventLoop.h"
#include "FileLoader.h"

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
            readFile( "myOtherFile.txtl", p ).then( [](EventLoop& ctrl, std::string& data) {
                std::cout << "Read another file: '" << data << "'\n";

                // Stop the event loop
                ctrl.stop();

            } ).except( myFileError );

        } ).except( [](EventLoop&, std::string& p, int err) {
            std::cout << "Oh no something went wrong!\n";
            std::cout << "Could not load file: '" << p << "' with error code: " << err << std::endl;
        } ) ;
    }

    // Run the event loop and process ingoing events
    loop.run();

    // Stop all worker threads
    p.stopAndJoin();

    return 0;
}