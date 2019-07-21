//
// Created by Matthias Preymann on 21.07.2019.
//

#ifndef PROMISE_FILELOADER_H
#define PROMISE_FILELOADER_H

#include <fstream>
#include "Event.h"
#include "Promise.h"

/**
 * Non-Abstract Event Class
 *
 * Contains the events call parameters as message properties
 */
class FileLoaderEventBase : public virtual EventBase {
protected:
    std::string m_data;

public:
    inline std::string& getData() { return m_data; }
};


/**
 * Templated Specialised Event Class
 *
 * Has the actual callable lambda object of variable size
 * @tparam T_Lambda - type of the functor (lambda) executed by the event loop
 */
template< typename T_Lambda >
class FileLoaderEvent : public FileLoaderEventBase {
private:
    LambdaContainer<T_Lambda> m_function;
public:
    FileLoaderEvent( T_Lambda&& lam )
            : m_function( std::forward<T_Lambda>(lam) ) {}

    void execute( EventLoop& l ) override { m_function.get()( l, m_data ); }
};


/**
 * File Loader Task
 *
 * Specialisation of the Promise class with 'FileLoaderEvent(Base)'
 * and 'Event' as resolve and reject event types
 * Contains the code run by the worker thread
 */
class FileLoaderTask : public Promise<FileLoaderEventBase, Event> {
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
