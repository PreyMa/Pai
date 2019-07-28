//
// Created by Matthias Preymann on 24.07.2019.
//

#ifndef PROMISE_FOREACH_H
#define PROMISE_FOREACH_H

#include "Event.h"
#include "Owner.h"
#include "Promise.h"

template< typename T_ItPointer, typename T_Lambda >
class ForEachTask : public Promise< EventContainer< T_ItPointer >, EventContainer<> > {
private:
    T_ItPointer m_array;
    LambdaContainer< T_Lambda > m_function;

public:
    template< typename T_Param >
    explicit ForEachTask( T_Param&& it, T_Lambda&& lam )
            : m_array( std::forward<T_Param>( it ) ), m_function( std::forward<T_Lambda>(lam) ) {}

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


template< typename T_ItPointer, typename T_Lambda >
auto forEach( T_ItPointer&& ptr, WorkerPool& p, T_Lambda&& lam ) {
    return createPromiseBuilder( std::make_unique< ForEachTask<T_ItPointer, T_Lambda> >(
            std::forward<T_ItPointer>(ptr), std::forward<T_Lambda>(lam)), p );
};


#endif //PROMISE_FOREACH_H
