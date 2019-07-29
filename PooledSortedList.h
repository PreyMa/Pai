//
// Created by Matthias Preymann on 29.07.2019.
//

#ifndef PROMISE_POOLEDSORTEDLIST_H
#define PROMISE_POOLEDSORTEDLIST_H

#include "PooledLinkedList.h"

/**
 * Templated Pooled Sorted List Class
 * Pooled single linked list that provides an 'insert'
 * method to add objects in a sorted manner, where the
 * front is smaller than the back
 *
 * @tparam T_DataType - Type of data to store: has to
 *                      provide a greater than operator
 */
template< typename T_DataType >
class PooledSortedList : public PooledLinkedList< T_DataType > {
public:
    static constexpr size_t T_defaultBucketSize= 32;

    explicit PooledSortedList( const size_t s= T_defaultBucketSize )
    : PooledLinkedList< T_DataType >( s ) {}

    void insert( T_DataType obj ) {
        typename PooledLinkedList< T_DataType >::Iterator it= this->begin(), prev;

        while( !it.isEnd() ) {
            if( *it > obj ) {
                break;
            }

            prev= it;
            it++;
        }

        this->insertAfter( prev, std::move( obj ) );
    }

};


#endif //PROMISE_POOLEDSORTEDLIST_H
