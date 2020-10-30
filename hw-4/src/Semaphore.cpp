#include "../include/Semaphore.h"

Semaphore::Semaphore() {
    if (::sem_init(&sem_, 1, 0) == -1) {
        throw std::runtime_error("Semaphore init error\n");
    }
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

void Semaphore::destroy() {
    ::sem_destroy(&sem_); // используется только в ~ShMap
}

SemLock::SemLock(Semaphore * sem) {
    sem_ = sem;
    sem_->wait();
}

SemLock::~SemLock() {
    ::sem_post(&(sem_->sem_));
}
