//
// Created by Matthias Preymann on 31.07.2019.
//

#include <fstream>
#include "FileSystem.h"

namespace FileSystem {
    void readSync(const Path &path, std::string &data, std::ios::openmode mode) {
        Detail::readSync(path.c_str(), data, data.capacity(), mode);
    }

    void readSync(const Path &path, std::string &data, size_t len, std::ios::openmode mode) {
        Detail::readSync(path.c_str(), data, len, mode);
    }

    void readSync(const char *const path, std::string &data, std::ios::openmode mode) {
        Detail::readSync(path, data, data.capacity(), mode);
    }

    void readSync( const char *const path, std::string &data, size_t len, std::ios::openmode mode ) {
        Detail::readSync( path, data, len, mode );
    }

    void readSync(std::ifstream &file, std::string &data) {
        Detail::readSync( file, data, data.capacity() );
    }

    void readSync(std::ifstream &file, std::string &data, size_t len) {
        Detail::readSync( file, data, len );
    }


    void Detail::readSync(const char *const path, std::string &data, size_t len, std::ios::openmode mode) {
        std::ifstream file(path, mode);
        if (!file) {
            throw std::runtime_error(std::string("Could not open file: ") + path);
        }

        Detail::readSync(file, data, len);
    }

    void Detail::readSync(std::ifstream &file, std::string &data, size_t len) {
        data.reserve(data.length() + len);
        while (len-- && file.good()) {
            data.push_back(file.get());
        }
    }


    void readFileSync(const Path &path, std::string &data, ::std::ios::openmode mode) {
        Detail::readFileSync( path.c_str(), data, mode );
    }

    void readFileSync(const char *const path, std::string &data, ::std::ios::openmode mode) {
        Detail::readFileSync( path, data, mode );
    }

    void readFileSync(std::ifstream &file, std::string &data) {
        Detail::readFileSync( file, data );
    }



    void Detail::readFileSync(const char* const path, std::string& data, ::std::ios::openmode mode) {
        std::ifstream file(path, mode);
        if (!file) {
            throw std::runtime_error( std::string("Could not open file: ") + path );
        }

        Detail::readFileSync( file, data );
    }

    void Detail::readFileSync(std::ifstream &file, std::string &data) {
        file.seekg( 0, std::ios::end );
        data.reserve( file.tellg() );
        file.seekg( 0, std::ios::beg );

        data.assign((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    }





    void writeSync(const Path &path, const std::string &data, std::ios::openmode mode ) {
        Detail::writeFileSync( path.c_str(), data, mode );
    }

    void writeSync(const Path &path, const std::string &data, size_t  len, std::ios::openmode mode ) {
        Detail::writeSync( path.c_str(), data, len, mode );
    }

    void writeSync(const char* const path, const std::string &data, std::ios::openmode mode ) {
        Detail::writeFileSync( path, data, mode );
    }

    void writeSync(const char* const path, const std::string &data, size_t  len, std::ios::openmode mode ) {
        Detail::writeSync( path, data, len, mode );
    }

    void writeSync( std::ofstream& file, const std::string& data ) {
        Detail::writeSync( file, data, data.length() );
    }

    void writeSync( std::ofstream& file, const std::string& data, size_t len ) {
        Detail::writeSync( file, data, len );
    }


    void Detail::writeSync(const char* path, const std::string &data, size_t len, std::ios::openmode mode) {
        std::ofstream file( path, mode );
        if( !file ) {
            throw std::runtime_error(std::string("Could not open file: ") + path);
        }

        Detail::writeSync( file, data, len );
    }

    void Detail::writeSync(std::ofstream &file, const std::string &data, size_t len) {
        auto it= data.begin();
        while( len-- && file.good() && (it != data.end()) ) {
            file << *it;
            it++;
        }
    }



    void writeFileSync(const Path &path, const std::string &data, ::std::ios::openmode mode) {
        Detail::writeFileSync( path.c_str(), data, mode );
    }

    void writeFileSync(const char *const path, const std::string &data, ::std::ios::openmode mode) {
        Detail::writeFileSync( path, data, mode );
    }

    void writeFileSync(std::ofstream &file, const std::string &data) {
        Detail::writeFileSync( file, data );
    }


    void Detail::writeFileSync(const char *path, const std::string &data, ::std::ios::openmode mode) {
        std::ofstream file( path, mode );
        if( !file ) {
            throw std::runtime_error( std::string("Could not open file: ") + path );
        }

        Detail::writeFileSync( file, data );
    }

    void Detail::writeFileSync(std::ofstream &file, const std::string &data) {
        file << data;
    }





    void Detail::FileLoaderTask::execute(Worker::WorkerInterface &intf) {
        if( !m_callbackResolve ) {
            return;
        }

        try {
            Detail::readFileSync( m_path.c_str(), m_buffer, m_mode );
            Path& p= m_callbackResolve->getData<0>();
            p= std::move( m_path );

            m_callbackResolve->getData<1>()= std::move( m_buffer );
            intf.sendEvent(std::move( m_callbackResolve ));

        } catch( std::runtime_error& e ) {
            if( m_callbackReject ) {
                m_callbackReject->getData<0>()= std::move( m_path );
                m_callbackReject->getData<1>()= -1;
                intf.sendEvent(std::move( m_callbackReject ));
            }
        }
    }

    void Detail::StrFileLoaderTask::execute(Worker::WorkerInterface &intf) {
        if( !m_callbackResolve ) {
            return;
        }

        try {
            Detail::readFileSync( m_path, m_buffer, m_mode );
            m_callbackResolve->getData<0>()= m_path;
            m_callbackResolve->getData<1>()= std::move( m_buffer );
            intf.sendEvent(std::move( m_callbackResolve ));

        } catch( std::runtime_error& e ) {
            if( m_callbackReject ) {
                m_callbackReject->getData<0>()= m_path;
                m_callbackReject->getData<1>()= -1;
                intf.sendEvent(std::move( m_callbackReject ));
            }
        }
    }

    void Detail::FileReaderTask::execute(Worker::WorkerInterface &intf) {
        if( !m_callbackResolve ) {
            return;
        }

        if( !m_file.good() ) {
            if( m_callbackReject ) {
                m_callbackReject->getData<0>()= std::move( m_file );
                m_callbackReject->getData<1>()= -1;
                intf.sendEvent(std::move( m_callbackReject ));
                return;
            }
        }

        Detail::readSync( m_file, m_buffer, m_length );
        m_callbackResolve->getData<0>()= std::move( m_file );
        m_callbackResolve->getData<1>()= std::move( m_buffer );
        intf.sendEvent(std::move( m_callbackResolve ));
    }
}