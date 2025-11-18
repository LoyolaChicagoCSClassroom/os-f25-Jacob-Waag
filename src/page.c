#include "page.h"
#include <stdint.h>

struct ppage physical_page_array[128];
struct ppage *free_page_list = 0;

extern int _end_kernel;


void init_pfa_list(void) {
  uintptr_t addr = (uintptr_t)&_end_kernel;

  for (int i = 0; i < 128; i++) {
      physical_page_array[i].physical_addr = (void *)addr;
      physical_page_array[i].next = (i < 128 - 1) ? &physical_page_array[i + 1]: 0;
      physical_page_array[i].prev = (i > 0) ? &physical_page_array[i - 1] : 0;

      addr += 0x200000;
  }

  free_page_list = &physical_page_array[0];

}

struct ppage *allocate_physical_pages(unsigned int npages) {
  if (free_page_list == 0 || npages == 0)
      return 0;

  struct ppage *allocd_list= free_page_list;
  struct ppage *tail = allocd_list;

  for (unsigned int i = 1; i < npages && tail->next; i++) 
      tail = tail->next;

  free_page_list = tail->next;
  if (free_page_list)
      free_page_list->prev = 0;

  tail->next = 0;
  allocd_list->prev= 0;

  return allocd_list;
}

void free_physical_pages(struct ppage *ppage_list) {
  if (ppage_list == 0)
      return;

  struct ppage *tail = ppage_list;
  while (tail->next)
      tail = tail->next;

  tail->next = free_page_list;
  if (free_page_list)
      free_page_list->prev = tail;

  free_page_list = ppage_list;
  ppage_list->prev = 0;
}
void loadPageDirectory(struct page_directory_entry *pd) {
    asm("mov %0, %%cr3" : : "r"(pd) : );
}
