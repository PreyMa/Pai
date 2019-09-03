//
// Created by Matthias Preymann on 02.09.2019.
//

#ifndef PROMISE_POOLDEFS_H
#define PROMISE_POOLDEFS_H

#include "ObjectPool.h"

/**
 * Workaround as nested types cannot be forward declared
 */
namespace PoolDefs {
    using T_EventPool= SyncObjectPool< std::aligned_storage<96, sizeof(void*)>::type >;
}


#endif //PROMISE_POOLDEFS_H
