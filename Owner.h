//
// Created by Matthias Preymann on 23.07.2019.
//

#ifndef PROMISE_OWNER_H
#define PROMISE_OWNER_H

#include <utility>
#include <stdexcept>

template< typename T_Type >
class BorrowPointer;

template< typename T_Type >
class Owner {
private:
    bool m_available;
    T_Type m_obj;

public:
    using T_Pointer= BorrowPointer<T_Type>;

    template< typename ... T_Args >
    Owner( T_Args&& ... args )
            : m_available(true), m_obj( std::forward<T_Args>( args )... ) {}

    ~Owner()= default;

    Owner<T_Type>& reset( BorrowPointer<T_Type> );
    Owner<T_Type>& operator= ( BorrowPointer<T_Type> );

    inline T_Type& get() { return m_obj; }

    inline bool available() const { return m_available; }
    inline operator bool() const { return m_available; }

    inline BorrowPointer<T_Type> move();
};


template< typename T_Type >
class BorrowPointer {
private:
    T_Type* m_ptr;

    friend Owner<T_Type>;

public:
    BorrowPointer( T_Type* p = nullptr )
            : m_ptr( p ) {}

    BorrowPointer( const BorrowPointer<T_Type>& x )= delete;

    BorrowPointer( BorrowPointer<T_Type>&& x )
            : m_ptr( x.m_ptr ) {
        x.m_ptr= nullptr;
    }

    BorrowPointer( Owner<T_Type>&& ow )
            : BorrowPointer( ow.move() ) {}

    inline void operator=( BorrowPointer<T_Type>&& x ) {
        m_ptr= x.m_ptr;
        x.m_ptr= nullptr;
    }

    inline T_Type& get() { return *m_ptr; }
    inline const T_Type& get() const { return *m_ptr; }

    inline T_Type& operator*() { return *m_ptr; }
    inline const T_Type& operator*() const { return *m_ptr; }

    inline operator bool() const { return static_cast<bool>(m_ptr); }

    inline T_Type* release() {
        auto x= m_ptr;
        m_ptr= nullptr;
        return x;
    }
};

template< typename T_Type >
Owner<T_Type>& Owner<T_Type>::reset( BorrowPointer<T_Type> p ) {
    if( &m_obj != &p.get() ) {
        throw std::runtime_error("Returning wrong object to owner.");
    }
    p.release();
    m_available= true;
    return *this;
}


template< typename T_Type >
Owner<T_Type>& Owner<T_Type>::operator=( BorrowPointer<T_Type> p ) {
    return this->reset( std::move(p) );
}

template< typename T_Type >
BorrowPointer<T_Type> Owner<T_Type>::move() {
    if( !m_available ) {
        throw std::runtime_error( "Owner is already empty." );
    }
    m_available= false;
    return BorrowPointer<T_Type>( &m_obj );
}


#endif //PROMISE_OWNER_H
