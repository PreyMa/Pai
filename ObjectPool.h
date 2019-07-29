//
// Created by Matthias Preymann on 28.07.2019.
//

#ifndef PROMISE_OBJECTPOOL_H
#define PROMISE_OBJECTPOOL_H


#include <mutex>
#include <vector>
#include "SmallStack.h"

namespace AllocArrayDetail {
    /**
     * Dummy Mutex Class to serve as interface, if no mutex is needed
     */
    class NoMutex {
    public:
        void lock() {}
        void unlock() {}
        bool try_lock() { return true; }
    };
}

/**
 *Templated Allocation Array Class
 * Stores objects that satisfy the size and alignment specifications
 * Objects are created in blocks of cells. Pointers to the empty cells
 * are held by an array, that is accessed whenever a new object is
 * constructed or an old one is destructed
 * Whether the creation and deletion of objects should happen
 * synchronized is determined by the T_Mutex template parameter
 *
 * @tparam T_CellSize - maximal needed cell size
 * @tparam T_CellAlign - maximal needed cell alignment
 * @tparam T_Mutex - Type of mutex to use for synchronisation (by default a NoMutex dummy is used)
 */
template< std::size_t T_CellSize, std::size_t T_CellAlign, typename T_Mutex= AllocArrayDetail::NoMutex >
class AllocArray {
private:

    using T_Cell= typename std::aligned_storage< T_CellSize, T_CellAlign >::type;

    T_Mutex m_mutex;
    std::vector< T_Cell* > m_slots;

    SmallStack< std::unique_ptr< T_Cell[] > > m_blocks;

    const unsigned int m_blockSize;

    void addBlock() {
        // Create new block of cells
        auto block= std::make_unique<T_Cell[]>( m_blockSize );

        // Add the empty cells as pointers to the array of slots
        for( unsigned int i= 0; i!= m_blockSize; i++ ) {
            m_slots.emplace_back( block.get()+ i );
        }

        // Add the block to the array of blocks
        m_blocks.push( std::move(block) );
    }

public:
    explicit AllocArray( const unsigned int s )
            : m_blockSize( s ) {
        m_slots.reserve( m_blockSize );
        addBlock();
    }

    AllocArray( const AllocArray& )= delete;

    template< typename T_Element, typename ... T_Args >
    T_Element* create( T_Args&& ... args ) {
        // Assert size and alignment
        static_assert(sizeof(T_Element) <= T_CellSize, "Element is too large for Object Pool cell.");
        static_assert(alignof(T_Element) <= T_CellAlign, "Element alignment is not compatible with Object Pool cell.");

        T_Cell* cell;
        {
            // Lock the array
            std::lock_guard<T_Mutex> lock(m_mutex);

            // Add new block, if no cells are left
            if (m_slots.empty()) {
                addBlock();
            }

            // Fetch empty cell
            cell = m_slots.back();
            m_slots.pop_back();
        }

        // Construct new object in the empty cell
        return new(cell) T_Element( std::forward<T_Args>(args)... );
    }

    template< typename T_Element >
    void free( T_Element* ptr ) {
        // Assert size and alignment
        static_assert(sizeof(T_Element) <= T_CellSize, "Element is too large for Object Pool cell.");
        static_assert(alignof(T_Element) <= T_CellAlign, "Element alignment is not compatible with Object Pool cell.");

        // Destruct the object
        ptr->~T_Element();

        // Return the cell
        std::lock_guard<T_Mutex> lock(m_mutex);
        m_slots.emplace_back( reinterpret_cast<T_Cell*>(ptr ) );
    }

    std::size_t space() const {
        return m_slots.size();
    }
};

namespace ObjectPoolDetail {
    /**
     * Class to determine the largest of all sizes of the provided types
     * @tparam T_X - Types provided
     */
    template<typename ... T_X>
    struct requiredSize;

    template<typename T_Type>
    struct requiredSize<T_Type> {
        static constexpr size_t value = sizeof(T_Type);
    };

    template<typename T_Type, typename ... T_Others>
    struct requiredSize<T_Type, T_Others...> {
        static constexpr size_t value =
                sizeof(T_Type) > requiredSize<T_Others...>::value ? sizeof(T_Type) : requiredSize<T_Others...>::value;
    };

    /**
     * Class to determine the largest of all alignments of the provided types
     * @tparam T_X - Types provided
     */
    template<typename ... T_X>
    struct requiredAlign;

    template<typename T_Type>
    struct requiredAlign<T_Type> {
        static constexpr size_t value = alignof(T_Type);
    };

    template<typename T_Type, typename ... T_Others>
    struct requiredAlign<T_Type, T_Others...> {
        static constexpr size_t value = alignof(T_Type) > requiredAlign<T_Others...>::value ? alignof(T_Type)
                                                                                            : requiredAlign<T_Others...>::value;
    };

}


/**
 * Templated Object Pool Class
 * Like its parent it holds objects of arbitrary type in a pool
 *
 * @tparam T_Elements - Pack of types to be stored
 */
template<typename ... T_Elements>
class ObjectPool : public AllocArray< ObjectPoolDetail::requiredSize<T_Elements...>::value,
                                      ObjectPoolDetail::requiredAlign<T_Elements...>::value > {
public:
    explicit ObjectPool(const unsigned int s)
            : AllocArray< ObjectPoolDetail::requiredSize<T_Elements...>::value,
                          ObjectPoolDetail::requiredAlign<T_Elements...>::value >( s ) {}

};


/**
 * Templated Synchronised Object Pool Class
 * Like its parent it holds objects of arbitrary type in a pool
 * It uses a standard mutex to synchronize creation and deletion
 * of objects
 *
 * @tparam T_Elements - Pack of types to be stored
 */
template<typename ... T_Elements>
class SyncObjectPool : public AllocArray< ObjectPoolDetail::requiredSize<T_Elements...>::value,
                                          ObjectPoolDetail::requiredAlign<T_Elements...>::value,
                                          std::mutex > {
public:
    explicit SyncObjectPool(const unsigned int s)
            : AllocArray< ObjectPoolDetail::requiredSize<T_Elements...>::value,
            ObjectPoolDetail::requiredAlign<T_Elements...>::value,
            std::mutex                                                          >( s ) {}

};




/**
 * Templated Static Object Pool Singelton Class
 * As this object pool defines its own unique_pointer type to handle
 * the life cycle of its objects the address of the pool has to be globally
 * available in order to delete the objects using a static functor
 * Therefore the objects stored do not need a pointer back to the instance
 * of the pool
 *
 * @tparam T_PointerBase - Base class type of the stored objects
 * @tparam T_Elements    - Pack of types to be stored
 */
template< typename T_PointerBase, typename ... T_Elements >
class StaticObjectPool : protected SyncObjectPool< T_Elements... > {
private:

    /**
     * Internal Delete Functor Class
     * Used as deleter type to destruct stored objects at the end of their life cycle
     */
    class DeleteFunctor {
    public:
        void operator()( T_PointerBase* el ) {
            StaticObjectPool<T_PointerBase, T_Elements...>::m_instance->free( el );
        }
    };

    static std::unique_ptr<StaticObjectPool<T_PointerBase, T_Elements...>> m_instance;

public:
    static constexpr unsigned int T_defaultBlockSize= 100;

    using T_Pointer= std::unique_ptr<T_PointerBase, DeleteFunctor>;

    explicit StaticObjectPool( const unsigned int s )
            : SyncObjectPool<T_Elements...>( s ) {}


    static void initialize( const unsigned int s= T_defaultBlockSize ) {
        m_instance= std::make_unique<StaticObjectPool<T_PointerBase, T_Elements...>>( s );
    }

    static inline StaticObjectPool<T_PointerBase, T_Elements...>& get() {
        return *m_instance;
    }

    using SyncObjectPool<T_Elements...>::space;

    template< typename T_Element, typename ...T_Args >
    T_Pointer alloc( T_Args&& ... args ) {

        // Assert that type can be stored as pointer
        static_assert( (std::is_base_of< T_PointerBase, T_Element >::value ||
                        std::is_same< T_PointerBase, T_Element>::value),
                       "Object Pool cannot allocate object, which is not of T_PointerBase inheritance." );

        // Create new object and return as T_Pointer
        T_Element* el= this-> template create<T_Element>( std::forward<T_Args>( args )... );
        return T_Pointer( el );
    };
};


template< typename T_PointerBase, typename ... T_Elements >
std::unique_ptr<StaticObjectPool<T_PointerBase, T_Elements...>> StaticObjectPool<T_PointerBase, T_Elements...>::m_instance;


#endif //PROMISE_OBJECTPOOL_H
