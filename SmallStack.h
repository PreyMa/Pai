//
// Created by Matthias Preymann on 29.07.2019.
//

#ifndef PROMISE_SMALLVECTOR_H
#define PROMISE_SMALLVECTOR_H

#include <cstddef>
#include <type_traits>
#include <stdexcept>


 /**
  * Templated Small Stack Class
  * Implements a stack with a push (emplace) and pop method, which throws
  * if the stack is already empty
  * If the stack stores less than T_localSize objects no allocations are
  * made and the internal pointers are used as array
  * When the limit of element that can be held locally is reached the stack
  * is converted to an allocated buffer, which is resized as needed in multiples
  * of 2
  *
  * Detail: The max number of objects that can be stored is size_t / 2 as one
  * of the bits of the length variable is used as a flag (local / dynamic alloc)
  *
  * @tparam T_Element      - Type of object to store
  * @tparam T_localSizeDef - Number of objects to store locally: if the parameter
  *                          is 0 (default) the maximal number of objects that can
  *                          be stored without increasing the size of the class
  *                          is calculated
  */
template< typename T_Element, unsigned int T_localSizeDef= 0 >
class SmallStack {
private:

    using T_ElementContainer = typename std::aligned_storage< sizeof(T_Element), alignof(T_Element) >::type;

    struct T_DynamicDataType {
        T_ElementContainer *m_data;
        std::size_t m_size;
    };

    static constexpr size_t T_defaultLocalSize= sizeof(struct T_DynamicDataType) / sizeof(T_ElementContainer);
    static constexpr size_t T_localSize= (T_localSizeDef < 1) ? T_defaultLocalSize : T_localSizeDef;


    union {
        struct T_DynamicDataType m_dynamic;

        struct {
            T_ElementContainer m_data[ T_localSize ];
        }  m_local;

    } m_inner;

    std::size_t m_length;


    inline bool isLocal() const {
        return static_cast<bool>(m_length & 0x1);
    }

    inline void lengthInc() {
        m_length += 0x2;
    }

    inline void lengthDec() {
        m_length -= 0x2;
    }

    static void copyElements( const size_t sz, T_ElementContainer* const block, T_ElementContainer *const elements ) {
        // Copy elements by creating new ones by move constructor
        for( size_t i= 0; i!= sz; i++ ) {
            new( block + i ) T_Element( std::move( reinterpret_cast<T_Element*>(elements)[i] ) );
        }
    }

    void realloc() {
        auto curSize= m_inner.m_dynamic.m_size;
        auto block= new T_ElementContainer[ 2*curSize ];

        copyElements( curSize, block, m_inner.m_dynamic.m_data );

        delete m_inner.m_dynamic.m_data;

        m_inner.m_dynamic.m_size= 2*curSize;
        m_inner.m_dynamic.m_data= block;
    }

    void enlarge() {
        auto curSize= T_localSize;
        auto block= new T_ElementContainer[ 2*curSize ];

        copyElements( curSize, block, m_inner.m_local.m_data );

        m_inner.m_dynamic.m_size= 2*curSize;
        m_inner.m_dynamic.m_data= block;
        m_length &= ~0x1;
    }

    T_Element* getEmptyCell() {
        T_ElementContainer* p;

        // Check if local mode is enabled
        if( isLocal() ) {
            // Convert to a dynamic stack if no more room is left
            if( getLength() == T_localSize ) {
                enlarge();
                p= m_inner.m_dynamic.m_data+ getLength();

            // Return pointer to the next free cell
            } else {
                p= m_inner.m_local.m_data+ getLength();
            }

        } else {
            // Allocate a new buffer if no more room is left
            if( getLength() == m_inner.m_dynamic.m_size ) {
                realloc();
            }

            // Return pointer to the next free cell
            p= m_inner.m_dynamic.m_data+ getLength();
        }

        lengthInc();
        return reinterpret_cast<T_Element*>(p);
    }

    T_Element* getTopCell() {
        // Throw if the stack is empty
        if( isEmpty() ) {
            throw std::runtime_error("Stack is already empty!");
        }

        T_ElementContainer * p;

        // Check for local flag
        if( isLocal() ) {
            p= m_inner.m_local.m_data+ getLength()-1;
        } else {
            p= m_inner.m_dynamic.m_data+ getLength() -1;
        }

        return reinterpret_cast<T_Element*>(p);
    }


public:

    SmallStack()
    : m_length(0x1) {}

    ~SmallStack() {
        // Delete all remaining objects stored
        while( !isEmpty() ) {
            pop();
        }

        // Delete the buffer if one was allocated
        if( !isLocal() ) {
            delete m_inner.m_dynamic.m_data;
        }
    }

    inline size_t getLength() const {
        return m_length >> 1;
    }

    inline bool isEmpty() const {
        return !getLength();
    }

    inline size_t getCapacity() const {
        return isLocal() ? T_localSize : m_inner.m_dynamic.m_size;
    }

    template< typename ...T_Args >
    void push( T_Args&& ... args ) {
        auto ptr= getEmptyCell();

        new(ptr) T_Element( std::forward<T_Args>( args )... );
    }

    inline T_Element& top() {
        return *getTopCell();
    }

    void pop() {
        auto ptr= getTopCell();
        lengthDec();
        ptr->~T_Element();
    }


};

#endif //PROMISE_SMALLVECTOR_H
