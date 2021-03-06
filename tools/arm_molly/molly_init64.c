// Simple boot-loader.
//
// This code is only intended for use on M5 where it is started via
// molly_boot.S which runs on Core 0.

/*
 * Copyright (c) 2007-2010,2015, ETH Zurich.
 * Copyright (c) 2015, Hewlett Packard Enterprise Development LP.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <barrelfish_kpi/types.h>
#include <errors/errno.h>
#include <elf/elf.h>
#include <multiboot.h>

#define BASE_PAGE_SIZE                  0x1000
#define ALIGNMENT                       0x10000

/// Round up n to the next multiple of size
#define ROUND_UP(n, size)           ((((n) + (size) - 1)) & (~((size) - 1)))


static lpaddr_t phys_alloc_start;

static errval_t linear_alloc(void *s, genvaddr_t base, size_t size, uint32_t flags,
                             void **ret)
{
    // round to base page size
    uint32_t npages = (size + BASE_PAGE_SIZE - 1) / BASE_PAGE_SIZE;

    /* *ret = (void *)(uintptr_t)base; */
    *ret = (void *)phys_alloc_start;

    phys_alloc_start += npages * BASE_PAGE_SIZE;
    return SYS_ERR_OK;
}

genvaddr_t kernel_entry;

// Prototypes for functions from molly_boot.S:
void aarch64_to_kernel_transition(genvaddr_t entry_addr, void* mbi_ptr);


// Prototypes for functions from arm_gem5_kernel.c:
extern struct multiboot_info *molly_get_mbi(void);

// Prototypes for symbols declared via linker script:
extern void *_start_img;
extern void *_end_img;

void aarch64_init(void);

void aarch64_init(void)
{
    errval_t err;

    //uint32_t kernel_blob_size = (uint32_t)(&kernel_blob_end - &kernel_blob_start);


    struct multiboot_info *mbi = molly_get_mbi();

    // align kernel start to 16KB
    phys_alloc_start = ROUND_UP((uint64_t) &_end_img, ALIGNMENT); //+
                // BASE_PAGE_SIZE);
    lpaddr_t kernel_start = phys_alloc_start;

    // Load the kernel out from the boot image:
    struct multiboot_modinfo *mbi_mods;
    mbi_mods = (struct multiboot_modinfo*)(uint64_t)(mbi->mods_addr);
    void *kernel = (void*)(uint64_t)(mbi_mods[0].mod_start);
    uint32_t kernel_bytes = mbi_mods[0].mod_end - mbi_mods[0].mod_start;

    err = elf64_load(EM_AARCH64, linear_alloc, NULL, (uint64_t)kernel,
                    kernel_bytes, &kernel_entry, NULL, NULL, NULL);
    if (err_is_fail(err)) {
        return;
    }

    // Relocate kernel image
    struct Elf64_Ehdr *cpu_head = (struct Elf64_Ehdr *)kernel;
    struct Elf64_Shdr *rela, *symtab, *symhead =
        (struct Elf64_Shdr *)(kernel + (uintptr_t)cpu_head->e_shoff);
    genvaddr_t elfbase = elf_virtual_base64(cpu_head);
    rela = elf64_find_section_header_type(symhead, cpu_head->e_shnum, SHT_RELA);
    symtab = elf64_find_section_header_type(symhead, cpu_head->e_shnum, SHT_DYNSYM);
    elf64_relocate(kernel_start, elfbase,
                   (struct Elf64_Rela *)(uintptr_t)(kernel + rela->sh_offset),
                   rela->sh_size,
                   (struct Elf64_Sym *)(uintptr_t)(kernel + symtab->sh_offset),
                   symtab->sh_size,
                   elfbase, (void *)kernel_start);
    kernel_entry = kernel_entry - elfbase + kernel_start;

    //initialize arm_core_data elf info for relocated header
    mbi->syms.elf.num = cpu_head->e_shnum;
    mbi->syms.elf.size = cpu_head->e_shentsize;
    mbi->syms.elf.addr = (uint64_t)kernel+ cpu_head->e_shoff;
    mbi->syms.elf.shndx = cpu_head->e_shstrndx;

    aarch64_to_kernel_transition((uintptr_t)kernel_entry, mbi );
    

}
