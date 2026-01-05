/***********************************************************
 * Edgar A. Leon
 * Lawrence Livermore National Laboratory
 ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>
#include <numaif.h>
#include <sys/mman.h>

#define MAX_LINE_LEN 4096
#define MAX_NODES 1024
#define BITS_PER_UL (8 * sizeof(unsigned long))


/************************
 * Auxiliary functions
 ************************/

static const char *mpol_mode_str(int mode)
{
  switch (mode) {
  case MPOL_DEFAULT:    return "Default";
  case MPOL_PREFERRED:  return "Preferred";
  case MPOL_BIND:       return "Bind";
  case MPOL_INTERLEAVE: return "Interleave";
#ifdef MPOL_LOCAL
  case MPOL_LOCAL:      return "Local";
#endif
  default:              return "Unknown";
  }
}

static int nodemask_str(const unsigned long *mask,
			unsigned long maxnode, char *out)
{
  bool first = true;
  int nc = 0;
  nc += sprintf(out+nc, "{");

  for (unsigned long node = 0; node < maxnode; node++) {
    unsigned long word = node / (8 * sizeof(unsigned long));
    unsigned long bit  = node % (8 * sizeof(unsigned long));

    if (mask[word] & (1UL << bit)) {
      if (!first)
	nc += sprintf(out+nc, ",");
      nc += sprintf(out+nc, "%lu", node);
      first = false;
    }
  }

  nc += sprintf(out+nc, "}");

  return nc;
}

static void get_regroup(const char *line, regmatch_t m, char *out)
{
  // rm_so: start offset of match
  // rm_eo: end offset of match (exclusive)
  int len = (int)(m.rm_eo - m.rm_so);
  if (len <= 0)
    return;

  // . introduces the precision
  // * take the precision from the next argument
  sprintf(out, "%.*s", len, line + m.rm_so);
}


/************************
 * User functions
 ************************/

int get_mem_policy(char *mpol)
{
  int mode;
  unsigned long nodemask[MAX_NODES / BITS_PER_UL];
  memset(nodemask, 0, sizeof(nodemask));

  // Get mode and nodemask
  if (get_mempolicy(&mode, nodemask, MAX_NODES, NULL, 0) == -1) {
    perror("get_mempolicy");
    return 1;
  }

  // Convert nodemask to string
  char mask[MAX_LINE_LEN];
  nodemask_str(nodemask, MAX_NODES, mask);

  sprintf(mpol, "%s%s", mpol_mode_str(mode), mask);

  return 0;
}

const size_t get_page_size()
{
  long ps = sysconf(_SC_PAGESIZE);

  if (ps == -1) {
    perror("sysconf(_SC_PAGESIZE)");
    return 1;
  }

  return (size_t)ps;
}

/*
 * Get NUMA domain used by the given process and VMA
 * using numa_maps
 *
 * numa_maps:
 * 7f8c3a120000 anon=256 dirty=256 active=0 N0=256 kernelpagesize_kB=4
 */
int get_numa(pid_t pid, uintptr_t vma,
	     size_t *npages, size_t *pagesize)
{
  char filename[128];
  snprintf(filename, sizeof(filename), "/proc/%d/numa_maps", pid);
  //printf("Inspecting %s\n", filename);

  FILE *fp = fopen(filename, "r");
  if (!fp) {
    perror("fopen");
    return -1;
  }

  // 15555541e000 bind:1 anon=258 dirty=258 active=0 N1=258 kernelpagesize_kB=4
  char pattern[128];
  snprintf(pattern, sizeof(pattern),
	   "^%" PRIxPTR ".+N([0-9]+)=([0-9]+) +kernelpagesize_kB=([0-9]+)",
	   vma);
  //printf("Regular expression: %s\n", str);

  // POSIX ERE (REG_EXTENDED) so + and () work as expected
  regex_t re;
  int rc = regcomp(&re, pattern, REG_EXTENDED);
  if (rc != 0) {
    char errbuf[256];
    regerror(rc, &re, errbuf, sizeof(errbuf));
    fprintf(stderr, "regcomp failed: %s\n", errbuf);
    fclose(fp);
    return -1;
  }

  // regmatch_t[0] = whole match, [1] and [2] are the capture groups
  int numa = -1;
  regmatch_t m[4];
  char str[128];
  char line[MAX_LINE_LEN];

  while (fgets(line, sizeof(line), fp) != NULL) {
    //printf("numa_maps: %s", line);
    if (regexec(&re, line, 4, m, 0) == 0) {
      //printf("Match: %s", line);
      get_regroup(line, m[1], str);
      numa = atoi(str);
      get_regroup(line, m[2], str);
      if (npages)
	*npages = atoi(str);
      get_regroup(line, m[3], str);
      if (pagesize)
	*pagesize = atoi(str);
      //printf("N%d = %zu PS = %zu\n", numa, *npages, *pagesize);
    }
  }

  regfree(&re);
  fclose(fp);

  return numa;
}

/*
 * Caller is responsible for freeing memory.
 *
 * numa_maps is VMA-centric
 * glibc chooses heap vs mmap dynamically
 * posix_memalign() does not force mmap()
 * Page alignment often keeps allocations on the heap
 *
 * Allocator           -> Backing          -> numa_maps
 * malloc via mmap()   -> separate mapping -> separate line
 * malloc via brk()    -> Heap             -> heap line
 * posix_memalign 4KB  -> Heap             -> heap line
 * posix_memalign 2MB  -> mmap()           -> separate line
 *
 * For predictable behavior: Use mmap() directly
 * This always creates a new VMA.
 *
 */
void* alloc_mem(size_t bytes, size_t *map_size)
{
  const size_t page_size = get_page_size();

  /* Round size up to page size for mmap */
  *map_size = (bytes + page_size - 1) & ~(page_size - 1);

  /*
   * Give me private, zero-filled memory like malloc,
   * but as a separate VMA:
   * MAP_PRIVATE: Changes are not visible to other processes
   * MAP_ANONYMOUS: Memory is not backed by a file
   *                (fd and offset are ignored).
   *                Equivalent to heap memory
   * Can use MAP_HUGETLB to get explicit huge pages (not THP)
   */
  void *ptr = mmap(NULL, *map_size,
		   PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS,
		   -1, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap");
    return NULL;
  }

  // Use THP
  // madvise(p, *map_size, MADV_HUGEPAGE);

#if 0
  void *ptr = NULL;

  int rc = posix_memalign(&ptr, page_size, bytes);
  if (rc != 0) {
    fprintf(stderr, "posix_memalign failed: %s\n", strerror(rc));
    return NULL;
  }
#endif

  unsigned char *buf = ptr;

  // Touch each page to force physical allocation
  // Initialize: byte i contains i % 256
  for (size_t i = 0; i < bytes; i++) {
    buf[i] = (unsigned char)i;
  }

  // Read back data to prevent dead-store elimination
  volatile unsigned long checksum = 0;
  for (size_t i = 0; i < bytes; i++) {
    checksum += buf[i];
  }

  /* Compute block sum dynamically (0 + 1 + ... + 255) */
  unsigned long block_sum = 0;
  for (unsigned int i = 0; i < 256; i++) {
    block_sum += i;
  }

  /*
   * Expected checksum:
   *  - Full 256-byte blocks
   *  - Plus remainder
   */
  size_t full_blocks = bytes / 256;
  size_t remainder   = bytes % 256;
  unsigned long expected =
    full_blocks * block_sum +
    (remainder * (remainder - 1)) / 2;

  /* Verify checksum */
  if (checksum != expected)
    fprintf(stderr, "Checksum verification failed\n");

  /* Verify alignment */
  if ((uintptr_t)ptr % page_size != 0)
    fprintf(stderr, "%p is not properly aligned\n", ptr);

  return ptr;
}
