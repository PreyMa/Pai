//
// Created by Matthias Preymann on 21.07.2019.
//

#ifndef PROMISE_FILELOADER_H
#define PROMISE_FILELOADER_H

#include <fstream>
#include "Event.h"
#include "Promise.h"


/**
 * File Loader Task
 *
 * Specialisation of the Promise class with two Event types as parameters for the parent class
 * The call parameters of the event functors are defined via these types
 * Contains the code run by the worker thread
 */
class FileLoaderTask : public Promise<EventContainer< std::string >, EventContainer<std::string, int> > {
private:
    std::string m_path;

public:

    FileLoaderTask( std::string path )
            : m_path( std::move(path) ) {}

    void execute(Worker::WorkerInterface& intf) override {
        std::cout << "Executing on a thread.\n";

        if( !m_callbackResolve ) {
            return;
        }

        std::ifstream t(m_path);
        std::string str;

        if( !t.is_open() ) {
            if( m_callbackReject ) {
                std::get<0>(m_callbackReject->getData())= std::move( m_path );
                std::get<1>(m_callbackReject->getData())= -1;
                intf.sendEvent(std::move(m_callbackReject));
                return;
            }
        }

        t.seekg(0, std::ios::end);
        str.reserve(t.tellg());
        t.seekg(0, std::ios::beg);

        str.assign((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());

        m_callbackResolve->getData()= std::move( str );

        intf.sendEvent( std::move( m_callbackResolve ) );
    }
};


/**
 * Function to create a new Promise Builder for a FileLoaderTask
 * @param path - Path of the file to be loaded
 * @param p - Reference to the Worker Pool
 * @return - New Promise Builder
 */
auto readFile( std::string path, WorkerPool& p ) {
    return createPromiseBuilder( std::make_unique<FileLoaderTask>(std::move(path)), p );
}



#endif //PROMISE_FILELOADER_H
