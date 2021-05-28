#include <sys/ptrace.h> /* ptrace */
#include <cstring>
#include <sstream>
#include "breakpoint.hpp"
#include "helpers.hpp"


std::unordered_map<std::intptr_t, poc::bp::Breakpoint> poc::bp::Breakpoint::m_breakpoints;

void poc::bp::set_breakpoint(
    pid_t child_pid, 
    std::string bp_name, 
    std::intptr_t addr, 
    std::string breakpoint_type) {

    poc::helpers::print_address(addr, "Set breakpoint for " + bp_name);
    poc::bp::Breakpoint breakpoint{breakpoint_type, bp_name, addr, 0, 0};

    breakpoint.enable_breakpoint(child_pid);
    poc::bp::Breakpoint::m_breakpoints[addr] = breakpoint;
}

void poc::bp::set_breakpoint(
    pid_t child_pid, 
    std::string bp_name, 
    std::intptr_t addr, 
    std::string breakpoint_type, 
    int cu_index, 
    int fn_index) {

    poc::helpers::print_address(addr, "Set breakpoint for " + bp_name);
    poc::bp::Breakpoint breakpoint{breakpoint_type, bp_name, addr, cu_index, fn_index};

    if (poc::bp::Breakpoint::m_breakpoints.count(addr)) {
        return;
    }

    breakpoint.enable_breakpoint(child_pid);
    poc::bp::Breakpoint::m_breakpoints[addr] = breakpoint;
}

void poc::bp::Breakpoint::enable_breakpoint(pid_t& child_pid) {
    uint64_t data = ptrace(PTRACE_PEEKDATA, child_pid, poc::bp::Breakpoint::m_bp_address, nullptr);
    poc::bp::Breakpoint::m_original_data = static_cast<uint8_t>(data & 0xff); //save bottom byte

    if(data == -1){
        std::stringstream ss;
        ss  << " [PTRACE FAILURE] " << " ; errno = " << errno << " ; msg = '" << strerror(errno) << "' \n";        
        throw std::runtime_error(ss.str());
    }

    //set bottom byte to 0xcc
    uint64_t int3 = 0xcc;
    uint64_t data_with_int3 = ((data & 0xFFFFFFFFFFFFFF00) | int3);
    ptrace(PTRACE_POKEDATA, child_pid, poc::bp::Breakpoint::m_bp_address, data_with_int3);

    poc::bp::Breakpoint::m_enabled = true;
}

void poc::bp::Breakpoint::disable_breakpoint(pid_t& child_pid) {
    uint64_t data = ptrace(PTRACE_PEEKDATA, child_pid, poc::bp::Breakpoint::m_bp_address, nullptr);
    if(data == -1){
        std::stringstream ss;
        ss  << " [PTRACE FAILURE] " << " ; errno = " << errno << " ; msg = '" << strerror(errno) << "' \n";        
        throw std::runtime_error(ss.str());
    }

    //overwrite the low byte with the original data and write it back to memory.
    auto restored_data = ((data & 0xFFFFFFFFFFFFFF00) | poc::bp::Breakpoint::m_original_data);
    ptrace(PTRACE_POKEDATA, child_pid, poc::bp::Breakpoint::m_bp_address, restored_data);

    poc::bp::Breakpoint::m_enabled = false;
}