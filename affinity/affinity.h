/***********************************************************
 * Edgar A. Leon
 * Lawrence Livermore National Laboratory
 ***********************************************************/

#ifndef AFFINITY_H_INCLUDED
#define AFFINITY_H_INCLUDED

#include <unistd.h>
#include <inttypes.h>

#define MB 1024*1024
#define SHORT_STR_SIZE 32
#define LONG_STR_SIZE 4096

#ifdef __cplusplus
extern "C" {
#endif

  int get_gpu_count();

  int get_gpu_pci_id(int dev);

  int get_gpu_affinity(char *buf);

  int get_gpu_info(int dev, char *buf);

  int get_gpu_info_all(char *buf);

  int get_num_cpus();

  int get_cpu_affinity(char *buf);

  int get_numa(pid_t pid, uintptr_t aligned_addr,
	       size_t *npages, size_t *pagesize);

  int get_mem_policy(char *mpol);

  size_t get_page_size();

  void* alloc_mem(size_t bytes, size_t *map_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
