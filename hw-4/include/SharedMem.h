#ifndef SHMEM_SHAREDMEM_H
#define SHMEM_SHAREDMEM_H

#include <sys/types.h>
#include <memory>
#include "Semaphore.h"
#include "SharedAllocator.h"

namespace shmem {

    class SharedMem {
    private:
        char *mmap_;
        size_t size_;
        Semaphore *sem_;
        std::unique_ptr<SharedAllocator < char>> alloc_;

    public:
        SharedMem(size_t block_size, size_t block_count);

        ~SharedMem();

        auto get_allocator() {
            return *alloc_;
        }

        template<typename Key, typename T>
        friend
        class SharedMap;
    };

}

#endif //SHMEM_SHAREDMEM_H
