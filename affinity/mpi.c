/***********************************************************
 * Edgar A. Leon
 * Lawrence Livermore National Laboratory
 ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <sys/mman.h>
#include "affinity.h"


int main(int argc, char *argv[])
{
  char buf[LONG_STR_SIZE];
  char hostname[MPI_MAX_PROCESSOR_NAME];
  int rank, np, size, i;
  int verbose = 0;
  int ncpus = get_num_cpus();
  int nc = 0;

  /* Get rid of compiler warning. Ay. */
  (void) verbose;

  /* Command-line options */
  if (argc > 1)
    for (i=1; i<argc; i++) {
      if ( strcmp(argv[i], "-v") == 0 )
	verbose = 1;
    }

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);
  MPI_Get_processor_name(hostname, &size);

  nc += sprintf(buf+nc, "%3d %s %3d CPUs: ",
		rank, hostname, ncpus);
  nc += get_cpu_affinity(buf+nc);

#ifdef HAVE_GPUS
  int ndevs = get_gpu_count();
  nc += sprintf(buf+nc, "%3d %s %3d GPUs: ",
		rank, hostname, ndevs);
  nc += get_gpu_affinity(buf+nc);
  if (verbose)
    nc += get_gpu_info_all(buf+nc);
#endif

#ifdef WITH_NUMA
  size_t map_size;
  void *ptr = alloc_mem(4*2*MB, &map_size);

  if (ptr) {
    char mpol[LONG_STR_SIZE];
    get_mem_policy(mpol);

    size_t npages, psize;
    int numa = get_numa(getpid(), (uintptr_t)ptr, &npages, &psize);

    nc += sprintf(buf+nc, "%3d %s %3s NUMA: %d %s %zuKB\n",
		  rank, hostname, "", numa, mpol, psize);

    munmap(ptr, map_size);
  };
#endif

  /* Print per-task information */
  printf("%s", buf);

  MPI_Finalize();

  return 0;
}
