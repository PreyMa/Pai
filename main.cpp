#include <iostream>

#include "WorkerPool.h"
#include "EventLoop.h"
#include "FileLoader.h"


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
        readFile( "myFile.txt", p ).then( createEvent<FileLoaderEvent>( [&p](EventLoop&, std::string& data ) {
            // Output the contents of the file
            std::cout << "Read a file: '" << data << "'\n";

            // Read another file
            readFile( "myOtherFile.txt", p ).then( createEvent<FileLoaderEvent>( [](EventLoop& ctrl, std::string& data) {
                std::cout << "Read another file: '" << data << "'\n";

                // Stop the event loop
                ctrl.stop();

            } ) );

        } ) ).except( createEvent<FunctionEvent>( [](EventLoop&) {
            std::cout << "Oh no something went wrong!\n";
        } ) ) ;
    }

    // Run the event loop and process ingoing events
    loop.run();

    // Stop all worker threads
    p.stopAndJoin();

    return 0;
}