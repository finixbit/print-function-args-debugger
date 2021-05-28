#ifndef H_CU
#define H_CU

#include <vector>
#include <string> 
#include <libelfin/elf/elf++.hh>
#include <libelfin/dwarf/dwarf++.hh>
#include <sys/user.h> /* user_regs_struct */
#include "common.hpp"


namespace poc {
namespace cu {
    class CompileUnitManager {
    public:
        // {filename: {compile_units}}
        // std::unordered_map<std::string, compile_unit_t> m_compile_units;
        std::vector<compile_unit_t> m_compile_units;

    private:
        std::intptr_t get_func_addr(const char *func_name);

        void process_cu_tree(
            const dwarf::die &node, 
            int depth, 
            compile_unit_t& compile_unit, 
            std::vector<datatype_t>& members, 
            std::string& prev_tag);

        void set_new_cu(
            std::string first, 
            std::string second, 
            compile_unit_t &compile_unit);

        void add_datatype_field(
            enum dwarf::DW_TAG node_tag, 
            std::string first, 
            std::string second, 
            datatype_t &datatype);

        void add_function_field(
            enum dwarf::DW_TAG node_tag, 
            std::string first, 
            std::string second, 
            datatype_t &func);

        void add_global_variable_field(
            enum dwarf::DW_TAG node_tag, 
            std::string first, 
            std::string second, 
            datatype_t &global_variable);

        void set_datatype(
            compile_unit_t &compile_unit, 
            datatype_t &global_variable);

        void get_datatype_metadata(
            compile_unit_t &cu, 
            datatype_t& datatype, 
            sdb_parameter_t& sdb_param);

        void get_datatype_value(
            pid_t child_pid, 
            sdb_parameter_t& sdb_param, 
            user_regs_struct& regs, 
            int index, 
            bool ret, 
            bool local, 
            int& complex_idx);

        void datatype_value_to_string(
            pid_t child_pid, 
            sdb_parameter_t& sdb_param, 
            long long value, 
            bool force_ptr_on_basictypes, 
            int& datatype_idx, 
            int& complex_idx, 
            user_regs_struct& reg);

        void complex_value_to_string(
            sdb_parameter_t& sdb_param, 
            long long value, 
            int& datatype_idx, 
            int& complex_idx, 
            user_regs_struct& regs);

    public:
        void get_compile_units(elf::elf &elf_object);

        sdb_function_t generate_function_payload(
            pid_t child_pid, 
            compile_unit_t &cu, 
            datatype_t& func, 
            int cu_index, 
            int fn_index);

        void print_function_payload(sdb_function_t& func);
      
        void generate_complex_payload(
            compile_unit_t &cu, 
            datatype_t& data_param, 
            sdb_parameter_t& sdb_param);
    };
}
}

#endif