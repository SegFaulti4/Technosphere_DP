#ifndef SHMEM_SHMAP_H
#define SHMEM_SHMAP_H

#include <map>
#include <iterator>
#include "SharedAllocator.h"
#include "SharedMem.h"
#include "Semaphore.h"
#include "ShmemException.h"

namespace shmem {

    template<typename Key, typename T>
    class SharedMap {
    private:
        using CustomAllocMap = std::map<Key, T, std::less<Key>, SharedAllocator<std::pair<const Key, T>>>;

        CustomAllocMap *map_;
        Semaphore *sem_;

    public:
        explicit SharedMap(SharedMem & mem) {
            sem_ = new(SharedAllocator<Semaphore>(mem.get_allocator()).allocate(1))
                    Semaphore();
            map_ = new(SharedAllocator<CustomAllocMap>(mem.get_allocator()).allocate(1))
                    CustomAllocMap(mem.get_allocator());
        }

        ~SharedMap() = default;

        int count(const Key & key) {
            SemLock tmp(*sem_);
            return map_->count(key);
        }

        size_t remove(const Key & key) {
            SemLock tmp(*sem_);
            return map_->erase(key);
        }

        void insert(const Key & key, const T & value) {
            remove(key);
            SemLock tmp(*sem_);
            map_->insert(std::make_pair(key, value));
        }

        void update(const Key & key, const T & value) {
            if (count(key) == 0) {
                throw ShmemException("No elements found to update");
            }
            insert(key, value);
        }

        T get(const Key & key) {    // можно было вызвать count, но в таком случае мы ходим по map'у дважды
            SemLock tmp(*sem_);
            auto res = map_->find(key);
            if (res == map_->end()) {
                throw ShmemException("No elements found to get");
            }
            return res->second;
        }

        void destroy() noexcept {
            try {
                sem_->destroy();
            } catch (std::exception &) {}
        }
    };

}

#endif //SHMEM_SHMAP_H
