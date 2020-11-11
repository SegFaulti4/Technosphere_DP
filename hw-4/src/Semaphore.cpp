#include "Semaphore.h"

namespace shmem {

    Semaphore::Semaphore() {
        if (::sem_init(&sem_, 1, 1) == -1) {
            throw std::runtime_error("Semaphore init error\n");
        }
    }

    void Semaphore::destroy() {
        ::sem_destroy(&sem_);
    }

    void Semaphore::post() {
        if (::sem_post(&sem_) == -1) {
            throw std::runtime_error("Semaphore post error\n");
        }
    }

    void Semaphore::wait() {
        if (::sem_wait(&sem_) == -1) {
            throw std::runtime_error("Semaphore wait error\n");
        }
    }

    SemLock::SemLock(Semaphore &sem) : sem_(sem) {
        sem_.wait();
    }

    SemLock::~SemLock() {
        sem_.post();
    }

}
