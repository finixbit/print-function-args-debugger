#ifndef H_PARAMETER_EXTRACTOR
#define H_PARAMETER_EXTRACTOR

#include "common.hpp"


namespace poc {
    namespace parameter_extractor {

        // static std::vector<std::string> string_types;

        // static std::vector<std::string> number_types;

        static std::vector<std::string> string_types = {
            "signed char",  "char",   "unsigned char",
        };

        static std::vector<std::string> number_types = {
            "short",        "bool",         "int",    
            "long",         "float",        "double", 
            "long long",    "long int",     "unsigned short",
            "signed int",   "signed long",  "signed long long",
            "signed short", "long double",  "long unsigned int",
            "unsigned int", "signed",       "long double",
            "unsigned",            
        };

        uint64_t read_memory(
            pid_t child_pid, 
            uint64_t address
        );

        std::string get_char_data(
            pid_t child, 
            uint64_t addr, 
            long byte_size
        );

        std::string get_number_data(
            pid_t child, 
            uint64_t addr, 
            long byte_size
        );

        std::vector<uint64_t> get_address_data(
            pid_t child, 
            uint64_t addr
        );

        bool is_number_value(std::string type_name);

        void get_number_value(
            pid_t pid, 
            sdb_parameter_t& sdb_param, 
            long long value, 
            bool is_array_type);

        void get_multi_number_value(
            pid_t pid, 
            sdb_parameter_t& sdb_param, 
            long long value, 
            bool is_array_type);

        bool is_string_value(std::string type_name);

        void get_string_value(
            pid_t pid, 
            sdb_parameter_t& sdb_param, 
            long long value, 
            bool is_array_type);

        void get_multi_string_value(
            pid_t pid, 
            sdb_parameter_t& sdb_param, 
            long long value, 
            bool is_array_type);

        void get_structure_value(
            pid_t pid,
            sdb_parameter_t& sdb_param, 
            long long value, 
            bool is_array_type);

    }
}

#endif