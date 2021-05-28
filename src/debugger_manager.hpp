#ifndef H_DEBUGGER
#define H_DEBUGGER

#include <string> 
#include <fcntl.h>       // O_RDONLY 
#include <linux/types.h> // pid_t
#include <sys/user.h>    // user_regs_struct 
#include <signal.h>
#include <capstone/capstone.h>
#include <libelfin/elf/elf++.hh>
#include <libelfin/dwarf/dwarf++.hh>
#include "common.hpp"
#include "compile_unit.hpp"
#include "breakpoint.hpp"


namespace poc {
namespace debugger {
    void run_debuggee(const std::string& program_path, char* cmd[]);

    class Debugger: public poc::cu::CompileUnitManager {
    public:
        Debugger (
            std::string program_path, std::string program_argv, pid_t chid_pid): 
                m_program_path{program_path}, m_program_argv{program_argv}, m_child_pid{chid_pid} {

            auto fd = open(m_program_path.c_str(), O_RDONLY);
            m_elf = elf::elf{elf::create_mmap_loader(fd)};
            m_dwarf = dwarf::dwarf{dwarf::elf::create_loader(m_elf)};

            try{
                m_dwarf = dwarf::dwarf{dwarf::elf::create_loader(m_elf)};
                m_debug_info = true;
            } catch(...) {}
        }

    private:
        std::string m_program_path;
        std::string m_program_argv;
        pid_t m_child_pid;

        dwarf::dwarf m_dwarf;
        elf::elf m_elf;

        std::vector<symbol_t> m_symbols;
        std::unordered_map<std::intptr_t, symbol_t> m_symbols_map;

        bool m_debug_info = false;

    public:
        uint64_t get_program_counter();
        void set_program_counter(uint64_t pc);        

        void get_symbols();
        bool get_symbol_by_value(std::intptr_t symbol_value, symbol_t& symbol);
        void print_symbols();

        void run();
        void continue_execution(); 
        void wait_for_signal();
        siginfo_t get_signal_info();

        void get_breakpoint_function_info(poc::bp::Breakpoint& breakpoint);
        void get_breakpoint_branch_info(poc::bp::Breakpoint& breakpoint);
        void get_breakpoint_info(poc::bp::Breakpoint& breakpoint);
        void step_over_breakpoint();
        void set_all_function_breakpoints();
    };

}
}


#endif