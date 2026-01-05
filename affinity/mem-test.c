/***********************************************************
 * Edgar A. Leon
 * Lawrence Livermore National Laboratory
 ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include "affinity.h"

#define BYTES 4*2*MB


int main(void)
{
  const size_t size = BYTES;
  const size_t page_size = get_page_size();
  pid_t pid = getpid();

  // Should probably allocate memory using
  // posix_memalign((void **)&buf, page_size, size);
  unsigned char *buf = malloc(size);
  if (!buf) {
    perror("malloc failed");
    return 1;
  }

  // Touch each page to force physical allocation
  for (size_t i = 0; i < size; i++) {
    buf[i] = (unsigned char)(i & 0xFF);
  }

  // Read back data to prevent dead-store elimination
  volatile unsigned long checksum = 0;
  for (size_t i = 0; i < size; i++) {
    checksum += buf[i];
  }

  // Compute number of (4KB) pages (rounded up)
  size_t pages = (size + page_size - 1) / page_size;

  // Compute page-aligned address inside the buffer
  uintptr_t aligned_addr = (uintptr_t)buf & ~(uintptr_t)(page_size - 1);


  /*****************************************
   * Source NUMA domain
   *****************************************/
  size_t npages, psize;
  int numa = get_numa(pid, aligned_addr, &npages, &psize);


  /*****************************************
   * Memory policy
   *****************************************/

  char mpol[LONG_STR_SIZE];
  get_mem_policy(mpol);

  /*****************************************
   * Report
   *****************************************/

  printf("Process ID            : %d\n", pid);
  printf("Page size             : %zu bytes\n", page_size);
  printf("Memory policy         : %s\n", mpol);
  printf("User buffer\n");
  printf(" Allocated memory     : %zu bytes\n", size);
  printf(" Pages used           : %zu\n", pages);
  printf(" Address              : %p\n", (void *)buf);
  printf(" Page-aligned address : %" PRIxPTR "\n", (uintptr_t)aligned_addr);
  printf(" Buffer checksum      : %lu\n", checksum);
  printf(" NUMA ID (maps)       : %d\n", numa);
  printf(" Pages used (maps)    : %zu\n", npages);
  printf(" Page size (maps)     : %zu KB\n", psize);

  printf("Sleeping...\n");
  sleep(2);

  free(buf);
  printf("Memory freed.\n");

  return 0;
}
