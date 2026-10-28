# MPI Load Imbalance

*Edgar A. León*<br>
Lawrence Livermore National Laboratory

## Table of contents

1. [Profiling your code with mpiP](#profiling-your-code-with-mpip)
1. [Linking against the mpiP library](#linking-against-the-mpip-library)
1. [Loading mpiP at runtime](#loading-mpip-at-runtime)
1. [Example output](#example-output)
1. [Hands-on exercises](#hands-on-exercises)


## Profiling your code with mpiP

To determine if there is imbalance among the processes in an MPI job,
we will use the **mpiP** library, which characterizes the time spent
in communication operations and the time spent in the application
(non-communication time).

mpiP is documented here:<br>
[https://software.llnl.gov/mpiP/](https://software.llnl.gov/mpiP/)

In this page, we will show an example of how to use it on the Leonardo
cluster.

mpiP can be used by linking a program against the mpiP library or
loading the library directly at runtime.

## Linking against the mpiP library

mpiP is a link-time library. We need to do two things:
1. Add `-g` when building the application to add debugging symbols.
1. Link the application to the mpiP library

An example is shown here:
```
$MAP_DIR/mpi-hello/Makefile
```

Once built, run the program:

``` bash
[eleonbor@login01 mpi-hello]$ srun_dcgp -N1 -n2 -t1 -qdcgp_qos_dbg hello-p
mpiP:
mpiP: mpiP V3.5.0 (Build Oct 26 2026/18:34:40)
mpiP:
Hello from task 0 of 2.
mpiP:
mpiP: Storing mpiP output in [./hello-p.2.2653326.1.mpiP].
mpiP:
Hello from task 1 of 2.
```

## Loading mpiP at runtime

``` bash
# Find the library here: MPIP_LIB
module load mpip
```

``` bash
# Load mpiP at runtime
# We can use the MPIP environment variable to
# pass parameters to the library
MPIP="-c" srun_dcgp --export=LD_PRELOAD=$MPIP_LIB/libmpiP.so,ALL -N1 -n4 -t1 -qdcgp_qos_dbg hello
mpiP: Found MPIP environment variable [-c]
mpiP:
mpiP: mpiP V3.5.0 (Build Oct 26 2026/18:34:40)
mpiP:
Hello from task 0 of 4.
mpiP:
mpiP: Storing mpiP output in [./hello.4.18891.1.mpiP].
mpiP:
Hello from task 3 of 4.
Hello from task 1 of 4.
Hello from task 2 of 4.
```

Currently, there are issues getting the symbol information (call sites)
from mpiP. To see those symbols, link your program against the mpiP
static library as shown in the Makefile above.

## Example output

``` bash
$ cat hello-p.8.2695210.1.mpiP
@ mpiP
@ Command : /leonardo_scratch/fast/ICT26_MHPC_0/leon/install/bin/hello-p
@ Version                  : 3.5.0
@ MPIP Build date          : Oct 26 2026, 20:41:29
@ Start time               : 2026 10 26 22:32:46
@ Stop time                : 2026 10 26 22:32:46
@ Timer Used               : PMPI_Wtime
@ MPIP env var             : -c
@ Collector Rank           : 0
@ Collector PID            : 2695210
@ Final Output Dir         : .
@ Report generation        : Single collector task

---------------------------------------------------------------------------
@--- Task Time Statistics (seconds) ---------------------------------------
---------------------------------------------------------------------------
                     AppTime           MPITime   MPI%   App Task   MPI Task
Max                 0.000280          0.000261                 1          1
Mean                0.000206          0.000186
Min                 0.000131          0.000109                 3          3
Stddev              0.000063          0.000061
Aggregate           0.001644          0.001492  90.73
---------------------------------------------------------------------------
@--- Aggregate Time (top twenty, descending, milliseconds) ----------------
---------------------------------------------------------------------------
Call                 Site       Time    App%    MPI%      Count    COV
Barrier                 8      0.261   15.86   17.48          1   0.00
Barrier                 7      0.247   15.04   16.58          1   0.00
Barrier                 4      0.244   14.83   16.34          1   0.00
Barrier                 1      0.213   12.93   14.26          1   0.00
Barrier                 2       0.15    9.12   10.06          1   0.00
Barrier                 6      0.136    8.29    9.14          1   0.00
Barrier                 3      0.132    8.04    8.86          1   0.00
Barrier                 5      0.109    6.61    7.29          1   0.00
---------------------------------------------------------------------------
@--- Callsites: 8 ---------------------------------------------------------
---------------------------------------------------------------------------
 ID Lev File/Address        Line Parent_Funct             MPI_Call
  1   0 hello.c               14 main                     Barrier
  2   0 hello.c               14 main                     Barrier
  3   0 hello.c               14 main                     Barrier
  4   0 hello.c               14 main                     Barrier
  5   0 hello.c               14 main                     Barrier
  6   0 hello.c               14 main                     Barrier
  7   0 hello.c               14 main                     Barrier
  8   0 hello.c               14 main                     Barrier
---------------------------------------------------------------------------
@--- Callsite Time statistics (all callsites, milliseconds): 8 ------------
---------------------------------------------------------------------------
Name              Site   Tasks       Max      Mean       Min MaxRnk MinRnk
Barrier              8       1    0.2607    0.2607    0.2607      1      1
Barrier              7       1    0.2474    0.2474    0.2474      2      2
Barrier              4       1    0.2438    0.2438    0.2438      5      5
Barrier              1       1    0.2127    0.2127    0.2127      0      0
Barrier              2       1      0.15      0.15      0.15      7      7
Barrier              6       1    0.1364    0.1364    0.1364      6      6
Barrier              3       1    0.1321    0.1321    0.1321      4      4
Barrier              5       1    0.1087    0.1087    0.1087      3      3
---------------------------------------------------------------------------
@--- End of Report --------------------------------------------------------
---------------------------------------------------------------------------
```

## Hands-on exercises

### Run your MPI program with mpiP enabled

What are the most time consuming communication operations?

What is the balance of communication to application time?
