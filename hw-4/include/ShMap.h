#ifndef SHMEM_SHMAP_H
#define SHMEM_SHMAP_H

#include <map>
#include <iterator>
#include "SharedAllocator.h"
#include "Semaphore.h"

template <typename Key, typename T>
class SharedMap {
private:
    std::map<Key, T, std::less<Key>, SharedAllocator<std::pair<const Key,T>>> *map_;
    Semaphore *sem_;
    char *mmap_;
    size_t mem_size_;
    SharedAllocator <std::pair<const Key, T>> *shared_allocator_;

public:
    SharedMap(size_t block_size, size_t block_count) {
        ShMemState tmp;
        size_t header_size = sizeof(tmp) + sizeof(*(tmp.used_blocks_table)) * block_count;
        header_size = (header_size / block_size + (header_size % block_size ? 1 : 0)) * block_size;
        mem_size_ = block_count * block_size + header_size;
        mmap_ = static_cast<char *>(::mmap(nullptr, mem_size_,
                                           PROT_READ | PROT_WRITE,
                                           MAP_ANONYMOUS | MAP_SHARED,
                                           -1, 0));

        if (mmap_ == MAP_FAILED) {
            throw std::runtime_error("Mmap error\n");
        }

        ShMemState *state = new(mmap_) ShMemState{};

        state->block_size = block_size;
        state->blocks_count = block_count;
        sem_ = &state->sem;
        state->used_blocks_table = mmap_ + sizeof(*state);
        state->first_block = mmap_ + header_size;
        ::memset(state->used_blocks_table, FREE_BLOCK, state->blocks_count);
        shared_allocator_ = new SharedAllocator<std::pair<const Key, T>>(mmap_);
        map_ = new(state->first_block) std::map<Key, T, std::less<Key>, SharedAllocator<std::pair<const Key,T>>> (*shared_allocator_);
    }

    ~SharedMap() {
        sem_->destroy();
        ::munmap(mmap_, mem_size_);
    }

    void update(Key & key, T & value) {}

    void insert(Key & key, T & value) {
        //SemLock tmp(sem_);
        map_->insert(key, value);
    }

    auto end() {
        //SemLock tmp(sem_);
        return map_->end();
    }

    auto get(Key & key) {
        //SemLock tmp(sem_);
        return map_->find(key);
    }

    size_t remove(Key & key) {
        //SemLock tmp(sem_);
        return map_->erase(key);
    }
};

#endif //SHMEM_SHMAP_H
