#include "SharedMem.h"

namespace shmem {

    SharedMem::SharedMem(size_t block_size, size_t block_count) {
        size_t header_size = sizeof(ShMemState) +
                             sizeof(*(std::declval<ShMemState>().used_blocks_table)) * block_count;
        header_size = (header_size / block_size + (header_size % block_size ? 1 : 0)) * block_size;
        size_ = block_count * block_size + header_size;
        mmap_ = static_cast<char *>(::mmap(nullptr, size_,
                                           PROT_READ | PROT_WRITE,
                                           MAP_ANONYMOUS | MAP_SHARED,
                                           -1, 0));

        if (mmap_ == MAP_FAILED) {
            throw std::runtime_error("Mmap error\n");
        }

        auto *state = new(mmap_) ShMemState{};

        state->block_size = block_size;
        state->blocks_count = block_count;
        sem_ = new(&state->sem) Semaphore();
        state->used_blocks_table = mmap_ + sizeof(*state);
        state->first_block = mmap_ + header_size;
        ::memset(state->used_blocks_table, FREE_BLOCK, state->blocks_count);
        alloc_ = std::make_unique<SharedAllocator<char>>(state);
    }

    SharedMem::~SharedMem() {
        sem_->destroy();
        ::munmap(mmap_, size_);
    }

}
