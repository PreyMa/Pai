//
// Created by Matthias Preymann on 17.07.2019.
//

#ifndef PROMISE_PROMISE_H
#define PROMISE_PROMISE_H

#include "Task.h"
#include "WorkerPool.h"

/**
 * Templated Promise Class
 * A task that can fire either a 'resolve' or a 'reject' event
 * The template parameters define the base types of the events to be stored
 * where the actual events stored are specialized with the type of functor (lambda)
 * they contain
 *
 * @tparam T_ResolveEventTemp - Base type of the resolve event
 * @tparam T_RejectEventTemp  - Base type of the reject event
 */
template< typename T_ResolveEventTemp, typename T_RejectEventTemp >
class Promise : public Task {
protected:
    PoolPointer<T_ResolveEventTemp> m_callbackResolve;
    PoolPointer<T_RejectEventTemp> m_callbackReject;

public:
    // Types of the base events (pointer types)
    using T_ResolveEventBase= T_ResolveEventTemp;
    using T_RejectEventBase= T_RejectEventTemp;

    // Types of the implemented events
    template< typename T_Lambda >
    using T_ResolveEvent = EventImplement< T_ResolveEventBase, T_Lambda >;

    template< typename T_Lambda >
    using T_RejectEvent = EventImplement< T_RejectEventBase, T_Lambda >;

    // Constructors
    Promise( Deallocator* d, PoolPointer<T_ResolveEventBase> res, PoolPointer<T_RejectEventBase> rej )
            : Task( d ), m_callbackResolve( std::move(res) ), m_callbackReject( std::move(rej) ) {}

    Promise( Deallocator* d )
            : Task( d ), m_callbackResolve(nullptr), m_callbackReject(nullptr) {}

    // Setters
    inline void setResolve( PoolPointer<T_ResolveEventBase> r ) {
        if( m_callbackResolve ) {
            throw std::runtime_error("Promise: Resolve callback cannot be overridden.");
        }
        m_callbackResolve= std::move( r );
    }

    inline void setReject( PoolPointer<T_RejectEventBase> r ) {
        if( m_callbackReject ) {
            throw std::runtime_error("Promise: Reject callback cannot be overridden.");
        }
        m_callbackReject= std::move( r );
    }
};

/**
 * Templated Promise Builder Class
 * Serves the setup of a promise with its resolve and reject event
 * Automatically adds the promise as a task to the Worker Pool on its destruction
 *
 * @tparam T_Promise - Type of promise to point to
 */
template< typename T_Promise, typename T_Allocator >
class PromiseBuilder {
private:
    PoolPointer<T_Promise> m_promise;
    WorkerPool& m_pool;
    T_Allocator& m_alloc;

public:

    // Types of the implemented events
    template< typename T_X >
    using T_ResolveEventType= typename T_Promise::template T_ResolveEvent<T_X>;

    template< typename T_X >
    using T_RejectEventType= typename T_Promise::template T_RejectEvent<T_X>;

    // Constructors & Destructors
    PromiseBuilder( PoolPointer<T_Promise> pr, WorkerPool& p, T_Allocator& a )
            : m_promise( std::move(pr) ), m_pool(p), m_alloc(a) {}

    ~PromiseBuilder() {
        m_pool.submitTask( std::move(m_promise) );
    }


    // Enable moving
    PromiseBuilder( PromiseBuilder<T_Promise, T_Allocator>&& x )= default;


    // Chainable setters
    inline PromiseBuilder& then( PoolPointer<typename T_Promise::T_ResolveEventBase> res ) {
        m_promise->setResolve( std::move(res) );
        return *this;
    }

    inline PromiseBuilder& except( PoolPointer<typename T_Promise::T_RejectEventBase> rej ) {
        m_promise->setReject( std::move(rej) );
        return *this;
    }



    template< typename T_Lambda >
    inline PromiseBuilder& then( T_Lambda&& lam ) {

        m_promise->setResolve( m_alloc.template allocate<T_ResolveEventType<T_Lambda> >( std::forward<T_Lambda>(lam) ) );
        return *this;
    }

    template< typename T_Lambda >
    inline PromiseBuilder& except( T_Lambda&& lam ) {
        m_promise->setReject( m_alloc.template allocate<T_RejectEventType<T_Lambda> >( std::forward<T_Lambda>(lam) ) );
        return *this;
    }
};




/**
 * Creates a Promise Builder Object which is returned (moved) by value
 * The template parameter needed to create a Promise Builder is stolen from the pointer to the promise
 *
 * @tparam T_PromiseType - Type of promise to be held by the Promise Builder
 * @param promise - Promise to be setup by the Promise Builder
 * @param p - Reference to the Worker Pool
 * @return - new Promise Builder
 */
template< typename T_PromiseType, typename T_Allocator >
auto createPromiseBuilder( PoolPointer<T_PromiseType> promise, WorkerPool& p, T_Allocator& a ) {
    return PromiseBuilder<T_PromiseType, T_Allocator>( std::move(promise), p, a );
}


#endif //PROMISE_PROMISE_H
