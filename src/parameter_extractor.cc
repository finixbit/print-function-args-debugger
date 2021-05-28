#include <string> 
#include <cctype>
#include <sys/ptrace.h> /* ptrace */
#include "parameter_extractor.hpp"
#include "helpers.hpp"
#include "common.hpp"

#define SIZEOF_LONG sizeof(long)


uint64_t poc::parameter_extractor::read_memory(pid_t child_pid, uint64_t address) {
    // uint64_t mem_val = 0;
    // if((mem_val = ptrace(PTRACE_PEEKDATA, m_child_pid, address, nullptr)) == -1) {
    //   printf("ptrace error @ read_memory\n");
    //   mem_val = 0;
    // }
    // return mem_val;
    return ptrace(PTRACE_PEEKDATA, child_pid, address, nullptr);
}

std::string poc::parameter_extractor::get_char_data(
    pid_t child, 
    uint64_t addr, 
    long byte_size) {

    std::string result;
    int index;
    char *str;

    typedef union _data {
        long val;
        char chars[SIZEOF_LONG];
    } Data;

    Data data;
    while(true) {
        long data_address = addr + (index * SIZEOF_LONG);
        if ((data.val = ptrace(PTRACE_PEEKDATA, child, data_address, NULL)) == -1) {
            printf("ptrace error @ get_char_data\n");
            break;
        }

        int j = 0;
        bool quit = false;
        for (; j < SIZEOF_LONG && data.chars[j] != '\n'; ++j, ++str) {
            if(isprint(data.chars[j]) || data.chars[j] == ' ') {
                result = result + std::to_string(data.chars[j]);
            } else {
                quit = true;
                break;
            }
        }

        if((quit) || (data.chars[j] == '\n'))
            break;
          
        ++index;
    }
    return result;
}

std::string poc::parameter_extractor::get_number_data(
    pid_t child, 
    uint64_t addr, 
    long byte_size) {

    std::string result;
    int index = 0;
    long data_val = 0;
    std::vector<int64_t> values;

    while(true) {
        long data_address = addr + (index * SIZEOF_LONG)/2;
        if(byte_size != 0)
            data_address = addr + (index * byte_size);

        if ((data_val = ptrace(PTRACE_PEEKTEXT, child, data_address, NULL)) == -1) {
            printf("ptrace error @ get_number_data\n");
            break;
        }

        //printf("bsize %llx, data_val=%d \n", byte_size, (int32_t)data_val);
        if((int32_t)data_val == 0)
            break;

        if((poc::helpers::long_to_hex_string(data_val).size() == 18)  || 
           (poc::helpers::long_to_hex_string(data_val).size() == 10))
            break;

        values.push_back((int32_t)data_val);
        index++;
    }

    if(values.size() == 0)
        return result;

    result = "[";
    for (std::vector<int64_t>::size_type idx = 0; idx < values.size(); idx++) {
        if(idx == values.size()-1)
            result = result + std::to_string(values[idx]);
        else
            result = result + std::to_string(values[idx]) + ",";
    }
    result = result + "]";
    return result;
}

std::vector<uint64_t> poc::parameter_extractor::get_address_data(pid_t child, uint64_t addr) {
    std::vector<uint64_t> result;
    int index = 0;
    long data_val = 0;

    while(true) {
        uint64_t data_address = addr + (index * SIZEOF_LONG);
        if ((data_val = ptrace(PTRACE_PEEKDATA, child, data_address, nullptr)) == -1) {
            printf("error: ptrace\n");
            break;
        }

        if((long long)data_val == 0)
            break;

        //printf("found: idx(%d) addr(%llx) val(%llx)\n", index, data_address, data_val);
        result.push_back((uint64_t)data_address);
        index++;
    }

    return result;
}

bool poc::parameter_extractor::is_number_value(std::string type_name) {
    for(auto tp: poc::parameter_extractor::number_types) {
        if(poc::helpers::contains_string(type_name, tp))   {
            return true;
        }
    }
    return false;
}

void poc::parameter_extractor::get_number_value(
    pid_t pid, 
    sdb_parameter_t& sdb_param, 
    long long value, 
    bool is_array_type) {

    if(!is_array_type) {
        if(value == 0) {
            sdb_param.value = std::string("0");
        } else {
            sdb_param.value = std::to_string(value);
        }
        return;
    }

    if(value == 0) {
        sdb_param.value = std::string("0x0");
        return;
    }

    sdb_param.value = poc::helpers::long_to_hex_string(value);
    sdb_param.value_alias = poc::parameter_extractor::get_number_data(pid, value, sdb_param.byte_size);
}

void poc::parameter_extractor::get_multi_number_value(
    pid_t pid,
    sdb_parameter_t& sdb_param, 
    long long value,
    bool is_array_type) {

    std::vector<uint64_t> addrs = poc::parameter_extractor::get_address_data(pid, value);

    sdb_param.value_alias = "[ ";
    for (std::vector<int64_t>::size_type idx = 0; idx < addrs.size(); idx++) {
        auto addr = poc::parameter_extractor::read_memory(pid, addrs[idx]);
        auto data = poc::parameter_extractor::get_number_data(pid, addr, sdb_param.byte_size);

        if(idx == addrs.size()-1)
            sdb_param.value_alias = sdb_param.value_alias + poc::helpers::long_to_hex_string(addr) + "=" + data + " ";

        else
            sdb_param.value_alias = sdb_param.value_alias + poc::helpers::long_to_hex_string(addr) + "=" + data + ", ";
    }
    sdb_param.value_alias = sdb_param.value_alias + "]";
}


bool poc::parameter_extractor::is_string_value(std::string type_name) {
    for(auto tp: poc::parameter_extractor::string_types) {
        if(poc::helpers::contains_string(type_name, tp))   {
            return true;
        }
    }
    return false;
}

void poc::parameter_extractor::get_string_value(
    pid_t pid,
    sdb_parameter_t& sdb_param, 
    long long value, 
    bool is_array_type) {

    if(!is_array_type) {
        sdb_param.value = std::to_string(value);
        return;
    }
    sdb_param.value = poc::helpers::long_to_hex_string(value);
    sdb_param.value_alias = get_char_data(pid, value, sdb_param.byte_size);
}

void poc::parameter_extractor::get_multi_string_value(
    pid_t pid,
    sdb_parameter_t& sdb_param, 
    long long value, 
    bool is_array_type) {

    std::vector<uint64_t> addrs = poc::parameter_extractor::get_address_data(pid, value);
    
    sdb_param.value_alias = "[ ";
    for (std::vector<int64_t>::size_type idx = 0; idx < addrs.size(); idx++) {
        auto addr = poc::parameter_extractor::read_memory(pid, addrs[idx]);
        auto data = poc::parameter_extractor::get_char_data(pid, addr, sdb_param.byte_size);

        if(idx == addrs.size()-1)
            sdb_param.value_alias = sdb_param.value_alias + poc::helpers::long_to_hex_string(addr) + "=" + data + " ";

        else
            sdb_param.value_alias = sdb_param.value_alias + poc::helpers::long_to_hex_string(addr) + "=" + data + ", ";
    }
    sdb_param.value_alias = sdb_param.value_alias + "]";
}

void poc::parameter_extractor::get_structure_value(
    pid_t pid,
    sdb_parameter_t& sdb_param, 
    long long value, 
    bool is_array_type) {

    // if(!is_array_type) {
    //   sdb_param.value = poc::helpers::to_string<long long>(value);
    //   return;
    // }
    // sdb_param.value = poc::helpers::long_to_hex_string(value);
    // sdb_param.value_alias = get_char_data(pid, value);
}


