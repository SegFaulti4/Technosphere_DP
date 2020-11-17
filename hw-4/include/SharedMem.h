#ifndef SHMEM_SHAREDMEM_H
#define SHMEM_SHAREDMEM_H

#include <sys/types.h>
#include <memory>
#include "Semaphore.h"
#include "SharedAllocator.h"
#include "ShmemException.h"

namespace shmem {

    class SharedMem {
    private:
        char *mmap_;
        ShMemState *state_;
        size_t size_;
        Semaphore *sem_;
        SharedAllocator<char> alloc_;

    public:
        SharedMem(size_t block_size, size_t block_count);

        void destroy() noexcept;
        SharedAllocator<char> & get_allocator();

        template<typename Key, typename T>
        friend class SharedMap;
    };

}

#endif //SHMEM_SHAREDMEM_H
