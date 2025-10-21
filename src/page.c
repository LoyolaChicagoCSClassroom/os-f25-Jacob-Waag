#include "page.h"
#include <stddef.h>

#define NUM_PAGES 128
#define PAGE_SIZE (2 * 1024 * 1024) // 2 MB per page

// Static array of pages
static struct ppage physical_page_array[NUM_PAGES];

// Head of the free list
static struct ppage *free_physical_pages = NULL;

void init_pfa_list(void) {
    free_physical_pages = &physical_page_array[0];

    for (int i = 0; i < NUM_PAGES; i++) {
        physical_page_array[i].physical_addr = (void *)(i * PAGE_SIZE);
        physical_page_array[i].next = (i < NUM_PAGES - 1) ? &physical_page_array[i + 1] : NULL;
        physical_page_array[i].prev = (i > 0) ? &physical_page_array[i - 1] : NULL;
    }
}

// Allocate 'npages' pages from the free list
struct ppage *allocate_physical_pages(unsigned int npages) {
    if (npages == 0 || free_physical_pages == NULL)
        return NULL;

    struct ppage *allocd_list = free_physical_pages;
    struct ppage *iter = allocd_list;

    // Move forward npages-1 times
    for (unsigned int i = 1; i < npages && iter->next != NULL; i++) {
        iter = iter->next;
    }

    // Update the free list head
    free_physical_pages = iter->next;
    if (free_physical_pages != NULL)
        free_physical_pages->prev = NULL;

    iter->next = NULL;
    return allocd_list;
}

// Return a list of freed pages to the free list
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
