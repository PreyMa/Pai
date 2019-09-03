//
// Created by Matthias Preymann on 31.07.2019.
//

#ifndef PROMISE_FILESYSTEM_H
#define PROMISE_FILESYSTEM_H


#include <string>
#include <fstream>
#include "Event.h"
#include "Promise.h"

namespace FileSystem {

    class Path {
    private:
        std::string m_path;

    public:
        static constexpr char T_Seperator= '/';

        Path()= default;

        explicit Path( std::string p )
        : m_path( std::move(p) ) {}

        void append( const Path& p ) { m_path += T_Seperator; m_path+= p.m_path; }
        void append( const char* p ) { m_path += T_Seperator; m_path+= p; }

        Path& operator +( const Path& p ) { append(p); return *this; }
        Path& operator +( const char* p ) { append(p); return *this; }

        const std::string& toString() const { return m_path; }
        std::string& toString() { return m_path; }

        const char* c_str() const { return m_path.c_str(); }

        // Path& pop()              // Remove last path element ( dir or file )
        // Path& mkCanonical()
    };

    /** Synchronoues File Reading / Writing **/
    namespace Detail {

        void readSync( const char*, std::string&, size_t, std::ios::openmode );
        void readSync( std::ifstream&, std::string&, size_t );

        void readFileSync( const char*, std::string&, std::ios::openmode );
        void readFileSync( std::ifstream&, std::string& );


        void writeSync( const char*, const std::string&, size_t, std::ios::openmode );
        void writeSync( std::ofstream&, const std::string&, size_t );

        void writeFileSync( const char*, const std::string&, std::ios::openmode );
        void writeFileSync( std::ofstream&, const std::string& );

    }

    void writeSync( const Path&, const std::string&, size_t, std::ios::openmode= std::ios::openmode() );
    void writeSync( const Path&, const std::string&,         std::ios::openmode= std::ios::openmode() );
    void writeFileSync( const Path&, const std::string&, std::ios::openmode= std::ios::openmode() );

    void writeSync( const char*, const std::string&, size_t, std::ios::openmode= std::ios::openmode() );
    void writeSync( const char*, const std::string&,         std::ios::openmode= std::ios::openmode() );
    void writeFileSync( const char*, const std::string&, std::ios::openmode= std::ios::openmode() );

    void writeSync( std::ofstream&, const std::string&, size_t );
    void writeSync( std::ofstream&, const std::string&         );
    void writeFileSync( std::ofstream&, const std::string& );


    void readSync( const Path&, std::string&, size_t, std::ios::openmode= std::ios::openmode() );  // Read number of characters
    void readSync( const Path&, std::string&,         std::ios::openmode= std::ios::openmode() );  // Read capacity of buffer
    void readFileSync( const Path&, std::string&, std::ios::openmode= std::ios::openmode() );      // Read whole file

    void readSync( const char*, std::string&, size_t, std::ios::openmode= std::ios::openmode() );
    void readSync( const char*, std::string&,         std::ios::openmode= std::ios::openmode() );
    void readFileSync( const char*, std::string&, std::ios::openmode= std::ios::openmode() );


    void readSync( std::ifstream&, std::string&, size_t );
    void readSync( std::ifstream&, std::string&         );
    void readFileSync( std::ifstream&, std::string& );

    // void readUtfFileSync


    /** Asynchronous File Reading / Writing **/
    namespace Detail {

        /**
         * Task to load a whole file in a worker thread
         */
        class FileLoaderTask : public Promise < EventContainer< Path, std::string >, EventContainer< Path, int > > {
        private:
            Path m_path;
            std::string m_buffer;
            std::ios::openmode m_mode;

        public:
            FileLoaderTask( Deallocator* d, Path p, std::string b, std::ios::openmode m= std::ios::openmode() )
                    : Promise(d), m_path( std::move(p) ), m_buffer( std::move(b) ), m_mode( m ) {}

            void execute( Worker::WorkerInterface& ) override;
        };

        /**
         * Task to load a whole file in a worker thread
         * Only takes a C-string instead of a std::string as path parameter (unsafe but quicker for string constants)
         */
        class StrFileLoaderTask : public Promise < EventContainer< const char*, std::string >, EventContainer< const char*, int > > {
        private:
            const char* m_path;
            std::string m_buffer;
            std::ios::openmode m_mode;

        public:
            StrFileLoaderTask( Deallocator* d, const char* p, std::string b, std::ios::openmode m= std::ios::openmode() )
                    : Promise(d), m_path( p ), m_buffer( std::move(b) ), m_mode( m ) {}

            void execute( Worker::WorkerInterface& ) override;
        };

        /**
         * Task to read a specified number of bytes/characters from a file in a worker thread
         */
        class FileReaderTask : public Promise< EventContainer< std::ifstream, std::string >, EventContainer< std::ifstream, int > > {
        private:
            std::ifstream m_file;
            std::string m_buffer;
            std::size_t m_length;

        public:
            FileReaderTask( Deallocator* d, std::ifstream f, std::string b, std::size_t l )
                    : Promise(d), m_file( std::move(f) ), m_buffer( std::move(b) ), m_length( l ) {}

            void execute( Worker::WorkerInterface& ) override;
        };

    }

    /**
     * Make functions fake templates to allow the definition in the header file without creating
     * a version in every source file that includes it
     * The function need to be defined to allow auto return type deduction
     */

    // Path provided as C-string constant
    template< typename T_Alloc >
    auto read( const char* pa, T_Alloc& alloc, WorkerPool& p, size_t l, std::ios::openmode m= std::ios::openmode() ) {
        return createPromiseBuilder( alloc.template allocate<Detail::FileReaderTask>(std::ifstream(pa, m), std::string(), l), p, alloc );
    }

    template< typename T_Alloc >
    auto read( const char* pa, T_Alloc& alloc, WorkerPool& p, std::string b, std::ios::openmode m= std::ios::openmode() ) {
        return createPromiseBuilder( alloc.template allocate<Detail::FileReaderTask>(std::ifstream(pa, m), std::move(b), b.capacity()), p, alloc );
    }

    template< typename T_Alloc >
    auto read( const char* pa, T_Alloc& alloc, WorkerPool& p, std::string b, size_t l, std::ios::openmode m ) {
        return createPromiseBuilder( alloc.template allocate<Detail::FileReaderTask>(std::ifstream(pa, m), std::move(b), l), p, alloc );
    }

    // Path provided as Path
    template< typename T_Alloc >
    auto read( const Path& pa, T_Alloc& alloc, WorkerPool& p, size_t l, std::ios::openmode m= std::ios::openmode() ) {
        return createPromiseBuilder( alloc.template allocate<Detail::FileReaderTask>(std::ifstream(pa.toString(), m), std::string(), l), p, alloc );
    }

    template< typename T_Alloc >
    auto read( const Path& pa, T_Alloc& alloc, WorkerPool& p, std::string b, std::ios::openmode m= std::ios::openmode() ) {
        return createPromiseBuilder( alloc.template allocate<Detail::FileReaderTask>(std::ifstream(pa.toString(), m), std::move(b), b.capacity()), p, alloc );
    }

    template< typename T_Alloc >
    auto read( const Path& pa, T_Alloc& alloc, WorkerPool& p, std::string b, size_t l, std::ios::openmode m= std::ios::openmode() ) {
        return createPromiseBuilder( alloc.template allocate<Detail::FileReaderTask>(std::ifstream(pa.toString(), m), std::move(b), l), p, alloc );
    }

    // File stream
    template< typename T_Alloc >
    auto read( std::ifstream f, T_Alloc& alloc, WorkerPool& p, size_t l ) {
        return createPromiseBuilder( alloc.template allocate<Detail::FileReaderTask>(std::move(f), std::string(), l), p, alloc );
    }

    template< typename T_Alloc >
    auto read( std::ifstream f, T_Alloc& alloc, WorkerPool& p, std::string b ) {
        return createPromiseBuilder( alloc.template allocate<Detail::FileReaderTask>(std::move(f), std::move(b), b.capacity()), p, alloc );
    }

    template< typename T_Alloc >
    auto read( std::ifstream f, T_Alloc& alloc, WorkerPool& p, std::string b, size_t l ) {
        return createPromiseBuilder( alloc.template allocate<Detail::FileReaderTask>(std::move(f), std::move(b), l), p, alloc );
    }


    // Read whole file: Path provided as Path
    template< typename T_Alloc >
    auto readFile( Path pa, T_Alloc& alloc, WorkerPool& p, std::ios::openmode m= std::ios::openmode() ) {
        return createPromiseBuilder( alloc.template allocate<Detail::FileLoaderTask>(std::move(pa), std::string(), m), p, alloc );
    }

    template< typename T_Alloc >
    auto readFile( Path pa, T_Alloc& alloc, WorkerPool& p, std::string b, std::ios::openmode m= std::ios::openmode() ) {
        return createPromiseBuilder( alloc.template allocate<Detail::FileLoaderTask>(std::move(pa), std::move(b), m), p, alloc );
    }

    // Read whole file: Path provided as C-string
    template< typename T_Alloc >
    auto readFile( const char* pa, T_Alloc& alloc, WorkerPool& p, std::ios::openmode m= std::ios::openmode() ) {
        return createPromiseBuilder( alloc.template allocate<Detail::StrFileLoaderTask>(pa, std::string(), m), p, alloc );
    }

    template< typename T_Alloc >
    auto readFile( const char* pa, T_Alloc& alloc, WorkerPool& p, std::string b, std::ios::openmode m= std::ios::openmode() ) {
        return createPromiseBuilder( alloc.template allocate<Detail::StrFileLoaderTask>(pa, std::move(b), m), p, alloc );
    }
}


#endif //PROMISE_FILESYSTEM_H
