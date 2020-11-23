#include "shmem.h"
#include <unistd.h>
#include <sys/wait.h>

using SharedString = std::basic_string<char, std::char_traits<char>, shmem::SharedAllocator<char>>;

int main() {
    shmem::SharedMem mem(1, 512);
    shmem::SharedMap<int, SharedString> mymap(mem);
    pid_t pid = fork();
    if (!pid) {
        mymap.insert(0, SharedString
                ("-------------------------------------------------------------------|", mem.get_allocator()));
        sleep(2);
        mymap.update(0, SharedString
                ("===================================================================|", mem.get_allocator()));
        sleep(2);
        mymap.insert(1, SharedString
                ("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++|", mem.get_allocator()));
        sleep(2);
        mymap.insert(1, SharedString
                ("*******************************************************************|", mem.get_allocator()));
    } else {
        sleep(1);
        std::cout <<  mymap.get(0) << std::endl;
        sleep(2);
        std::cout <<  mymap.get(0) << std::endl;
        sleep(2);
        std::cout <<  mymap.get(1) << std::endl;
        sleep(2);
        std::cout <<  mymap.get(1) << std::endl;
        waitpid(pid, nullptr, 0);
        mymap.destroy();
        mem.destroy();
    }
    return 0;
}
