#include <sys/ptrace.h> /* ptrace */
#include "memory.hpp"


uint64_t poc::mem::read_memory(pid_t child_pid, uint64_t address) {
    return ptrace(PTRACE_PEEKDATA, child_pid, address, nullptr);
}

std::string poc::mem::read_memory_str(pid_t child_pid, uint64_t address) {
    char c;
    std::string results;
    while(c){
     c=(char)ptrace(PTRACE_PEEKTEXT, child_pid, address, nullptr);
     address++;
     //results= results + std::string(c);
     char cc[] = {c};
     results= results + (std::string)cc;
    }
    return results;
}

void poc::mem::write_memory(pid_t child_pid, uint64_t address, uint64_t value) {
    ptrace(PTRACE_POKEDATA, child_pid, address, value);
}