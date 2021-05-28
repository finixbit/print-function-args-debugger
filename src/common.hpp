#ifndef H_COMMON
#define H_COMMON

#include <vector> // vector
#include <unordered_map> // unordered_map
#include <link.h> /* ElfW */

#define MAX_PROCMAP_FILE_SIZE 600
#define MAX_PROCMAP_LINE_SIZE 600
#define NODE_TAG_DEBUG false

typedef struct {
    int symbol_num;
    std::intptr_t symbol_value;
    std::string symbol_name;
    int symbol_size;
    std::string symbol_index;
    std::string symbol_binding;
    std::string symbol_type;  //[section, notype, file, object, func]
    std::string symbol_table; //[.dynsym, .symtab]
} symbol_t;

typedef struct {
    std::string mnemonic;
    uint8_t opcode;
} branch_type_t;

typedef struct {
    std::string mnemonic; //[jmp, jz, etc]
    std::intptr_t location_addr;
} branch_instruction_t;

typedef struct datatype_t {
    long section_offset;
    int datatype_index;
    int depth;

    /* supported types
       depth(2)
       DW_TAG_base_type,    DW_TAG_structure_type,  DW_TAG_subroutine_type
       DW_TAG_pointer_type, DW_TAG_const_type,      DW_TAG_typedef,
       DW_TAG_array_type,   DW_TAG_union_type,      DW_TAG_enumeration_type,
       DW_TAG_subprogram,   DW_TAG_variable,
       
       depth(gt 2)
       DW_TAG_subrange_type after DW_TAG_array_type,
       DW_TAG_member after DW_TAG_structure_type
       DW_TAG_formal_parameter after DW_TAG_subprogram */
    std::string type_name;

    /* used by
       DW_TAG_base_type,    DW_TAG_typedef,     DW_TAG_union_type,
       DW_TAG_member,       DW_TAG_subprogram,  DW_TAG_variable, 
       DW_TAG_formal_parameter, */
    std::string name;

    /* used by
       DW_TAG_base_type,    DW_TAG_union_type,  DW_TAG_pointer_type, 
       DW_TAG_structure_type, DW_TAG_enumeration_type, */
    long byte_size;

    /* used by
       DW_TAG_base_type,  */
    long encoding;

    /* used by
       DW_TAG_typedef,      DW_TAG_union_type,  DW_TAG_structure_type,
       DW_TAG_member,       DW_TAG_subprogram,  DW_TAG_enumeration_type, 
       DW_TAG_variable,     DW_TAG_formal_parameter, */
    long decl_file;

    /* used by
       DW_TAG_typedef,      DW_TAG_union_type,  DW_TAG_structure_type,
       DW_TAG_member,       DW_TAG_subprogram,  DW_TAG_enumeration_type, 
       DW_TAG_variable,     DW_TAG_formal_parameter, */
    long decl_line;

    /* used by
       DW_TAG_array_type,   DW_TAG_union_type,  DW_TAG_subroutine_type, 
       DW_TAG_structure_type,   DW_TAG_enumeration_type,
       DW_TAG_subprogram, */
    long sibling;

    /* used by
       DW_TAG_array_type,   DW_TAG_const_type,  DW_TAG_subroutine_type,
       DW_TAG_typedef,      DW_TAG_member,      DW_TAG_subrange_type,
       DW_TAG_variable,     DW_TAG_pointer_type(empty if void*)   
       DW_TAG_formal_parameter, */
    long type;

    /* used by
       DW_TAG_subprogram,   DW_TAG_subroutine_type,  */
    bool prototyped;

    /* used by
       DW_TAG_subrange_type,  */
    long upper_bound;

    /* used by
       DW_TAG_member,  */
    long data_member_location;

    /* used by (visible outside the current compilation.)
       DW_TAG_subprogram,   DW_TAG_variable, */
    bool external;

    /* used by
       DW_TAG_subprogram,  */
    std::intptr_t low_pc;

     /* used by
       DW_TAG_subprogram,  */
    long inline_;

    /* used by
       DW_TAG_subprogram,  */
    std::intptr_t high_pc;

    /* used by
       DW_TAG_subprogram,  */
    std::string frame_base;

    /* used by
       DW_TAG_variable, */
    bool declaration;

    /* used by
       DW_TAG_formal_parameter, */
    std::string location;

    /* used by
       DW_TAG_subprogram, */
    bool dyn_sym;

    /* used by
       DW_TAG_structure_type (DW_TAG_member),  
       DW_TAG_subroutine_type/DW_TAG_subprogram(for DW_TAG_variable) */
    std::vector<datatype_t> members;

    /* used by
       DW_TAG_structure_type,  
       DW_TAG_subroutine_type/DW_TAG_subprogram(for DW_TAG_formal_parameter) */
    std::vector<datatype_t> parameters;    

    /* used by 
       DW_TAG_subprogram, */
    std::vector<datatype_t> embedded_functions;    
} datatype_t;

typedef struct {
    /* function_call,   function_ret,   branch(jmp/je), 
       memory */
    std::string breakpoint_type; 
    std::intptr_t location_addr;
    uint8_t original_data;
    bool enabled;

    int cu_index;
    int fn_index;
} breakpoint_t;

typedef struct {
    std::string             filename;
    std::string             directory;
    std::intptr_t           low_addr;
    std::intptr_t           high_addr;
    std::string             producer;
    std::vector<datatype_t> datatypes;
    std::vector<datatype_t> global_variables;
    std::vector<datatype_t> functions;
} compile_unit_t;

typedef struct sdb_parameter_t {
    // debug purpose
    int cu_index;
    int fn_index;

    std::string name;
    std::vector<std::string> datatypes;
    std::string dt_name;
    std::string value;
    std::string value_alias;
    long byte_size;
    bool is_complex;

    std::vector<sdb_parameter_t> members;
} sdb_parameter_t;

typedef struct sdb_function_t {
    std::string name;
    std::string filename;
    int line_no;
    std::intptr_t address;
    int total_params;

    sdb_parameter_t              return_var;
    std::vector<sdb_parameter_t> param_vars;
    std::vector<sdb_parameter_t> local_vars;
    std::vector<sdb_parameter_t> global_vars;
} sdb_function_t;

#endif
