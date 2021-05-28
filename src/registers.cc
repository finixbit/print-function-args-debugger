#include <iostream>
#include <sys/ptrace.h> /* ptrace */
#include "registers.hpp"


user_regs_struct poc::reg::get_registers(pid_t &child_pid) {
    struct user_regs_struct regs;
    long esp, eax, ebx, edx, ecx, esi, edi, eip;
    #ifdef __x86_64__
      esp = regs.rsp;
      eip = regs.rip;
      eax = regs.rax;
      ebx = regs.rbx;
      ecx = regs.rcx;
      edx = regs.rdx;
      esi = regs.rsi;
      edi = regs.rdi;
    #else
      esp = regs.esp;
      eip = regs.eip;
      eax = regs.eax;
      ebx = regs.ebx;
      ecx = regs.ecx;
      edx = regs.edx;
      esi = regs.esi;
      edi = regs.edi;
    #endif
    if(ptrace(PTRACE_GETREGS, child_pid, nullptr, &regs) == -1) {
        std::cout << "Error: PTRACE_GETREGS" << std::endl;
        exit(1);
    };
    return regs;
}

void poc::reg::set_registers(pid_t &child_pid, user_regs_struct &regs) {
    user_regs_struct current_regs = poc::reg::get_registers(child_pid);
    ptrace(PTRACE_SETREGS, child_pid, nullptr, &regs);
}

void poc::reg::print_registers(user_regs_struct &regs) {
    printf("[+] rax 0x%lx\n", regs.rax);
    printf("[+] rbx 0x%lx\n", regs.rbx);
    printf("[+] rcx 0x%lx\n", regs.rcx);
    printf("[+] rdx 0x%lx\n", regs.rdx);
    printf("[+] rsi 0x%lx\n", regs.rsi);
    printf("[+] rdi 0x%lx\n", regs.rdi);
    printf("[+] rbp 0x%lx\n", regs.rbp);
    printf("[+] rsp 0x%lx\n", regs.rsp);
    printf("[+] rip 0x%lx\n", regs.rip);
}