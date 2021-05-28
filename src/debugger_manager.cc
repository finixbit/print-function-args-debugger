#include <iostream>
#include <sys/ptrace.h> /* ptrace */
#include <sys/wait.h> /* wait */
#include <sys/user.h> /* user_regs_struct */
#include <inttypes.h> /* PRIx64 */
#include <sys/stat.h> /* For the size of the file. , fstat */
#include <sys/prctl.h> // prctl
#include <unistd.h> // execvp
#include "debugger_manager.hpp"
#include "parameter_extractor.hpp"
#include "registers.hpp"
#include "helpers.hpp"
#include "disassembler.hpp"
#include "breakpoint.hpp"
using namespace poc;


void poc::debugger::run_debuggee(const std::string& program_path, char* cmd[]) {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        std::cerr << "Ptrace Error.\n";
        return;
    }

    long ptrace_opts;
    ptrace_opts = PTRACE_O_TRACECLONE|PTRACE_O_TRACEFORK|PTRACE_O_TRACEEXEC|PTRACE_O_TRACEEXIT|PTRACE_O_EXITKILL;
    ptrace(PTRACE_SETOPTIONS, 0, 0, ptrace_opts);

    prctl(PR_SET_PDEATHSIG, SIGHUP);
    execvp(cmd[0], cmd);
}

uint64_t poc::debugger::Debugger::get_program_counter() {
    user_regs_struct regs = poc::reg::get_registers(poc::debugger::Debugger::m_child_pid);
    return (uint64_t)regs.rip;
}

void poc::debugger::Debugger::set_program_counter(uint64_t pc) {
    user_regs_struct regs = poc::reg::get_registers(poc::debugger::Debugger::m_child_pid);

    #ifdef __x86_64__
        regs.rip = pc;
    #else
        regs.eip = pc;
    #endif
    ptrace(PTRACE_SETREGS, poc::debugger::Debugger::m_child_pid, nullptr, &regs);
}

void poc::debugger::Debugger::get_symbols() {
    for (auto &sec : poc::debugger::Debugger::m_elf.sections()) {
        if ((sec.get_hdr().type != elf::sht::symtab) && 
            (sec.get_hdr().type != elf::sht::dynsym))
            continue;

        int index = 0;
        for (auto sym : sec.as_symtab()) {
            auto &d = sym.get_data();

            symbol_t symbol;
            symbol.symbol_num   = index++;
            symbol.symbol_value = d.value;
            symbol.symbol_name  = sym.get_name();
            symbol.symbol_size  = d.size;
            symbol.symbol_binding = elf::to_string(d.binding());
            symbol.symbol_type  = elf::to_string(d.type());
            symbol.symbol_table = sec.get_name();
            symbol.symbol_index = elf::enums::to_string(d.shnxd);

            if((symbol.symbol_type == "func") && (symbol.symbol_value > 0)) {
                poc::debugger::Debugger::m_symbols_map[symbol.symbol_value] = symbol;
            }

            poc::debugger::Debugger::m_symbols.push_back(symbol);
        }
    }
}

bool poc::debugger::Debugger::get_symbol_by_value(std::intptr_t symbol_value, symbol_t& symbol) {
    for (auto &sym : poc::debugger::Debugger::m_symbols) {
        if(symbol_value == sym.symbol_value) {
            symbol = sym;
            return true;
        }
    }
    return false;
}

void poc::debugger::Debugger::print_symbols() {
    printf("%6s: %-16s %-5s %-7s %-7s %-5s %s\n",
        "Num", "Value", "Size", "Type", "Binding", "Index", "Name");

    for (auto &symbol : poc::debugger::Debugger::m_symbols) {
        if(symbol.symbol_type == "func")
            printf("%6d: %016" PRIx64 " %5" PRId64 " %-7s %-7s %5s %s\n",
                symbol.symbol_num, 
                symbol.symbol_value, 
                symbol.symbol_size,
                symbol.symbol_type.c_str(),
                symbol.symbol_binding.c_str(),
                symbol.symbol_index.c_str(),
                symbol.symbol_name.c_str());     
    }
}

void poc::debugger::Debugger::run() {
    poc::debugger::Debugger::wait_for_signal();

    if(!m_debug_info) {
        std::cout << "[*] No Symbols found! ... Exiting" << std::endl;
        exit(1);
    }

    poc::debugger::Debugger::get_symbols();
    poc::disasm::get_disassembled_instructions(poc::debugger::Debugger::m_program_path);
    poc::cu::CompileUnitManager::get_compile_units(poc::debugger::Debugger::m_elf);
    set_all_function_breakpoints();

    // print_compile_units();
    // print_symbols();

    poc::debugger::Debugger::continue_execution();
}

void poc::debugger::Debugger::continue_execution() {
    while(1) { 
        ptrace(PTRACE_CONT, poc::debugger::Debugger::m_child_pid, nullptr, nullptr);
        poc::debugger::Debugger::wait_for_signal();
        poc::debugger::Debugger::step_over_breakpoint();
    }
}

void poc::debugger::Debugger::wait_for_signal() {
    int wait_status = 0;
    auto options = 0;
    waitpid(poc::debugger::Debugger::m_child_pid, &wait_status, options);

    if (WIFSTOPPED(wait_status)) {
        if (WSTOPSIG(wait_status) == 5) {
            return;
            
        } else if(WIFEXITED(wait_status) || (WIFSIGNALED(wait_status) && WTERMSIG(wait_status) == SIGKILL)) {
            std::cout << wait_status << std::endl;
            std::cout << "[+] process " << poc::debugger::Debugger::m_child_pid << " terminated" << std::endl;
            exit(1);
            
        } else {
            printf("Child got a signal: %s - %d\n", strsignal(WSTOPSIG(wait_status)), WSTOPSIG(wait_status));
            exit(1);
        }            
    }
}

void poc::debugger::Debugger::step_over_breakpoint() {
    // - 1 because execution will go past the breakpoint
    auto possible_breakpoint_location = poc::debugger::Debugger::get_program_counter() - 1;

    if (poc::bp::Breakpoint::m_breakpoints.count(possible_breakpoint_location)) {
        auto& bp = poc::bp::Breakpoint::m_breakpoints[possible_breakpoint_location];

        if (bp.m_enabled) {
            poc::debugger::Debugger::get_breakpoint_info(bp);

            auto previous_instruction_address = possible_breakpoint_location;
            poc::debugger::Debugger::set_program_counter(previous_instruction_address);

            bp.disable_breakpoint(m_child_pid);
            ptrace(PTRACE_SINGLESTEP, poc::debugger::Debugger::m_child_pid, nullptr, nullptr);
            poc::debugger::Debugger::wait_for_signal();

            bp.enable_breakpoint(m_child_pid);
        }
    } 
}

siginfo_t poc::debugger::Debugger::get_signal_info() {
    siginfo_t info;
    ptrace(PTRACE_GETSIGINFO, poc::debugger::Debugger::m_child_pid, nullptr, &info);
    return info;
}

void poc::debugger::Debugger::get_breakpoint_function_info(poc::bp::Breakpoint& breakpoint) {
    poc::bp::Breakpoint& bp    = breakpoint;
    compile_unit_t& cu  = poc::cu::CompileUnitManager::m_compile_units[bp.m_cu_index];
    datatype_t& func    = cu.functions[bp.m_fn_index];

    sdb_function_t sdb_func = poc::cu::CompileUnitManager::generate_function_payload(m_child_pid, cu, func, bp.m_cu_index, bp.m_fn_index);
    poc::cu::CompileUnitManager::print_function_payload(sdb_func);
}

void poc::debugger::Debugger::get_breakpoint_branch_info(poc::bp::Breakpoint& breakpoint) {
    poc::bp::Breakpoint& bp = breakpoint; 
    // todo 
}

void poc::debugger::Debugger::get_breakpoint_info(poc::bp::Breakpoint& breakpoint) {
    std::cout << "\n............................................" << std::endl;
    std::cout << "[*] 0x";
    std::cout << std::hex << breakpoint.m_bp_address << std::dec << std::endl;
    
    if(breakpoint.m_breakpoint_type == "function_call") {
        poc::debugger::Debugger::get_breakpoint_function_info(breakpoint);
    } 
    if(breakpoint.m_breakpoint_type == "branch") {
        poc::debugger::Debugger::get_breakpoint_branch_info(breakpoint);  
    }
}

void poc::debugger::Debugger::set_all_function_breakpoints() {
    for (std::vector<compile_unit_t>::size_type cu_idx = 0;
         cu_idx < poc::cu::CompileUnitManager::m_compile_units.size(); 
         cu_idx++) {

        for (std::vector<datatype_t>::size_type fn_idx = 0;
             fn_idx < poc::cu::CompileUnitManager::m_compile_units[cu_idx].functions.size(); 
             fn_idx++) {

            compile_unit_t& cu  = poc::cu::CompileUnitManager::m_compile_units[cu_idx];
            datatype_t& func    = poc::cu::CompileUnitManager::m_compile_units[cu_idx].functions[fn_idx];
            //print_all_possible_breakpoint(cu, func);

            if(func.low_pc == 0)
               continue;

            if(func.name.length() == 0)
                continue;

            if(func.decl_line == 0)
                continue;

            std::intptr_t &bp_address = poc::cu::CompileUnitManager::m_compile_units[cu_idx].functions[fn_idx].low_pc;

            if(!poc::debugger::Debugger::m_symbols_map.count(bp_address)) {
                continue;
            }

            auto &sym = poc::debugger::Debugger::m_symbols_map[bp_address];

            poc::bp::set_breakpoint(
                poc::debugger::Debugger::m_child_pid,
                sym.symbol_name,
                bp_address, 
                "function_call", 
                (int)cu_idx, (int)fn_idx);
        }

    }
}

