#include <iostream>

#include "ForEach.h"
#include "Console.h"
#include "Application.h"
#include "Window.h"
#include "FileSystem.h"


namespace FS= FileSystem;

void myFileError( EventLoop& l, const char* p, const int err ) {
    Console::errorln( "File '", p, "' could not be loaded with code: ", err );
    l.stop();
}


// Demo app that reads files and calculates stuff
class MyApp : public Application {
private:
    // Create ownership guarded vector of ints
    Owner<std::vector<int>> myVector;

public:
    MyApp()
    : myVector(std::initializer_list<int>{0, 1, 2, 3, 4, 5, 6}) {}

protected:
    void init() override {

        using T_Mills= std::chrono::milliseconds;

        // Keep the promise builder scoped, so that it submits the promise on destruction
        {
            // Read a text file
            FS::readFile( "myFile.txt", m_alloc, m_workes ).then( [](EventLoop& ctrl, const char* path, std::string& data ) {
                // Output the contents of the file
                Console::println( "Read a file: '", data, '\'' );

                // Read another file
                FS::readFile( "myOtherFile.txt", ctrl.getAlloc(), ctrl.getWorkers() ).then( [](EventLoop& ctrl, const char* path, std::string& data) {
                    Console::println( "Read another file: '", data, '\'' );

                    // Stop the event loop
                    ctrl.stop();

                } ).except( myFileError );

            } ).except( [](EventLoop&, const char* p, int err) {
                Console::errorln( "Oh no something went wrong!" );
                Console::errorln( "Could not load file: '", p, "' with error code: ", err );
            } ) ;
        }

        {
            // Iterate over the vector and multiply every int by two
            forEach( myVector.move(), m_alloc, m_workes, []( int& v ) { v*= 2; } ).then( [this](EventLoop&, BorrowPointer<std::vector<int>>& ptr) {
                // Output the array
                for( auto& v : *ptr ) {
                    std::cout << v << std::endl;
                }
                // Return the array back to the event loop thread
                this->myVector= std::move( ptr );

            } );
        }

    }

    void exit() override {}
};


int main() {
    Console::println( "Hello, World! This is a demo!" );

    // Create the app
    MyApp app;

    // Run the app
    app.start();

    return 0;
}

