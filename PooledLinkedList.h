//
// Created by Matthias Preymann on 28.07.2019.
//

#ifndef PROMISE_POOLEDLINKEDLIST_H
#define PROMISE_POOLEDLINKEDLIST_H


#include "ObjectPool.h"

/**
 * Templated Pooled Linked List Class
 * Single linked list, that allocates its entries in a
 * pool of variable size.
 * Discarded entries are put back into the pool.
 *
 * @tparam T_DataType - Type to store
 */
template< typename T_DataType >
class PooledLinkedList {
private:

    /**
     * Internal List Entry Class
     * Stores the data and the next pointer publicly
     */
    class ListEntry {
    public:
        ListEntry* m_next;
        T_DataType m_data;

        template< typename ... T_Args >
        explicit ListEntry( ListEntry *const n, T_Args&& ... args )
        : m_next( n ), m_data( std::forward<T_Args>( args )... ) {}
    };


    ObjectPool< ListEntry > m_pool;
    ListEntry* m_begin;

    inline ListEntry* getBeginPointer() {
        if( isEmpty() ) {
            throw std::runtime_error("Cannot access front element of empty list.");
        }
        return m_begin;
    }

public:
    /**
     * Iterator Class
     * Provides an interface to forward iterate over the list
     */
    class Iterator {
    private:
        friend PooledLinkedList;

        ListEntry *m_position;

    public:
        explicit Iterator( ListEntry* const e )
        : m_position( e ) {}

        Iterator()
        : m_position( nullptr ) {}

        inline void next() {
            m_position= m_position->m_next;
        }

        inline void operator++(int) {
            next();
        }

        inline bool isEnd() const {
            return !m_position;
        }

        explicit inline operator bool() const {
            return static_cast<bool>( m_position );
        }

        inline T_DataType& get() {
            return m_position->m_data;
        }

        inline T_DataType& operator*() {
            return get();
        }
    };


    explicit PooledLinkedList( const size_t s )
    : m_pool( s ), m_begin(nullptr) {}

    ~PooledLinkedList() {
        // Destruct all entries
        for( auto it= begin(); it; it++ ) {
            m_pool.free( it.m_position );
        }
    }

    inline bool isEmpty() const {
        return !m_begin;
    }

    inline Iterator begin() {
        return Iterator( m_begin );
    }

    inline T_DataType& front() {
        auto p= getBeginPointer();
        return p->m_data;
    }

    void popFront() {
        auto p= getBeginPointer();
        m_begin= p->m_next;
        m_pool.free( p );
    }

    template< typename ... T_Args >
    void insertAfter( Iterator pos, T_Args&& ... args ) {
        if(pos.isEnd()) {
            pushFront( std::forward<T_Args>( args )... );
            return;
        }

        auto neighbour= pos.m_position;
        auto elem= m_pool.template create<ListEntry>( neighbour->m_next, std::forward<T_Args>( args )... );
        neighbour->m_next= elem;
    }

    template< typename ... T_Args >
    void pushFront( T_Args&& ... args ) {
        auto elem= m_pool.template create<ListEntry>( m_begin, std::forward<T_Args>( args )... );
        m_begin= elem;
    }
};


#endif //PROMISE_POOLEDLINKEDLIST_H
