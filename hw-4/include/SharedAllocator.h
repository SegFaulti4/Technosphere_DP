#ifndef HW_4_SHAREDALLOCATOR_H
#define HW_4_SHAREDALLOCATOR_H

#include <iostream>
#include <memory>
#include <cmath>
#include <cstring>
#include <functional>
#include <sys/mman.h>
#include "Semaphore.h"

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

template<typename T>
class SharedAllocator {
public:
    char *mmap_;

    typedef T value_type;

    explicit SharedAllocator(char *mmap)
            : mmap_ {mmap} {}

    template<class U>
    SharedAllocator(const SharedAllocator<U>& other) noexcept {
        mmap_ = other.mmap_;
    }

    T* allocate(std::size_t n) {
        ShMemState* state = reinterpret_cast<ShMemState*>(mmap_);

        size_t blocks_needed = get_size_in_blocks(sizeof(T) * n, state->block_size);
        std::string_view table{state->used_blocks_table, state->blocks_count};
        size_t blocks_pos = find_free_blocks(blocks_needed, table);
        ::memset(state->used_blocks_table + blocks_pos, USED_BLOCK, blocks_needed);
        return reinterpret_cast<T*>(state->first_block + blocks_pos * state->block_size);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        ShMemState* state = reinterpret_cast<ShMemState*>(mmap_);

        size_t offset = (reinterpret_cast<char*>(p) - state->first_block) / state->block_size;
        size_t blocks_count = get_size_in_blocks(sizeof(T) * n, state->block_size);
        ::memset(state->used_blocks_table + offset, FREE_BLOCK, blocks_count);
    }
};

template<class T, class U>
bool operator==(const SharedAllocator<T> &a, const SharedAllocator<U> &b);

template<class T, class U>
bool operator!=(const SharedAllocator<T> &a, const SharedAllocator<U> &b);

using CharAlloc = SharedAllocator<char>;
using ShString = std::basic_string<char, std::char_traits<char>, CharAlloc>;
using ShUPtr = std::unique_ptr<char, std::function<void(char *)>>;

#endif //HW_4_SHAREDALLOCATOR_H
