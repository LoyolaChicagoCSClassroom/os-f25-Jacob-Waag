#include "page.h"
#include <stddef.h>
#include <stdint.h>

#define NUM_PAGES 128
#define PAGE_SIZE (2 * 1024 * 1024) // 2 MiB pages

// Static array of physical pages for allocator
static struct ppage physical_page_array[NUM_PAGES];
static struct ppage *free_physical_pages = NULL;

// Aligned global page directory/table
struct page_directory_entry pd[1024] __attribute__((aligned(4096)));
struct page pt[1024] __attribute__((aligned(4096)));

// Initialize free list for page frame allocator
void init_pfa_list(void) {
    free_physical_pages = &physical_page_array[0];
    for (int i = 0; i < NUM_PAGES; i++) {
        physical_page_array[i].physical_addr = (void *)(i * PAGE_SIZE);
        physical_page_array[i].next = (i < NUM_PAGES - 1) ? &physical_page_array[i + 1] : NULL;
        physical_page_array[i].prev = (i > 0) ? &physical_page_array[i - 1] : NULL;
    }
}

// Allocate npages from free list as linked list
struct ppage *allocate_physical_pages(unsigned int npages) {
    if (npages == 0 || free_physical_pages == NULL)
        return NULL;
    struct ppage *allocd_list = free_physical_pages;
    struct ppage *iter = allocd_list;
    for (unsigned int i = 1; i < npages && iter->next != NULL; i++) {
        iter = iter->next;
    }
    free_physical_pages = iter->next;
    if (free_physical_pages != NULL)
        free_physical_pages->prev = NULL;
    iter->next = NULL;
    return allocd_list;
}

// Return pages to free list
void free_physical_pages(struct ppage *ppage_list) {
    if (ppage_list == NULL)
        return;
    struct ppage *iter = ppage_list;
    while (iter->next != NULL) {
        iter = iter->next;
    }
    iter->next = free_physical_pages;
    if (free_physical_pages != NULL)
        free_physical_pages->prev = iter;
    free_physical_pages = ppage_list;
    ppage_list->prev = NULL;
}

// Map a linked list of physical pages at virtual addr vaddr into page tables pd
void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd) {
    uint32_t va = (uint32_t)vaddr;
    struct ppage *curr = pglist;
    uint32_t dir_idx = va >> 22;
    uint32_t table_idx = (va >> 12) & 0x3FF;

    // Point page directory entry to page table
    pd[dir_idx].present = 1;
    pd[dir_idx].rw = 1;
    pd[dir_idx].user = 0;
    pd[dir_idx].frame = ((uint32_t)&pt) >> 12;

    while (curr) {
        pt[table_idx].present = 1;
        pt[table_idx].rw = 1;
        pt[table_idx].user = 0;
        pt[table_idx].frame = (uint32_t)curr->physical_addr >> 12;

        curr = curr->next;
        table_idx++;
        va += 4096;
    }
    return vaddr;
}

// Identity map kernel range 0x100000 - &_end_kernel
void identity_map_kernel(struct page_directory_entry *pd, uint32_t end_kernel) {
    uint32_t addr = 0x100000;
    while (addr < end_kernel) {
        struct ppage tmp;
        tmp.next = NULL;
        tmp.physical_addr = (void *)addr;
        map_pages((void *)addr, &tmp, pd);
        addr += 4096;
    }
}

// Identity map current kernel stack page
void identity_map_stack(struct page_directory_entry *pd) {
    uint32_t esp;
    asm("mov %%esp, %0" : "=r"(esp));
    esp &= 0xFFFFF000; // Align to page boundary
    struct ppage tmp;
    tmp.next = NULL;
    tmp.physical_addr = (void *)esp;
    map_pages((void *)esp, &tmp, pd);
}

// Identity map VGA text buffer
void identity_map_video(struct page_directory_entry *pd) {
    struct ppage tmp;
    tmp.next = NULL;
    tmp.physical_addr = (void *)0xB8000;
    map_pages((void *)0xB8000, &tmp, pd);
}

// Load page directory base address into CR3 register
void loadPageDirectory(struct page_directory_entry *pd) {
    asm("mov %0, %%cr3" : : "r"(pd) : );
}
