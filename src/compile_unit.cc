#include <iostream>
#include <sstream>
#include <dlfcn.h> /* dlsym */
#include "compile_unit.hpp"
#include "helpers.hpp"
#include "parameter_extractor.hpp"
#include "memory.hpp"
#include "registers.hpp"


void poc::cu::CompileUnitManager::get_compile_units(elf::elf &elf_object) {
    auto dwarf = dwarf::dwarf{dwarf::elf::create_loader(elf_object)};

    for (auto cu : dwarf.compilation_units()) {
        compile_unit_t compile_unit;
        std::vector<datatype_t> members;
        std::string prev_tag;

        try {
            poc::cu::CompileUnitManager::process_cu_tree(cu.root(), 1, compile_unit, members, prev_tag);
            m_compile_units.push_back(compile_unit);
        } catch(...) {

        }
    }
}

std::intptr_t poc::cu::CompileUnitManager::get_func_addr(const char *func_name) {
    auto handle = dlopen(NULL, RTLD_LOCAL | RTLD_LAZY | RTLD_LOCAL);
    //return (std::intptr_t)dlsym(NULL, func_name);
    return (std::intptr_t)dlsym(handle, func_name);
}

sdb_function_t poc::cu::CompileUnitManager::generate_function_payload(
    pid_t child_pid, 
    compile_unit_t &cu, 
    datatype_t& func, 
    int cu_index, 
    int fn_index) {
    
    sdb_function_t sdb_func;
    sdb_func.name           = func.name;
    sdb_func.filename       = cu.filename;
    sdb_func.line_no        = func.decl_line;
    sdb_func.address        = func.low_pc;
    sdb_func.total_params   = func.parameters.size();

    // get registers data for parameters
    user_regs_struct regs = poc::reg::get_registers(child_pid);

    // get return variable
    datatype_t& func_ret = func;
    // poc::cu::CompileUnitManager::get_datatype_metadata(cu, func_ret, sdb_func.return_var);
    // poc::cu::CompileUnitManager::get_datatype_value(child_pid, sdb_func.return_var, regs, index);

    // get parameters variables
    int complex_idx = 0;
    for(std::vector<datatype_t>::size_type index = 0;
            index < func.parameters.size(); index++) {
        
        datatype_t& func_param = func.parameters[index];
        sdb_parameter_t sdb_param;
        sdb_param.name = func_param.name;
        sdb_param.byte_size = 0;
        sdb_param.cu_index = cu_index;
        sdb_param.fn_index = fn_index;

        poc::cu::CompileUnitManager::get_datatype_metadata(cu, func_param, sdb_param);

        poc::cu::CompileUnitManager::get_datatype_value(
            child_pid, 
            sdb_param, regs, 
            index+1, 
            false, 
            false, 
            complex_idx);

        sdb_func.param_vars.push_back(sdb_param);

        if(sdb_param.is_complex) {
            printf("complex %s %s\n", sdb_param.name.c_str(), sdb_param.value.c_str());
            generate_complex_payload(cu, func_param, sdb_param);            
        }
    }

    // get local variables
    // for(std::vector<datatype_t>::size_type index = 0;
    //         index < func.members.size(); index++) {
        
    //     datatype_t& func_member = func.members[index];
    //     sdb_parameter_t sdb_param;
    //     sdb_param.name = func_member.name;

    //     poc::cu::CompileUnitManager::get_datatype_metadata(cu, func_member, sdb_param);
    //     poc::cu::CompileUnitManager::get_datatype_value(child_pid, sdb_param, regs, index+1, false, true);

    //     sdb_func.local_vars.push_back(sdb_param);
    // }

    return sdb_func;
}

void poc::cu::CompileUnitManager::print_function_payload(sdb_function_t& func) {
    std::string return_type = func.return_var.dt_name;

    // if(func.global_vars.size() > 0) {
    //     printf("[*] globals_variables {\n");
    //     for (std::vector<sdb_parameter_t>::size_type index = 0;
    //         index < func.global_vars.size(); index++) {

    //         printf("\t%s%s;\n", func.global_vars[index].dt_name.c_str(), 
    //                             func.global_vars[index].name.c_str());
    //     }
    //     printf("[*] }\n");
    // }

    if(func.total_params < 1) {
        printf("[*] %s%s()\n", return_type.c_str(), func.name.c_str());
    }
    
    if(func.total_params == 1) {
        std::string value_alias;
        if(func.param_vars[0].value_alias.size() > 0)
            value_alias = " = '" + func.param_vars[0].value_alias + "'";

        printf("[*] %s%s(%s%s<%s%s>)\n",  return_type.c_str(), 
                                    func.name.c_str(), 
                                    func.param_vars[0].dt_name.c_str(), 
                                    func.param_vars[0].name.c_str(),
                                    func.param_vars[0].value.c_str(),
                                    value_alias.c_str());
    }

    if(func.total_params > 1) {
        printf("[*] %s%s(\n", return_type.c_str(), func.name.c_str());

        for (std::vector<sdb_parameter_t>::size_type index = 0;
            index < func.param_vars.size(); index++) {

            std::string value_alias;
            if(func.param_vars[index].value_alias.length() > 0) 
                value_alias = " = '" + func.param_vars[index].value_alias + "'";
                
            if (index == func.param_vars.size()-1)
                printf("\t%s%s<%s%s>)\n", func.param_vars[index].dt_name.c_str(), 
                                    func.param_vars[index].name.c_str(),
                                    func.param_vars[index].value.c_str(),
                                    value_alias.c_str());
            else 
                printf("\t%s%s<%s%s>,\n", func.param_vars[index].dt_name.c_str(), 
                                    func.param_vars[index].name.c_str(),
                                    func.param_vars[index].value.c_str(),
                                    value_alias.c_str());
        }
    }

    // if(func.local_vars.size() > 0) {
    //     printf("[*] {\n");
    //     for (std::vector<sdb_parameter_t>::size_type index = 0;
    //         index < func.local_vars.size(); index++) {

    //         printf("\t%s%s;\n", func.local_vars[index].dt_name.c_str(), 
    //                             func.local_vars[index].name.c_str());
    //     }
    //     printf("[*] }\n");
    // }

    //filename and line
    printf("[*] %s:%d\n", func.filename.c_str(), func.line_no);
}

void poc::cu::CompileUnitManager::generate_complex_payload(
    compile_unit_t &cu, 
    datatype_t& data_param, 
    sdb_parameter_t& sdb_param) {

    if((sdb_param.value == "0xfffffffffffffff") || (sdb_param.value == "0x0"))
        return;

    //get complex param datatype_t
    datatype_t complex_param;
    datatype_t temp_param = data_param;

    bool found = false;
    for(std::vector<datatype_t>::size_type index = 0; index < cu.datatypes.size(); index++) {
        if(temp_param.type == cu.datatypes[index].section_offset) {
            if(cu.datatypes[index].type_name == "DW_TAG_structure_type") {
                complex_param = cu.datatypes[index];
                found = true;
                break;
            } else {
                temp_param = cu.datatypes[index];
                index = 0;
            }
        }
    }

    if(!found)
        return;

    for(std::vector<datatype_t>::size_type index = 0;
            index < complex_param.members.size(); index++) {

        datatype_t& member_param = complex_param.members[index];
        sdb_parameter_t sdb_param_temp;
        sdb_param_temp.name = member_param.name;
        poc::cu::CompileUnitManager::get_datatype_metadata(cu, member_param, sdb_param_temp);

        long long sdb_param_value = std::stol(sdb_param.value, nullptr, 0);
        long long offset_;
        offset_ = sdb_param_value + (long long)member_param.data_member_location;
        
        //datatype_value_to_string(sdb_param_temp, offset_, true);
        sdb_param.members.push_back(sdb_param_temp);

        printf("0x%llx = 0x%llx + 0x%llx = (val)%s (%s%s(%s))\n", 
            offset_, sdb_param_value, (long long)member_param.data_member_location, 
            sdb_param_temp.value.c_str(), sdb_param_temp.dt_name.c_str(), 
            sdb_param_temp.name.c_str(),  sdb_param_temp.value_alias.c_str());

        // if(sdb_param_temp.is_complex) {
        //     printf("inner-complex %s\n", sdb_param_temp.name.c_str());
        //     generate_complex_payload(cu, member_param, sdb_param_temp);   
        // }
    }
}

void poc::cu::CompileUnitManager::process_cu_tree(
    const dwarf::die &node, 
    int depth, 
    compile_unit_t& compile_unit, 
    std::vector<datatype_t>& members, 
    std::string& prev_tag) {
    
    // start a new cu 
    if((depth == 1) && (dwarf::to_string(node.tag) == "DW_TAG_compile_unit")) {
        members.clear();
        for (auto &attr : node.attributes())
            poc::cu::CompileUnitManager::set_new_cu(
                dwarf::to_string(attr.first), dwarf::to_string(attr.second), compile_unit);
    }

    // add new datatype
    if(depth == 2) {
        if(members.size() > 0) {
            // save members(to subprogram/structure/array_type)
            if(prev_tag == "functions") {
                datatype_t& dt = \
                    compile_unit.functions[compile_unit.functions.size()-1];

                for(auto& i: members) {
                    //printf("%s: %s\n", dt.name.c_str(), i.type_name.c_str());
                    if(i.type_name == "DW_TAG_formal_parameter")
                        dt.parameters.push_back(i);

                    if(i.type_name == "DW_TAG_unspecified_parameters")
                        dt.parameters.push_back(i);

                    if(i.type_name == "DW_TAG_variable")
                        dt.members.push_back(i);  
                }
            
            } else if (prev_tag == "datatype") {
                datatype_t& dt = \
                    compile_unit.datatypes[compile_unit.datatypes.size()-1];
                dt.members = members;

            } else {}
            
            // clear members after saving
            members.clear();
            prev_tag = "";
        }

        if(dwarf::to_string(node.tag) == "DW_TAG_subprogram") {
            datatype_t func;
            func.type = 0;
            func.section_offset = node.get_section_offset();
            func.type_name = dwarf::to_string(node.tag);
            func.low_pc = 0;
            func.high_pc = 0;
            func.dyn_sym = false;
            func.inline_ = 0;

            for (auto &attr : node.attributes()) {
                poc::cu::CompileUnitManager::add_function_field(node.tag, 
                    dwarf::to_string(attr.first), dwarf::to_string(attr.second), func);
            }
            if(func.type > 0)
                poc::cu::CompileUnitManager::set_datatype(compile_unit, func);
            
            // since value of high_addr is offset (eg.<offset-from-lowpc>2261)
            func.high_pc = func.low_pc + func.high_pc;

            if((func.low_pc == 0) && (func.inline_ < 1)) {
                func.low_pc = poc::cu::CompileUnitManager::get_func_addr(func.name.c_str());
                func.dyn_sym = true;
            }

            compile_unit.functions.push_back(func);
            prev_tag = "functions";

        } else if (dwarf::to_string(node.tag) == "DW_TAG_variable") {
            datatype_t variable;
            variable.type = 0;
            variable.section_offset = node.get_section_offset();
            variable.type_name = dwarf::to_string(node.tag);

            for (auto &attr : node.attributes()) {
                poc::cu::CompileUnitManager::add_global_variable_field(node.tag, 
                    dwarf::to_string(attr.first), dwarf::to_string(attr.second), variable);
            }     

            if(variable.type > 0)
                poc::cu::CompileUnitManager::set_datatype(compile_unit, variable);
            compile_unit.global_variables.push_back(variable);
            prev_tag = "";

        } else {
            datatype_t datatype;
            datatype.type = 0;
            datatype.section_offset = node.get_section_offset();
            datatype.type_name = dwarf::to_string(node.tag);

            for (auto &attr : node.attributes()) {
                poc::cu::CompileUnitManager::add_datatype_field(node.tag, 
                    dwarf::to_string(attr.first), dwarf::to_string(attr.second), datatype);
            }     
            
            if(datatype.type > 0)
                poc::cu::CompileUnitManager::set_datatype(compile_unit, datatype);
            compile_unit.datatypes.push_back(datatype);
            prev_tag = "datatype";
        }
    }

    if(depth > 2) {
        if(dwarf::to_string(node.tag) == "DW_TAG_lexical_block") {
            // do nothing 

        // if DW_TAG_inlined_subroutine close function params     
        } else if(dwarf::to_string(node.tag) == "DW_TAG_inlined_subroutine") {
            if((prev_tag == "functions") && (members.size() > 0)) {
                // save members(to subprogram/structure/array_type)
                if(prev_tag == "functions") {
                    datatype_t& dt = \
                        compile_unit.functions[compile_unit.functions.size()-1];

                    for(auto& i: members) {
                        //printf("%s: %s\n", dt.name.c_str(), i.type_name.c_str());
                        if(i.type_name == "DW_TAG_formal_parameter")
                            dt.parameters.push_back(i);

                        if(i.type_name == "DW_TAG_unspecified_parameters")
                            dt.parameters.push_back(i);

                        if(i.type_name == "DW_TAG_variable")
                            dt.members.push_back(i);   
                    }
                
                } else if (prev_tag == "datatype") {
                    datatype_t& dt = \
                        compile_unit.datatypes[compile_unit.datatypes.size()-1];
                    dt.members = members;

                } else {}
                
                // clear members after saving
                members.clear();
                prev_tag = "";
            }

        } else if(  (dwarf::to_string(node.tag) == "DW_TAG_formal_parameter") || \
                    (dwarf::to_string(node.tag) == "DW_TAG_unspecified_parameters") || \
                    (dwarf::to_string(node.tag) == "DW_TAG_variable")) {

            datatype_t datatype;
            datatype.type = 0;
            datatype.section_offset = node.get_section_offset();
            datatype.type_name = dwarf::to_string(node.tag);

            for (auto &attr : node.attributes()) {
                poc::cu::CompileUnitManager::add_datatype_field(node.tag, 
                    dwarf::to_string(attr.first), dwarf::to_string(attr.second), datatype);
            }  
            if(datatype.type > 0)
                poc::cu::CompileUnitManager::set_datatype(compile_unit, datatype);
            members.push_back(datatype);

        } else if (dwarf::to_string(node.tag) == "DW_TAG_member") {
            datatype_t datatype;
            datatype.type = 0;
            datatype.section_offset = node.get_section_offset();
            datatype.type_name = dwarf::to_string(node.tag);

            for (auto &attr : node.attributes()) {
                poc::cu::CompileUnitManager::add_datatype_field(node.tag, 
                    dwarf::to_string(attr.first), dwarf::to_string(attr.second), datatype);
            }  
            if(datatype.type > 0)
                poc::cu::CompileUnitManager::set_datatype(compile_unit, datatype);
            members.push_back(datatype);
        } else {}
    }
    
    for (auto &child : node)
        poc::cu::CompileUnitManager::process_cu_tree(child, depth + 1, compile_unit, members, prev_tag);
}

void poc::cu::CompileUnitManager::set_new_cu(
    std::string first, 
    std::string second, 
    compile_unit_t &compile_unit) {

    if(first == "DW_AT_name")
        compile_unit.filename = second;

    if(first == "DW_AT_low_pc") 
        compile_unit.low_addr = std::stol(second, nullptr, 0);
    
    if(first == "DW_AT_high_pc")
        compile_unit.high_addr = std::stol(second, nullptr, 0);
    
    if(first == "DW_AT_producer")
        compile_unit.producer = second;

    if(first == "DW_AT_comp_dir")
        compile_unit.directory = second;
}

void poc::cu::CompileUnitManager::add_datatype_field(
    enum dwarf::DW_TAG node_tag, 
    std::string first, 
    std::string second, 
    datatype_t &datatype) {

    if(first == "DW_AT_name") {
        switch(node_tag) {
            case dwarf::DW_TAG::base_type:
            case dwarf::DW_TAG::typedef_:
            case dwarf::DW_TAG::union_type:
            case dwarf::DW_TAG::member:
            case dwarf::DW_TAG::formal_parameter:
            case dwarf::DW_TAG::variable:
            case dwarf::DW_TAG::structure_type:
                datatype.name = second;
                break;
        }
    } else if(first == "DW_AT_byte_size") {
        switch(node_tag) {
            case dwarf::DW_TAG::base_type:
            case dwarf::DW_TAG::union_type:
            case dwarf::DW_TAG::pointer_type:
            case dwarf::DW_TAG::structure_type:
            case dwarf::DW_TAG::enumeration_type:
                datatype.byte_size = std::stol(second, nullptr, 0);
                break;
        }
    } else if(first == "DW_AT_encoding") {
        switch(node_tag) {
            case dwarf::DW_TAG::base_type:
                datatype.encoding = std::stol(second, nullptr, 0);;
                break;
        }
    } else if(first == "DW_AT_decl_file") {
        switch(node_tag) {
            case dwarf::DW_TAG::typedef_:
            case dwarf::DW_TAG::union_type:
            case dwarf::DW_TAG::structure_type:
            case dwarf::DW_TAG::member:
            case dwarf::DW_TAG::enumeration_type:
            case dwarf::DW_TAG::formal_parameter:
            case dwarf::DW_TAG::variable:
                datatype.decl_file = std::stol(second, nullptr, 0);;
                break;
        }
    } else if(first == "DW_AT_decl_line") {
        switch(node_tag) {
            case dwarf::DW_TAG::typedef_:
            case dwarf::DW_TAG::union_type:
            case dwarf::DW_TAG::structure_type:
            case dwarf::DW_TAG::member:
            case dwarf::DW_TAG::enumeration_type:
            case dwarf::DW_TAG::formal_parameter:
            case dwarf::DW_TAG::variable:
                datatype.decl_line = std::stol(second, nullptr, 0);
                break;
        }
    } else if(first == "DW_AT_sibling") {
        switch(node_tag) {
            case dwarf::DW_TAG::array_type:
            case dwarf::DW_TAG::union_type:
            case dwarf::DW_TAG::subroutine_type:
            case dwarf::DW_TAG::structure_type:
            case dwarf::DW_TAG::enumeration_type:
                // naively assume that the user has written <0x77>
                // convert to 77

                std::string addr (std::string(second), 3, std::string(second).length()-1); 
                datatype.sibling = std::stol(addr, 0, 16);
                break;
        }
    } else if(first == "DW_AT_type") {
        switch(node_tag) {
            case dwarf::DW_TAG::typedef_:
            case dwarf::DW_TAG::array_type:
            case dwarf::DW_TAG::const_type:
            case dwarf::DW_TAG::subroutine_type:
            case dwarf::DW_TAG::member:
            case dwarf::DW_TAG::subrange_type:
            case dwarf::DW_TAG::formal_parameter:
            case dwarf::DW_TAG::variable:
            case dwarf::DW_TAG::pointer_type:
                // naively assume that the user has written <0x77>
                // convert to 77
                std::string addr (std::string(second), 3, std::string(second).length()-1); 
                datatype.type = std::stol(addr, 0, 16);
                break;
        }
    } else if(first == "DW_AT_prototyped") {
        switch(node_tag) {
            case dwarf::DW_TAG::subroutine_type:
                std::istringstream(second) >> std::boolalpha >> datatype.prototyped;
                break;
        }
    } else if(first == "DW_AT_external") {
        switch(node_tag) {
            case dwarf::DW_TAG::variable:
                std::istringstream(second) >> std::boolalpha >> datatype.external;
                break;
        }
    } else if(first == "DW_AT_declaration") {
        switch(node_tag) {
            case dwarf::DW_TAG::variable:
                std::istringstream(second) >> std::boolalpha >> datatype.declaration;
                break;
        }
    } else if(first == "DW_AT_upper_bound") {
        switch(node_tag) {
            case dwarf::DW_TAG::subrange_type:
                datatype.upper_bound = std::stol(second, nullptr, 0);;
                break;
        }
    } else if(first == "DW_AT_data_member_location") {
        switch(node_tag) {
            case dwarf::DW_TAG::member:
                if (std::string(second).find("<loclist") == 0) {
                    // naively assume that the user has written <0x77> <loclist 0x10080>
                    // convert to 77
                    std::string addr (std::string(second), 11, std::string(second).length()-1); 
                    datatype.data_member_location = std::stol(addr, 0, 16);
                } else {
                    datatype.data_member_location = std::stol(second, nullptr, 0);
                }   
                break;
        }
    } else if(first == "DW_AT_location") {
        switch(node_tag) {
            case dwarf::DW_TAG::member:
            case dwarf::DW_TAG::formal_parameter:
                datatype.location = second;
                break;
        }
    }
}

void poc::cu::CompileUnitManager::add_function_field(
    enum dwarf::DW_TAG node_tag, 
    std::string first, 
    std::string second, 
    datatype_t &func) {

    if(first == "DW_AT_external")
        std::istringstream(second) >> std::boolalpha >> func.external;

    if(first == "DW_AT_name")
        func.name = second;

    if(first == "DW_AT_decl_file") 
        func.decl_file = std::stol(second, nullptr, 0);

    if(first == "DW_AT_decl_line") 
        func.decl_line = std::stol(second, nullptr, 0);

    if(first == "DW_AT_prototyped")
        std::istringstream(second) >> std::boolalpha >> func.prototyped;

    if(first == "DW_AT_low_pc") 
        func.low_pc = std::stol(second, nullptr, 0);

    if(first == "DW_AT_high_pc") 
        func.high_pc = std::stol(second, nullptr, 0);

    if(first == "DW_AT_frame_base") 
        func.frame_base = second;

    if(first == "DW_AT_inline") 
        func.inline_ = std::stol(second, nullptr, 0);

    if(first == "DW_AT_type") {
        std::string addr (std::string(second), 3, std::string(second).length()-1); 
        func.type = std::stol(addr, 0, 16);
    }

    if(first == "DW_AT_sibling") {
        std::string addr (std::string(second), 3, std::string(second).length()-1); 
        func.sibling = std::stol(addr, 0, 16);
    }
}

void poc::cu::CompileUnitManager::add_global_variable_field(
    enum dwarf::DW_TAG node_tag, 
    std::string first, 
    std::string second, 
    datatype_t &global_variable) {

    if(first == "DW_AT_name")
        global_variable.name = second;

    if(first == "DW_AT_decl_file") 
        global_variable.decl_file = std::stol(second, nullptr, 0);

    if(first == "DW_AT_decl_line") 
        global_variable.decl_line = std::stol(second, nullptr, 0);

    if(first == "DW_AT_type") {
        std::string addr (std::string(second), 3, std::string(second).length()-1); 
        global_variable.type = std::stol(addr, 0, 16);
    }

    if(first == "DW_AT_external")
        std::istringstream(second) >> std::boolalpha >> global_variable.external;

    if(first == "DW_AT_declaration")
        std::istringstream(second) >> std::boolalpha >> global_variable.declaration;
}

void poc::cu::CompileUnitManager::set_datatype(
    compile_unit_t &compile_unit, 
    datatype_t &datatype) {

    datatype.datatype_index = 0;
    for (std::vector<datatype_t>::size_type idx = 0;
         idx < compile_unit.datatypes.size(); 
         idx++) {
        
        if(compile_unit.datatypes[idx].section_offset ==datatype.type) {
            datatype.datatype_index = (int)idx;
            break;
        }
    }
}

void poc::cu::CompileUnitManager::get_datatype_metadata(
    compile_unit_t &cu, 
    datatype_t& datatype, 
    sdb_parameter_t& sdb_param) {

    std::string dt;
    bool stop_search = true;

    if(datatype.type_name == "DW_TAG_unspecified_parameters") {
        sdb_param.datatypes.push_back("DW_TAG_unspecified_parameters");
        dt = "...";
    }

    if(datatype.type == 0)
        stop_search = false;

    datatype_t current_dt = datatype;
    while(stop_search) {
        bool found = false;

        for (auto& i: cu.datatypes) {
            if(current_dt.type == i.section_offset) {
                sdb_param.byte_size = i.byte_size;

                if(i.type_name == "DW_TAG_pointer_type") {
                    dt =  "*" + dt;
                    if(i.type == 0)
                        dt =  "void " + dt;

                } else if (i.type_name == "DW_TAG_typedef") {
                    // if current dt is typedef and previous dt is
                    // typedef, put in bracket
                    if( (sdb_param.datatypes.size() > 0) &&
                        (sdb_param.datatypes.at(
                            sdb_param.datatypes.size()-1) == "DW_TAG_typedef"))
                        dt = "(" + i.name + ") " + dt;
                    else
                        dt = i.name + " " + dt;
                    
                } else if(i.type_name == "DW_TAG_const_type") {
                    dt = "const " + dt;

                } else {
                    // if current dt is base_type and previous 2 dt(s) is
                    // typedef, put in bracket
                    if( (sdb_param.datatypes.size() > 0) &&
                        (i.type_name == "DW_TAG_base_type") && 
                        (sdb_param.datatypes.at(
                            sdb_param.datatypes.size()-1) == "DW_TAG_typedef") &&
                        (i.name.length() > 0))
                        dt = "(" + i.name + ") " + dt;
                    else
                        if(i.name.length() > 0)
                            dt = i.name + " " + dt; 
                }

                sdb_param.datatypes.push_back(i.type_name);
                current_dt = i;
                found = true;

                if(current_dt.type == 0)
                    stop_search = false;
                break;
            }
        }
        if(!found)
            stop_search = false;
    }
    sdb_param.dt_name = dt;
}

void poc::cu::CompileUnitManager::get_datatype_value(
    pid_t child_pid, 
    sdb_parameter_t& sdb_param, 
    user_regs_struct& regs, 
    int index, 
    bool ret, 
    bool local, 
    int& complex_idx) {

    int pindex = index + complex_idx;
    int datatype_idx = index;

    /* x86_64 supported only at this point--
    * We are essentially parsing this
    * calling convention here:

        mov    %rsp,%rbp
        mov    $0x6,%r9d
        mov    $0x5,%r8d
        mov    $0x4,%ecx
        mov    $0x3,%edx
        mov    $0x2,%esi
        mov    $0x1,%edi
        callq  400144 <func>
    */

    // return value located at %rbp+8
    if(ret) {
        return;
    }

    // if its  local variable %rbp-${index*8}
    if(local) {
        return;
    }

    switch(pindex) {
        // get value from %rdi
        case 1:
            poc::cu::CompileUnitManager::datatype_value_to_string(
                child_pid, sdb_param, regs.rdi, false, datatype_idx, complex_idx, regs);
            break;

        // get value from %rsi
        case 2:
            poc::cu::CompileUnitManager::datatype_value_to_string(
                child_pid, sdb_param, regs.rsi, false, datatype_idx, complex_idx, regs);
            break;

        // get value from %rdx
        case 3:
            poc::cu::CompileUnitManager::datatype_value_to_string(
                child_pid, sdb_param, regs.rdx, false, datatype_idx, complex_idx, regs);
            break;

        // get value from %rcx
        case 4:
            poc::cu::CompileUnitManager::datatype_value_to_string(
                child_pid, sdb_param, regs.rcx, false, datatype_idx, complex_idx, regs);
            break;

        // get value from %r8
        case 5:
            poc::cu::CompileUnitManager::datatype_value_to_string(
                child_pid, sdb_param, regs.r8, false, datatype_idx, complex_idx, regs);
            break;

        // get value from %r9
        case 6:
            poc::cu::CompileUnitManager::datatype_value_to_string(
                child_pid, sdb_param, regs.r9, false, datatype_idx, complex_idx, regs);
            break;

        // from 7 going get from the stack
        default:
            break;
    }
}

void poc::cu::CompileUnitManager::datatype_value_to_string(
    pid_t child_pid, 
    sdb_parameter_t& sdb_param, 
    long long value, 
    bool force_ptr_on_basictypes, 
    int& datatype_idx, 
    int& complex_idx, 
    user_regs_struct& regs) {
    
    int  pointer_count  = 0;
    bool array_type     = false;
    long byte_size      = 0;
    std::string datatype_name;

    sdb_param.is_complex = false;
    sdb_param.value = std::string("unknown");

    for (auto&i : sdb_param.datatypes) {
        if(i == "DW_TAG_unspecified_parameters") {
            sdb_param.value = std::string("...");
            return;
        }
        if(i == "DW_TAG_pointer_type") 
            pointer_count = pointer_count + 1;
        
        if(i == "DW_TAG_base_type") {
            if(poc::parameter_extractor::is_number_value(sdb_param.dt_name)) 
                datatype_name = "number";

            if(poc::parameter_extractor::is_string_value(sdb_param.dt_name)) 
                datatype_name = "string";
        }

        // if(i == "DW_AT_byte_size") 
        //     byte_size = pointer_count + 1;

        if(i == "DW_TAG_array_type") 
            pointer_count = pointer_count + 1;

        if(i == "DW_TAG_structure_type") {
            datatype_name = "structure";
            sdb_param.is_complex = true;
        }

        if(i == "DW_TAG_union_type") {
        }
        if(i == "DW_TAG_enumeration_type") {
        }
    }

    if(pointer_count == 0) {
        if(datatype_name == "number")
            poc::parameter_extractor::get_number_value(child_pid, sdb_param, value, array_type); 

        // get pointer to array and values(alais) where to points 
        // 0x412165 (["a", "b", "c", "d"])
        if(datatype_name == "string")
            poc::parameter_extractor::get_string_value(child_pid, sdb_param, value, array_type);

        // get pointer to structure. eg.0x412165
        if(datatype_name == "structure") {
            sdb_param.value = "...";
            poc::cu::CompileUnitManager::complex_value_to_string(
                sdb_param, 
                value, 
                datatype_idx, 
                complex_idx, 
                regs);
        }
    }

    if(pointer_count > 0) {
        sdb_param.value = poc::helpers::long_to_hex_string(value);

        if(sdb_param.value == "0xffffffffffffffff") {
            sdb_param.value_alias = "-1";
            return;
        }

        if(sdb_param.value == "0x0") {
            sdb_param.value_alias = "0";
            return;
        }

        // check if value is null 
        auto data_at_pointer_addr = poc::mem::read_memory(child_pid, (uint64_t)value);
        if(data_at_pointer_addr == 0) {
            sdb_param.value_alias = "NULL";
            return;
        }
    }

    if(pointer_count == 1) {
        if(datatype_name == "number") 
            poc::parameter_extractor::get_number_value(child_pid, sdb_param, value, true); 

        if(datatype_name == "string")
            poc::parameter_extractor::get_string_value(child_pid, sdb_param, value, true); 

        // if(datatype_name == "structure") {
        //     sdb_param.value = poc::helpers::long_to_hex_string(value);
        //     _get_structure_value(child_pid, sdb_param, value, true);
        // }
    }

    if(pointer_count == 2) {
        if(datatype_name == "number")
            poc::parameter_extractor::get_multi_number_value(child_pid, sdb_param, value, true); 

        if(datatype_name == "string")
            poc::parameter_extractor::get_multi_string_value(child_pid, sdb_param, value, true); 

        // if(datatype_name == "structure") {
        //     sdb_param.value = poc::helpers::long_to_hex_string(value);
        //     _get_multi_structure_value(child_pid, sdb_param, value, true);
        // }
    }
}

void poc::cu::CompileUnitManager::complex_value_to_string(
    sdb_parameter_t& sdb_param, 
    long long value, 
    int& datatype_idx, 
    int& complex_idx, 
    user_regs_struct& regs) {

    if((sdb_param.value == "0xfffffffffffffff") || (sdb_param.value == "0x0"))
        return;

    printf("datatype_idx  = %d\n", datatype_idx);
    printf("complex_idx   = %d\n", complex_idx);
    printf("fn_index      = %d\n", sdb_param.fn_index);
    printf("cu_index      = %d\n", sdb_param.cu_index);

    compile_unit_t& cu = m_compile_units[sdb_param.cu_index];
    datatype_t& fn     = cu.functions[sdb_param.fn_index];
    datatype_t& dt     = fn.parameters[datatype_idx-1];

    //get complex param datatype_t
    datatype_t complex_param;
    datatype_t temp_param = dt;

    bool found = false;
    for(std::vector<datatype_t>::size_type index = 0; index < cu.datatypes.size(); index++) {
        if(temp_param.type == cu.datatypes[index].section_offset) {
            if(cu.datatypes[index].type_name == "DW_TAG_structure_type") {
                complex_param = cu.datatypes[index];
                found = true;
                break;
            } else {
                temp_param = cu.datatypes[index];
                index = 0;
            }
        }
    }

    if(!found)
        return;

    for(std::vector<datatype_t>::size_type index = 0;
            index < complex_param.members.size(); index++) {

        datatype_t& member_param = complex_param.members[index];

        sdb_parameter_t sdb_param_temp;
        sdb_param_temp.name = member_param.name;
        poc::cu::CompileUnitManager::get_datatype_metadata(cu, member_param, sdb_param_temp);

        printf("member= %s %llx\n", member_param.name.c_str(), sdb_param_temp.byte_size);

        // if()

        // long long sdb_param_value = std::stol(sdb_param.value, nullptr, 0);
        // long long offset_;
        // offset_ = sdb_param_value + (long long)member_param.data_member_location;
        
        // //datatype_value_to_string(sdb_param_temp, offset_, true);
        // sdb_param.members.push_back(sdb_param_temp);

        // printf("0x%llx = 0x%llx + 0x%llx = (val)%s (%s%s(%s))\n", 
        //     offset_, sdb_param_value, (long long)member_param.data_member_location, 
        //     sdb_param_temp.value.c_str(), sdb_param_temp.dt_name.c_str(), 
        //     sdb_param_temp.name.c_str(),  sdb_param_temp.value_alias.c_str());

        // if(sdb_param_temp.is_complex) {
        //     printf("inner-complex %s\n", sdb_param_temp.name.c_str());
        //     generate_complex_payload(cu, member_param, sdb_param_temp);   
        // }
    }
}
