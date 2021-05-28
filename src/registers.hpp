#ifndef H_REGISTERS
#define H_REGISTERS

#include <sys/user.h> /* user_regs_struct */


namespace poc {
    namespace reg {
        user_regs_struct get_registers(pid_t &child_pid);

        void set_registers(pid_t &child_pid, user_regs_struct &regs);

        void print_registers(user_regs_struct &regs);
    }
}

#endif