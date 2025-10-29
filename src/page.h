#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

// Physical page node
struct ppage {
    struct ppage *next;
    uint32_t physical_addr;
};

// Page Directory and Table Entries (i386)
struct page_directory_entry {
    uint32_t present       : 1;
    uint32_t rw            : 1;
    uint32_t user          : 1;
    uint32_t writethru     : 1;
    uint32_t cachedisabled : 1;
    uint32_t accessed      : 1;
    uint32_t pagesize      : 1;
    uint32_t ignored       : 2;
    uint32_t os_specific   : 3;
    uint32_t frame         : 20;
};

struct page {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;
};

// Paging functions
void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd);
void identity_map_kernel(struct page_directory_entry *pd, uint32_t end_kernel);
void identity_map_stack(struct page_directory_entry *pd);
void identity_map_video(struct page_directory_entry *pd);
void loadPageDirectory(struct page_directory_entry *pd);

#endif // PAGE_H
