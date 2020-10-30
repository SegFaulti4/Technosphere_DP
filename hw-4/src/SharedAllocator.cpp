#include "../include/SharedAllocator.h"

size_t get_size_in_blocks(size_t bytes, size_t block_size) {
    float blocks_needed = bytes / static_cast<float>(block_size);
    return std::ceil(blocks_needed);
}

size_t find_free_blocks(size_t blocks_count, const std::string_view &used_table) {
    std::string pattern(blocks_count, FREE_BLOCK);
    size_t pos = used_table.find(pattern);
    if (pos == std::string::npos) {
        throw std::bad_alloc{};
    }
    return pos;
}

template<class T, class U>
bool operator==(const SharedAllocator<T> &a, const SharedAllocator<U> &b) {
    return a.state_ == b.state_;
}

template<class T, class U>
bool operator!=(const SharedAllocator<T> &a, const SharedAllocator<U> &b) {
    return a.state_ != b.state_;
}
