#ifndef H_PARAMETERS
#define H_PARAMETERS

#include <capstone/capstone.h>
#include <vector>
#include "common.hpp"

namespace poc {
    namespace disasm {
        typedef struct {
            ElfW(Addr) text_section_entry;
            ElfW(Word) text_section_size;
            uint8_t *program_text_code;
        } text_section_t;

        text_section_t get_text_section(std::string &program_path);

        void get_disassembled_instructions(std::string &program_path);

        void filter_branch_instruction(cs_insn &disassembled_instruction);

        void print_disassembled_instructions();

        void print_branch_instructions();

        static std::vector<branch_type_t> branch_type_table = {
            {"jo",  0x70},
            {"jno", 0x71},  {"jb",  0x72}, {"jnae", 0x72},  {"jc",  0x72},  {"jnb", 0x73},
            {"jae", 0x73},  {"jnc", 0x73}, {"jz",   0x74},  {"je",  0x74},  {"jnz", 0x75},
            {"jne", 0x75},  {"jbe", 0x76}, {"jna",  0x76},  {"jnbe",0x77},  {"ja",  0x77},
            {"js",  0x78},  {"jns", 0x79}, {"jp",   0x7a},  {"jpe", 0x7a},  {"jnp", 0x7b},
            {"jpo", 0x7b},  {"jl",  0x7c}, {"jnge", 0x7c},  {"jnl", 0x7d},  {"jge", 0x7d},
            {"jle", 0x7e},  {"jng", 0x7e}, {"jnle", 0x7f},  {"jg",  0x7f},  {"jmp", 0xeb},
            {"jmp", 0xe9},  {"jmpf",0xea}, {"",     0}
        };

        static std::vector<cs_insn> disassembled_instructions;
        static std::vector<branch_instruction_t> branch_instructions;
    }
}

#endif