#ifndef HW_4_SHAREDALLOCATOR_H
#define HW_4_SHAREDALLOCATOR_H

#include <iostream>
#include <memory>
#include <cstring>
#include <cmath>
#include <functional>
#include <sys/mman.h>
#include "Semaphore.h"

namespace shmem {

    constexpr char USED_BLOCK = '1';
    constexpr char FREE_BLOCK = '0';

    size_t get_size_in_blocks(size_t bytes, size_t block_size);

    size_t find_free_blocks(size_t blocks_count, const std::string_view &used_table);

    struct ShMemState {
        size_t blocks_count;
        size_t block_size;
        Semaphore sem;
        char *used_blocks_table;
        char *first_block;
    };

    template<class T>
    class SharedAllocator {
    private:
        ShMemState *state_;

    public:
        typedef T value_type;

        explicit SharedAllocator(ShMemState *state) {
            state_ = state;
        }

        template<class U>
        SharedAllocator(const SharedAllocator<U> &other) noexcept {
            state_ = other.state_;
        }

        T *allocate(std::size_t n) {
            SemLock(state_->sem);
            size_t blocks_needed = get_size_in_blocks(sizeof(T) * n, state_->block_size);
            std::string_view table{state_->used_blocks_table, state_->blocks_count};
            size_t blocks_pos = find_free_blocks(blocks_needed, table);
            ::memset(state_->used_blocks_table + blocks_pos, USED_BLOCK, blocks_needed);
            return reinterpret_cast<T *>(state_->first_block + blocks_pos * state_->block_size);
        }

        void deallocate(T *p, std::size_t n) noexcept {
            SemLock(state_->sem);
            size_t offset = (reinterpret_cast<char *>(p) - state_->first_block) / state_->block_size;
            size_t blocks_count = get_size_in_blocks(sizeof(T) * n, state_->block_size);
            ::memset(state_->used_blocks_table + offset, FREE_BLOCK, blocks_count);
        }

        friend class SharedMem;

        template<class V>
        friend
        class SharedAllocator;

        template<typename Key, typename V>
        friend
        class SharedMap;
    };

    template<class T, class U>
    bool operator==(const SharedAllocator<T> &a, const SharedAllocator<U> &b);

    template<class T, class U>
    bool operator!=(const SharedAllocator<T> &a, const SharedAllocator<U> &b);

}

#endif //HW_4_SHAREDALLOCATOR_H
