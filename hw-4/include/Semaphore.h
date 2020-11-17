#ifndef SHMEM_SEMAPHORE_H
#define SHMEM_SEMAPHORE_H

#include <semaphore.h>
#include <sys/sem.h>
#include <stdexcept>
#include <pthread.h>
#include "ShmemException.h"

namespace shmem {

    class Semaphore {
    private:
        sem_t sem_;

    public:
        Semaphore();

        void destroy();

        void post();

        void wait();
    };

    class SemLock {
    private:
        Semaphore &sem_;
    public:
        explicit SemLock(Semaphore &sem);

        ~SemLock();
    };

}

#endif //SHMEM_SEMAPHORE_H
