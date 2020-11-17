#include "Semaphore.h"

namespace shmem {

    Semaphore::Semaphore() {
        if (::sem_init(&sem_, 1, 1) == -1) {
            throw ShmemException("Semaphore init error\n");
        }
    }

    void Semaphore::destroy() {
        if (::sem_destroy(&sem_) == -1) {
            throw ShmemException("Semaphore destroy error\n");
        }
    }

    void Semaphore::post() {
        if (::sem_post(&sem_) == -1) {
            throw ShmemException("Semaphore post error\n");
        }
    }

    void Semaphore::wait() {
        if (::sem_wait(&sem_) == -1) {
            throw ShmemException("Semaphore wait error\n");
        }
    }

    SemLock::SemLock(Semaphore &sem) : sem_(sem) {
        sem_.wait();
    }

    SemLock::~SemLock() {
        try {
            sem_.post();
        } catch(std::exception &) {}
    }

}
