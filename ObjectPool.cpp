//
// Created by Matthias Preymann on 28.07.2019.
//

#include "ObjectPool.h"

void ObjectPoolDetail::PoolPointerDeleteFunctor::operator()(PooledObject *el)  {
    el->freeSelf();
}

HeapAllocator heapAlloc;