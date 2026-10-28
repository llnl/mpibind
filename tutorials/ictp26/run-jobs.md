# Running Parallel Jobs

*Edgar A. León*<br>
Lawrence Livermore National Laboratory

## Table of contents

1. [Learning objectives](#learning-objectives)
1. [Workshop setup](#workshop-setup)
1. [Machine partitions](#machine-partitions)
1. [Running jobs](#running-jobs)
1. [Hands-on exercises](#hands-on-exercises)

## Learning objectives

* Become familiar with Leonardo's different compute partitions
* Learn how to load the workshop's environment
* Learn how to submit parallel jobs interactively
* Learn how to submit parallel jobs in batch mode
* Become familiar with basic Slurm commands for submitting and
  managing jobs


## Workshop setup

We will be using Leonardo for demonstrations and hands-on
exercises, which require loading an environment as follows:

1. Authenticate your identity via CINECA's Single Sign-On (SSO) and
   generate a short-lived SSH certificate to connect to CINECA supercomputers
   ```bash
   # Assumes your local Smallstep CLI client has been configured to
   # trust and communicate with CINECA's Certificate Authority (CA)
   $ step ssh login '<user-email>' --provisioner cineca-hpc
   ```
2. Access Leonardo via secure shell
   ```bash
   $ ssh -X <username>@login.leonardo.cineca.it
   ```
3. Load the workshop's environment
   ```bash
   source /leonardo_scratch/fast/ICT26_MHPC_0/leon/scripts/user-env.sh
   ```

   Once the environment is loaded, use the environment variable
   `MAP` to access the workshop's material.

4. Test the environment is working
   ```bash
   # Booster
   $ srun_boost -N1 -n4 -t1 -c8 -qboost_qos_dbg --cpu-bind=core --gpu-bind=single:1 mpi+gpu
     0 lrdn0218.leonardo.local   1 GPUs: 0000:1D:00.0
     0 lrdn0218.leonardo.local   8 CPUs: 0-7
     1 lrdn0218.leonardo.local   1 GPUs: 0000:56:00.0
     1 lrdn0218.leonardo.local   8 CPUs: 8-15
     2 lrdn0218.leonardo.local   1 GPUs: 0000:8F:00.0
     2 lrdn0218.leonardo.local   8 CPUs: 16-23
     3 lrdn0218.leonardo.local   1 GPUs: 0000:C8:00.0
     3 lrdn0218.leonardo.local   8 CPUs: 24-31
   ```
   ```bash
   # DCGP
   $ srun_dcgp -N1 -n4 -t1 -c28 -qdcgp_qos_dbg --cpu-bind=core mpi
     0 lrdn4931.leonardo.local  28 CPUs: 0-27
     1 lrdn4931.leonardo.local  28 CPUs: 28-55
     2 lrdn4931.leonardo.local  28 CPUs: 56-83
     3 lrdn4931.leonardo.local  28 CPUs: 84-111
   ```


Use this [link](https://docs.hpc.cineca.it/general/access.html) for
more information about accessing Leonardo.


## Machine partitions

Relevant Slurm parameters
```
-A, --account=<account>
-p, --partition=<partition_names>
-q, --qos=<qos>
```

|  | Booster | DCGP |
|--:|:--:|:--:|
| Specs | 2x16 Intel SMT-1 cores + 4 NVIDIA GPUs | 2x56 Intel SMT-1 cores |
| Account | `ict26_mhpc_0` |  `ict26_mhpc` |
| Partition | `boost_usr_prod` | `dcgp_usr_prod` |
| QOS | `boost_qos_dbg`<br>max 8 nodes, 30 mins | `dcgp_qos_dbg`<br>max 2 nodes, 30 mins |
| QOS | `boost_qos_lprod`<br>max 8 nodes | `dcgp_qos_lprod`<br>max 3 nodes |
| QOS | `normal`<br>max 64 nodes | `normal`<br>max 16 nodes |
| QOS | `boost_qos_bprod`<br>min 65 nodes, max 256 nodes | `dcgp_qos_bprod`<br>min 17 nodes, max 128 nodes |

``` bash
$ alias srun_boost
alias srun_boost='srun -Aict26_mhpc_0 -pboost_usr_prod --gres=gpu:4'
```

``` bash
$ alias srun_dcgp
alias srun_dcgp='srun -Aict26_mhpc -pdcgp_usr_prod --gres=tmpfs:10g'
```

Use this
[link](https://docs.hpc.cineca.it/hpc/leonardo.html#leonardo-card) for
more information about Leonardo's partitions.


## Running jobs

We will be using the Slurm resource manager to launch jobs into a
cluster.

We will consider two types of jobs:
* **Interactive** jobs
* **Batch** jobs

And, two types of nodes:
* **Login** nodes
* **Compute** nodes

When logging into Leonardo, the landing node is a login node.

``` bash
% ssh leonardo
[eleonbor@login07 ~]$ hostname
login07.leonardo.local
```

From here, one can launch interactive or batch jobs.

### Interactive jobs

Relevant parameters
```
--exclusive                # For job allocations
--pty                      # Execute task zero in pseudo terminal mode
-t, --time=<time>          # MM, MM:SS, HH:MM:SS
```

We will launch interactive jobs in two ways:
1. **Allocate and run**

   On the login node, we execute an `srun` command, which will
   allocate the requested resources and run the given command on those
   resources

1. **Allocate first, then run**

   On the login node, we execute an `salloc` command first to allocate
   compute resources. Once the request is satisfied, we can run a
   number of `srun` commands on those allocated resources from the
   login node. Alternatively, we can jump into a compute node of the
   allocation and run commands on that node.


We've already seen examples of *allocate and run*. Here are two more:
``` bash
[eleonbor@login07 scripts]$ srun_dcgp -N1 -n2 -t1 --qos=dcgp_qos_dbg hostname
lrdn4941.leonardo.local
lrdn4941.leonardo.local

[eleonbor@login07 scripts]$ srun_dcgp -N1 -n4 -t1 -c28 --qos=dcgp_qos_dbg mpi
  0 lrdn4301.leonardo.local 112 CPUs: 0-111
  1 lrdn4301.leonardo.local 112 CPUs: 0-111
  2 lrdn4301.leonardo.local 112 CPUs: 0-111
  3 lrdn4301.leonardo.local 112 CPUs: 0-111
```

Note how the node where we launched the command (login node) is
different than the node used to execute the command. Furthermore, note
that the compute node chosen for the first command is different than
the one used for the second command since each request is independent
of each other.

Let's now see some examples of *allocate first, then run*.

``` bash
# Allocate a compute node first
[eleonbor@login07 scripts]$ salloc_boost -N1 -t30 --exclusive -qboost_qos_dbg
salloc: Pending job allocation 61868591
salloc: job 61868591 queued and waiting for resources
salloc: job 61868591 has been allocated resources
salloc: Granted job allocation 61868591
salloc: Waiting for resource configuration
salloc: Nodes lrdn3267 are ready for job
[eleonbor@login07 scripts]$
```

``` bash
# Run commands on the same already allocated resources
[eleonbor@login07 scripts]$ srun -N1 -n2 hostname
lrdn3267.leonardo.local
lrdn3267.leonardo.local

[eleonbor@login07 scripts]$ srun -N1 -n2 mpi
  0 lrdn3267.leonardo.local  32 CPUs: 0-31
  1 lrdn3267.leonardo.local  32 CPUs: 0-31

[eleonbor@login07 scripts]$ srun -N1 -n2 mpi+gpu
  0 lrdn3267.leonardo.local  32 CPUs: 0-31
  0 lrdn3267.leonardo.local   4 GPUs: 0000:1D:00.0 0000:56:00.0 0000:8F:00.0 0000:C8:00.0
  1 lrdn3267.leonardo.local  32 CPUs: 0-31
  1 lrdn3267.leonardo.local   4 GPUs: 0000:1D:00.0 0000:56:00.0 0000:8F:00.0 0000:C8:00.0
```

Notice how all of the above commands run on the same allocated node,
i.e., lrdn3267.


#### Running a pseudo terminal on compute nodes

Relevant parameters
```
--pty        # Execute task zero in pseudo terminal mode
--overlap    # Allow steps to share resources with other steps
```

If needed, we can jump into a compute node and run commands
there. This is helpful when needing to build codes on the compute
nodes because specific libraries are only available there, e.g., CUDA or NVML.

We use the `--pty` parameter, which executes task zero with pseudo terminal mode

``` bash
[eleonbor@login07 scripts]$ srun -N1 -n1 -c32 --pty bash

[eleonbor@lrdn3267 scripts]$ hostname
lrdn3267.leonardo.local
```

In the example above we used one process. We can specify more than one
process to allow launching multiple processes in subsequent commands,
but we should use the `--overlap` parameter to avoid hangs if not
enough resources are available.

In general, the overlap parameter allows job steps to share all
resources with all other steps. By default, steps do not share
resources with other parallel steps.

Here's an example.

``` bash
[eleonbor@login07 pcluster]$ srun -N1 -n8 --pty /bin/bash

[eleonbor@lrdn0897 pcluster]$ srun -N1 -n8 --overlap mpi
  0 lrdn0897.leonardo.local  32 CPUs: 0-31
  3 lrdn0897.leonardo.local  32 CPUs: 0-31
  2 lrdn0897.leonardo.local  32 CPUs: 0-31
  1 lrdn0897.leonardo.local  32 CPUs: 0-31
  5 lrdn0897.leonardo.local  32 CPUs: 0-31
  7 lrdn0897.leonardo.local  32 CPUs: 0-31
  6 lrdn0897.leonardo.local  32 CPUs: 0-31
  4 lrdn0897.leonardo.local  32 CPUs: 0-31
```


### Batch jobs

Relevant commands
```
sbatch
squeue
sinfo
scancel
scontrol
```

A batch job is the result of submitting a non-blocking job specified by a
script. We employ the `sbatch` command for this purpose. When this
command is executed the terminal is not blocked and we can continue
executing more commands. We can check the status of the job using the
`squeue` command. When the job is complete, Slurm writes the output
and error streams into files the user can review.

The workflow is as follows:
1. Submit the non-blocking job
2. Check the status of the job
3. When job is completed, review the output file(s)

Let's see an example of this.
``` bash
# Submit the job
[eleonbor@login07 pcluster]$ sbatch submit-slurm-leonardo.sh
Submitted batch job 61984537

# Check the status of the job
[eleonbor@login07 pcluster]$ squeue -j 61984537
[..]

# Alternatively, check the status of all of my jobs
[eleonbor@login07 pcluster]$ squeue -u eleonbor
JOBID    PARTITION NAME     USER      ST TIME  NODES NODELIST(REASON)
61984537 boost_usr ompi_job eleonbor  R  0:05      1 lrdn3098

# Review the output
[eleonbor@login07 pcluster]$ cat ompi_job_61984537.out
[..]
```

We will now review the structure of a batch script by looking at the
following script:

``` bash
$MAP/scripts/submit-slurm.sh
```

Use this [link](https://docs.hpc.cineca.it/hpc/hpc_scheduler.html) for
more information about monitoring jobs.


### Running on specific compute nodes

Relevant parameters

```
-x, --exclude=<nodelist>
-w, --nodelist=<nodelist>
```

We also highlight how to select or exclude
specific nodes from our jobs, which could be helpful for reproducibility.

Let's allocate 3 nodes first:

``` bash
[eleonbor@login07 scripts]$ salloc_boost -N3 -t30 --exclusive -qboost_qos_dbg
salloc: Pending job allocation 61872309
salloc: job 61872309 queued and waiting for resources
salloc: job 61872309 has been allocated resources
salloc: Granted job allocation 61872309
salloc: Waiting for resource configuration
salloc: Nodes lrdn[0148,1751,1865] are ready for job
```

And, then, run only on nodes 1751 and 1865:

``` bash
# Select the nodes of interest
[eleonbor@login07 scripts]$ srun -N2 -n2 -wlrdn[1751,1865] hostname
lrdn1865.leonardo.local
lrdn1751.leonardo.local

# Or, exclude the complement
[eleonbor@login07 scripts]$ srun -N2 -n2 -xlrdn[0148] hostname
lrdn1751.leonardo.local
lrdn1865.leonardo.local
```


## Hands-on exercises

Make sure you specify appropriate time limits (`-t`) to acquire
compute nodes promptly

### 1. Build your MPI program(s) using Open MPI on a compute node

Note that the goal is to build on a compute node rather than a login
node

Make sure your program records a **performance metric**, e.g., execution
time, throughput, figure of merit.

An example measuring parallel execution time is here:

``` bash
$MAP/mpi-hello/runtime.c
```

Try running the `runtime` program to make sure you understand how time
is measured.

### 2. Run your program on two compute nodes interactively

Make sure you can run with at least these two configurations:
* 1 process per node
* 4 processes per node

### 3. Run your program on two compute nodes via a batch script

Run using the configurations from the previous exercise

### 4. Start building your optimization matrix

Record your program's performance for every configuration you test. We
will cover many configurations during this workshop.

At the end of the workshop, let's analyze what work best.

| | App 1 Performance | App 2 Performance |
|--|--|--|
| Configuration 1 |
| Configuration 2 |
| Configuration 3 |


| | Configuration 1 | Configuration 2 |
|--|--|--|
| Parameter 1 |
| Parameter 2 |
| Parameter 3 |
