//
// Created by Matthias Preymann on 17.07.2019.
//

#ifndef PROMISE_EVENT_H
#define PROMISE_EVENT_H

#include <memory>
#include "LambdaContainer.h"
#include "ObjectPool.h"

class EventLoop;

/**
 * Abstract Event Class
 * Interface for code to be run on the event loop thread
 */
class Event : public PooledObject {
private:

public:
    Event( Deallocator* d )
            : PooledObject( d ) {}

    virtual ~Event() = default;
    virtual void execute( EventLoop& ) = 0;
};


/**
 * Templated Event Container Class
 * Stored the data of an event with an internal tuple
 * This class serves as a base class for all the different events containing a
 * functor (lambda) to be executed
 * As the type of the functor as to be templated the actual event class is
 * a specialised version of this one
 *
 * @tparam T_TupleData - Types of data which are used to call the functor with
 */
template< typename ... T_TupleData >
class EventContainer : public Event {
protected:
    using T_Tuple= std::tuple< T_TupleData... >;
    T_Tuple m_data;

public:
    EventContainer( Deallocator* d )
            : Event( d ) {}

    inline T_Tuple& getData() { return m_data; }

    template< unsigned int T_index >
    inline auto& getData() { return std::get<T_index>(m_data); }
};



/**
 * Templated EventImplement Class
 * This class derives from a class provided as a template parameter which
 * is an EventContainer<>
 *
 *
 * @tparam T_Parent - Event Container to implement
 * @tparam T_Lambda - Lambda type to store
 */
template< typename T_Parent, typename T_Lambda >
class EventImplement : public T_Parent {
private:
    LambdaContainer<T_Lambda> m_function;

    template<size_t ... I>
    auto call( EventLoop& l, std::index_sequence<I ...> )
    {
        return m_function.get()( l, std::get<I>(T_Parent::m_data) ...);
    }

public:
    EventImplement( Deallocator* d, T_Lambda&& lam )
            : T_Parent( d ), m_function( std::forward<T_Lambda>(lam) ) {}

    void execute( EventLoop& l ) override {

        static constexpr auto size = std::tuple_size<typename T_Parent::T_Tuple>::value;
        call( l, std::make_index_sequence<size>{} );
    }
};



/**
 * Templated Function Event Class
 * Allowing the execution of a Lambda which is held by value
 * @tparam T_Lambda - Lambda Type to be stored
 */
template <typename  T_Lambda>
class FunctionEvent : public Event {
protected:
    LambdaContainer<T_Lambda> m_function;

public:
    FunctionEvent( Deallocator* d, T_Lambda&& fn )
            : Event( d ), m_function( std::forward<T_Lambda>(fn) ) {}

    void execute( EventLoop& loop ) override {
        m_function.get()( loop );
    }
};


template< typename T_Class >
class MethodEvent : public Event {
public:
    using T_Method= void(T_Class::*)( EventLoop& );
protected:
    T_Class* const m_objPtr;
    T_Method const m_method;

public:
    MethodEvent( Deallocator* d, T_Class* const o, T_Method m )
            : Event( d ), m_objPtr(o), m_method(m) {}

    void execute( EventLoop& loop ) override {
        // Call member function with object pointer as context
        ((m_objPtr)->*(m_method))( loop );
    }
};


/**
 * Create a unique object of type T_Create which is a template with one parameter
 * ... T_Lambda is the unknown lambda-type which is used as the template parameter for T_Create
 * -> Function for object generation of Lambda Container instances as lambda types can not be queried
 *
 * @tparam T_Create - Object type to be created, has one template parameter T_Lambda
 * @tparam T_Lambda - Lambda type to be stored in T_Create
 * @param lam - Lambda to be stored
 * @return Object of type T_Create
 */
template< template<class> typename T_Create, typename T_Alloc, typename T_Lambda >
std::unique_ptr<T_Create<T_Lambda>> createEvent( T_Alloc& alloc, T_Lambda&& lam ) {
    return alloc.template allocate<T_Create<T_Lambda>>( std::forward<T_Lambda>(lam) );
};

#endif //PROMISE_EVENT_H
