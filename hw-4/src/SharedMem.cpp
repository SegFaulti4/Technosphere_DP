#include "SharedMem.h"

namespace shmem {

    SharedMem::SharedMem(size_t block_size, size_t block_count) : alloc_(nullptr) {
        size_t header_size = sizeof(ShMemState) +
                             sizeof(*(std::declval<ShMemState>().used_blocks_table)) * block_count;
        header_size = (header_size / block_size + (header_size % block_size ? 1 : 0)) * block_size;
        size_ = block_count * block_size + header_size;
        mmap_ = static_cast<char *>(::mmap(nullptr, size_,
                                           PROT_READ | PROT_WRITE,
                                           MAP_ANONYMOUS | MAP_SHARED,
                                           -1, 0));

        if (mmap_ == MAP_FAILED) {
            throw ShmemException("Mmap error");
        }

        state_ = new(mmap_) ShMemState{};
        sem_ = &(state_->sem);

        state_->block_size = block_size;
        state_->blocks_count = block_count;
        state_->used_blocks_table = mmap_ + sizeof(*state_);
        state_->first_block = mmap_ + header_size;
        ::memset(state_->used_blocks_table, FREE_BLOCK, state_->blocks_count);
        alloc_.state_ = state_;
    }

    void SharedMem::destroy() noexcept {
        try {
            sem_->destroy();
        } catch (std::exception &) {}
        ::munmap(mmap_, size_);
    }

    SharedAllocator<char> & SharedMem::get_allocator() {
        return alloc_;
    }

}
