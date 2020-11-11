#ifndef SHMEM_SHMAP_H
#define SHMEM_SHMAP_H

#include <map>
#include <iterator>
#include "SharedAllocator.h"
#include "SharedMem.h"
#include "Semaphore.h"

namespace shmem {

    template<typename Key, typename T>
    class SharedMap {
    private:
        std::map<Key, T, std::less<Key>, SharedAllocator < std::pair<const Key, T>>> *map_;
        Semaphore *sem_;

    public:
        explicit SharedMap(SharedMem &mem) {
            sem_ = mem.sem_;
            SharedAllocator<std::pair<const Key, T>> tmp(mem.get_allocator());
            map_ = new(tmp.state_->first_block)
                    std::map<Key, T, std::less<Key>, SharedAllocator<std::pair<const Key, T>>>(tmp);
            tmp.allocate(sizeof(*map_));
        }

        ~SharedMap() = default;

        void update(const Key &key, const T &value) {
            if (get(key) == end()) {
                throw std::runtime_error("No matching element");
            }
            SemLock tmp(*sem_);
            (*map_)[key] = value;
        }

        void insert(const Key &key, const T &value) {
            if (get(key) == end()) {
                map_->insert(std::pair<Key, T>(key, value));
            } else {
                SemLock tmp(*sem_);
                map_->insert(std::pair<Key, T>(key, value));
            }
        }

        auto end() {
            SemLock tmp(*sem_);
            return map_->end();
        }

        auto get(const Key &key) {
            SemLock tmp(*sem_);
            return map_->find(key);
        }

        size_t remove(Key &key) {
            SemLock tmp(*sem_);
            return map_->erase(key);
        }
    };

}

#endif //SHMEM_SHMAP_H
