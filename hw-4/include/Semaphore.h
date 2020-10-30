#ifndef SHMEM_SEMAPHORE_H
#define SHMEM_SEMAPHORE_H

#include <semaphore.h>
#include <sys/sem.h>
#include <stdexcept>
#include <pthread.h>

class Semaphore {
private:
    sem_t sem_;

public:
    Semaphore();

    void post();
    void wait();
    void destroy();

    friend class SemLock;
};

class SemLock {
private:
    Semaphore * sem_;
public:
    explicit SemLock(Semaphore * sem);
    ~SemLock();
};

#endif //SHMEM_SEMAPHORE_H
