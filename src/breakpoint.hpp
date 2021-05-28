#ifndef H_BREAKPOINT
#define H_BREAKPOINT

#include <unordered_map> // unordered_map


namespace poc {
namespace bp {

    class Breakpoint {
    public:
        Breakpoint () {}
        Breakpoint (std::string breakpoint_type, 
                    std::string bp_name, 
                    std::intptr_t bp_address, 
                    int cu_index, 
                    int fn_index):

                        m_breakpoint_type{breakpoint_type}, 
                        m_bp_name{bp_name}, 
                        m_bp_address{bp_address}, 
                        m_cu_index{cu_index}, 
                        m_fn_index{fn_index} {}

        /* function_call,   function_ret,   branch(jmp/je), memory */
        std::string m_breakpoint_type;

        std::string m_bp_name;
        std::intptr_t m_bp_address;
        uint8_t m_original_data = 0;
        bool m_enabled = false;

        int m_cu_index;
        int m_fn_index;

    public:
        void enable_breakpoint(pid_t& child_pid);
        void disable_breakpoint(pid_t& child_pid);

    public:
        static std::unordered_map<std::intptr_t, Breakpoint> m_breakpoints;
    };

    void set_breakpoint(
        pid_t child_pid, 
        std::string bp_name, 
        std::intptr_t addr, 
        std::string breakpoint_type);

    void set_breakpoint(
        pid_t child_pid, 
        std::string bp_name, 
        std::intptr_t addr, 
        std::string breakpoint_type, 
        int cu_index, 
        int fn_index);

}
}


#endif