#ifndef H_MEMORY
#define H_MEMORY

#include <cctype>
#include <string>


namespace poc {
    namespace mem {
        std::string read_memory_str(pid_t child_pid, uint64_t address);

        uint64_t read_memory(pid_t child_pid, uint64_t address);

        void write_memory(pid_t child_pid, uint64_t address, uint64_t value);
    }
}

#endif