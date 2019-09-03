//
// Created by Matthias Preymann on 24.07.2019.
//

#ifndef PROMISE_FOREACH_H
#define PROMISE_FOREACH_H

#include "Event.h"
#include "Owner.h"
#include "Promise.h"
#include "Console.h"


/**
 * For Each Task
 *
 * Specialisation of the Promise class with two Event types as parameters for the parent class
 * The call parameters of the event functors are defined via these types
 * Contains the code run by the worker thread
 */
template< typename T_ItPointer, typename T_Lambda >
class ForEachTask : public Promise< EventContainer< T_ItPointer >, EventContainer<> > {
private:
    T_ItPointer m_array;
    LambdaContainer< T_Lambda > m_function;

public:
    template< typename T_Param >
    explicit ForEachTask( Deallocator* d, T_Param&& it, T_Lambda&& lam )
            : Promise< EventContainer< T_ItPointer >, EventContainer<> >( d ),
              m_array( std::forward<T_Param>( it ) ), m_function( std::forward<T_Lambda>(lam) ) {}

    void execute(Worker::WorkerInterface& intf) override {
        Console::println( "Executing Foreach on a thread." );

        for( auto& x : *m_array ) {
            m_function.get()( x );
        }

        auto& cb= this->m_callbackResolve;
        if( cb ) {
            //std::get<0>(cb->getData())= std::move( m_array );
            cb->getData()= std::move( m_array );
            intf.sendEvent( std::move( cb ) );
        }
    }
};


/**
 * Function to create a new Promise Builder for a ForEachTask
 * @tparam T_ItPointer - Type of pointer to an iteratable object
 * @tparam T_Lambda - Functor (Lambda) Type
 * @param ptr - Pointer to an iteratable object
 * @param p - Reference to the Worker Pool
 * @param lam - Functor (Lambda) to call on each instance
 * @return - New Promise Builder
 */
template< typename T_ItPointer, typename T_Allocator, typename T_Lambda >
auto forEach( T_ItPointer&& ptr, T_Allocator& alloc, WorkerPool& p, T_Lambda&& lam ) {
    return createPromiseBuilder( alloc.template allocate< ForEachTask<T_ItPointer, T_Lambda> >(
            std::forward<T_ItPointer>(ptr), std::forward<T_Lambda>(lam)), p, alloc );
};


#endif //PROMISE_FOREACH_H
