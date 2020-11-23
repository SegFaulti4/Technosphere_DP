#ifndef SHMEM_SHMEMEXCEPTION_H
#define SHMEM_SHMEMEXCEPTION_H

#include <stdexcept>

namespace shmem {

    class ShmemException : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

}

#endif //SHMEM_SHMEMEXCEPTION_H
