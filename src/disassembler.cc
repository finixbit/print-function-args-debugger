#include <iostream>
#include <sys/mman.h> /* mmap, MAP_PRIVATE */
#include <sys/stat.h> /* For the size of the file. , fstat */
#include <inttypes.h> /* PRIx64 */
#include <fcntl.h>       // O_RDONLY 
#include "disassembler.hpp"


poc::disasm::text_section_t poc::disasm::get_text_section(std::string &program_path) {
    poc::disasm::text_section_t handle;
    int fd, i;
    struct stat st;
    uint8_t *mem;

    /* Open the binary file for reading. */
    if ((fd = open(program_path.c_str(), O_RDONLY)) < 0) {
        std::cout << "Err: open" << std::endl;
        exit(-1);
    }
  
    /* Get the size of the binary file. */
    if (fstat(fd, &st) < 0) {
        std::cout << "Err: fstat" << std::endl;
        exit(-1);
    }

    /* Memory-map the binary file. */
    mem = static_cast<uint8_t*>(mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (mem == MAP_FAILED) {
        std::cout << "Err: mmap" << std::endl;
        exit(-1);
    }

    ElfW(Ehdr) *ehdr = (ElfW(Ehdr) *)mem;
    ElfW(Phdr) *phdr = (ElfW(Phdr) *)&mem[ehdr->e_phoff];
    ElfW(Addr) entry = ehdr->e_entry;
    ElfW(Addr) textVaddr;
    ElfW(Word) textSize;

    for (i = 0; i < ehdr->e_phnum; i++) {
        switch(phdr[i].p_type) {
            case PT_LOAD:
                switch(!(!phdr[i].p_offset)) { 
                    case 0:
                        textVaddr = phdr[i].p_vaddr;
                        textSize = phdr[i].p_memsz;
                        break;
                }
                break;
        }
    }

    ElfW(Addr) textSectionVAddress = textVaddr;
    handle.text_section_entry = entry;
    handle.text_section_size = textSize;

    ElfW(Off) offset = handle.text_section_entry - textSectionVAddress; 
    handle.program_text_code = &mem[offset];
    return handle;
}

void poc::disasm::get_disassembled_instructions(std::string &program_path) {
    csh capstone_handle;
    cs_insn *dis_insns;
    size_t count;
    poc::disasm::text_section_t text_section = poc::disasm::get_text_section(program_path);

    if (cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle) != CS_ERR_OK) {
        std::cout << "Initializing Capstone failed ..." << std::endl;
        return;
    }

    count = cs_disasm(capstone_handle, text_section.program_text_code, 
                      text_section.text_section_size, 
                      text_section.text_section_entry, 0, 
                      &dis_insns);

    if (count > 0) {
        size_t j;
        for (j = 0; j < count; j++) {
            poc::disasm::filter_branch_instruction(dis_insns[j]);
            poc::disasm::disassembled_instructions.push_back(dis_insns[j]);
        }
        cs_free(dis_insns, count);
    } else {
        std::cout << "ERROR: Failed to disassemble program ..." << std::endl;
    }
    cs_close(&capstone_handle);
}

void poc::disasm::filter_branch_instruction(cs_insn &disassembled_instruction) {
    for(auto &branch: poc::disasm::branch_type_table) {
        if(branch.mnemonic == disassembled_instruction.mnemonic) {
            branch_instruction_t branch_instruction;
            branch_instruction.mnemonic = disassembled_instruction.mnemonic;
            branch_instruction.location_addr = disassembled_instruction.address;
            poc::disasm::branch_instructions.push_back(branch_instruction);
            break; 
        }
    }
}

void poc::disasm::print_disassembled_instructions() {
    for (auto &disassembled_instruction : poc::disasm::disassembled_instructions) {
        printf("0x%" PRIx64 ":\t%s\t\t%s\n", disassembled_instruction.address, 
                disassembled_instruction.mnemonic, disassembled_instruction.op_str);
    }
}

void poc::disasm::print_branch_instructions() {
    for (auto &branch_instruction : poc::disasm::branch_instructions) {
        printf("0x%" PRIx64 ": %s\n", branch_instruction.location_addr, branch_instruction.mnemonic.c_str());
    }
}