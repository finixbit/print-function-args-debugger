#include <iostream>
#include <unistd.h> /* fork() */
#include <sys/personality.h> // personality
#include "debugger_manager.hpp"
using namespace poc;


int main(int argc, char* argv[]) { 
    char usage_banner[] = "usage: ./debugger [<cmd>]\n";
    if (argc < 2) {
        std::cerr << usage_banner;
        return -1;
    }

    auto program_path = argv[1];
    auto child_pid = fork();

    switch(child_pid) {
        case -1: 
            std::cerr << "error forking\n"; 
            break;
        case  0:
            // disable address-space-layout randomization (ASLR)
            personality(ADDR_NO_RANDOMIZE);

            // provide legacy virtual address space layout
            personality(ADDR_COMPAT_LAYOUT);

            poc::debugger::run_debuggee(program_path, argv+1);
            break;
        default: 
            poc::debugger::Debugger debugger_instance{program_path, program_path, child_pid};
            debugger_instance.run();
            break;
    }
    return 0;
}