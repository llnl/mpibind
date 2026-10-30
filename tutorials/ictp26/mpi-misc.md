
# MPI Observations

*Edgar A. León*<br>
Lawrence Livermore National Laboratory

## Table of contents

1. [Open MPI affinity](#1-open-mpi-affinity)
1. [MPI send operations](#2-mpi-send-operations)
1. [The classic MPI Send deadlock](#3-the-classic-mpi-send-deadlock)
1. [The MPI progress engine](#4-the-mpi-progress-engine)
1. [Diagnosing scaling results](#5-diagnosing-scaling-results)

## 1. Open MPI affinity

``` bash
mpirun --map-by ppr:R:node:PE=T --bind-to core
```

This command controls process mapping, ranking, and binding in Open
MPI to optimize how parallel tasks utilize a cluster's CPU
architecture. It specifies exactly how many MPI processes to run per
node, reserves computing resources for multi-threaded applications,
and restricts processes to specific hardware cores to maximize cache
locality.

Here is the breakdown of what each part of `mpirun --map-by ppr:R:node:PE=T --bind-to core` does:

1. `--map-by ppr:R:node` (Processes Per Resource)
   * What it does: It launches exactly R MPI processes per cluster node.
   * The Syntax: ppr stands for "Processes Per Resource." The structure follows ppr:N:resource.
   * Workshop Analogy: If you have 4 nodes and set R=2, Open MPI will place exactly 2 processes on Node 1, 2 on Node 2, and so on, regardless of how many CPU cores those nodes actually have.

2. `:PE=T` (Processing Elements)
   * What it does: It allocates T hardware threads (cores) to each MPI process.
   * Why use it: This is the standard way to configure hybrid programming (MPI + OpenMP). It tells the Open MPI launcher to space out the MPI processes so that each process has a dedicated "pocket" of T cores to spawn its own local OpenMP worker threads without overlapping with other MPI processes.
   * Workshop Analogy: If T=4, each MPI process is given a "workspace" of 4 cores to run its own internal sub-threads.

3. `--bind-to core`
   * What it does: It pins (binds) each MPI process to a specific physical core (or a specific set of cores defined by PE=T).
   * Why use it: By default, operating systems like to move processes around from core to core to balance the thermal load. In HPC, this is terrible for performance because moving a process wipes out its CPU cache (causing "cache misses"). Binding forces the process to stay put, ensuring incredibly fast data access.
   * How it interacts with PE=T: Because PE=T is active, each MPI process will be strictly bound to a specific set of T cores, rather than just a single core.

### Summary Example

``` bash
mpirun --map-by ppr:2:node:PE=4 --bind-to core -np 4 ./my_hybrid_program
```

Open MPI will:
1. Identify how many nodes are available.
2. Place 2 MPI processes on each node.
3. Reserve 4 cores for each of those processes (perfect for setting OMP_NUM_THREADS=4).
4. Bind those processes to their respective 4-core blocks so they never drift across the CPU.


## 2. MPI send operations

``` bash
MPI_Send()
MPI_Isend()
MPI_Put()
```


The underlying MPI library switches between different **protocols and memory transfer mechanisms** based on the size of the data and whether the operation is cooperative or one-sided.

Here is what happens behind the scenes in a modern MPI implementation (like Open MPI or MPICH):


### 1. Cooperative (Two-Sided) Communication: `MPI_Send` vs. `MPI_Isend`
For two-sided operations, the sender and receiver must match. To handle this efficiently, MPI implementations choose a protocol dynamically based on data size thresholds (often configurable via environment variables, usually around 16KB–64KB).

#### Short Messages: The Eager Protocol
Used when the data is small enough to be sent immediately without waiting for the receiver to say it's ready.

*   **Behind the Scenes:**
    1.  **Allocation:** The sender wraps the data in an envelope (metadata containing tag, communicator, rank) and copies it into a pre-allocated internal MPI buffer on the network card or host memory.
    2.  **Transmission:** The sender fires the data across the fabric immediately. The `MPI_Send` or `MPI_Isend` call on the sender side completes almost instantly.
    3.  **Buffering:** On the receiving node, if the matching `MPI_Recv` hasn't been called yet, the MPI runtime drops the data into an internal **unexpected message queue**.
    4.  **Delivery:** When the receiver finally calls `MPI_Recv`, MPI copies the data from its internal "unexpected" buffer into the user's actual receive buffer.
*   **The Difference:**
    *   `MPI_Send` blocks only until its data is safely out of the user's buffer and into the local MPI buffer.
    *   `MPI_Isend` starts this process and returns immediately, giving the user a request handle to track completion.

#### Large Messages: The Rendezvous Protocol
Used for large datasets to prevent the sender from overwhelming the receiver's internal memory buffers.

*   **Behind the Scenes:**
    1.  **The Handshake:** The sender transmits *only* a small "Ready to Send" (RTS) handshake message containing the envelope data.
    2.  **The Wait:** The sender stalls. The data stays put in the sender's user memory buffer.
    3.  **The Match:** On the receiving node, MPI waits until the user calls `MPI_Recv` to match the incoming RTS envelope.
    4.  **The Clear:** The receiver allocates its user buffer and sends back a "Clear to Send" (CTS) acknowledgment.
    5.  **Zero-Copy Transfer:** Upon receiving the CTS, the sender initiates a direct transfer—often utilizing hardware-level **RDMA (Remote Direct Memory Access)**—moving data directly from the sender's user buffer to the receiver's user buffer, bypassing intermediate OS or MPI buffers entirely.
*   **The Difference:**
    *   `MPI_Send` will **physically block** and not return until the entire handshake completes and data transfer is underway or finished.
    *   `MPI_Isend` returns immediately after posting the RTS message, allowing the CPU to do other math while the background progression engine handles the handshake and RDMA transfer.


### 2. One-Sided Communication: `MPI_Put`
`MPI_Put` belongs to Remote Memory Access (RMA). It decouples the sender from the receiver; the sender pushes data directly into a remote process's memory without that process actively calling a matching receive function.

*   **Behind the Scenes:**
    1.  **The Window:** Before any transfer can happen, all processes must call `MPI_Win_allocate` or `MPI_Win_create`. This registers a specific segment of their local memory with the network interface card (NIC), effectively making it "public" to the network.
    2.  **The Push:** When a process calls `MPI_Put`, it specifies the target rank and the memory displacement inside that remote window.
    3.  **Hardware Acceleration (RDMA Write):** If the cluster uses an HPC fabric like InfiniBand, RoCE, or Omni-Path, the local NIC bypasses the remote node's CPU entirely. It issues a network-level **RDMA Write** packet directly to the remote NIC, which modifies the remote RAM on its own.
    4.  **The Catch (Synchronization):** Because the remote CPU doesn't know its memory was just altered, `MPI_Put` is inherently **non-blocking and asynchronous**. The data is not guaranteed to be safely delivered or visible until a synchronization barrier is called (like `MPI_Win_fence`, or `MPI_Win_lock`/`MPI_Win_unlock`).


### Comparison Table

| Operation | Protocol/Mechanism | Does it block user code? | Does it require remote CPU help? | Main Performance Hazard |
| :--- | :--- | :--- | :--- | :--- |
| **`MPI_Send`** | Eager (Small) / Rendezvous (Large) | **Yes**, until safe to reuse buffer. | Yes, requires matching `MPI_Recv`. | Can cause deadlocks if pairs are poorly ordered. |
| **`MPI_Isend`** | Eager (Small) / Rendezvous (Large) | **No**, returns immediately. | Yes, requires matching `MPI_Recv`. | Memory corruption if user alters buffer before `MPI_Wait`. |
| **`MPI_Put`** | One-Sided RDMA Write | **No**, returns immediately. | **No**, bypasses remote CPU entirely. | Data race conditions if synchronization points are omitted. |


## 3. The classic MPI Send deadlock
<!-- The "After You" Problem -->

One of the most common bugs in writing MPI code is a **deadlock** caused by a naive ordering of synchronous or large-message `MPI_Send` calls.

Because `MPI_Send` uses the **Rendezvous Protocol** for large data sizes, it will block execution until the receiving process explicitly acknowledges and matches the transfer. If all processes are waiting for someone else to receive, the entire program freezes permanently.


### The Broken Code Pattern
Imagine two processes (Rank 0 and Rank 1) trying to swap large data arrays with each other. A student might write the logic symmetrically like this:

```c
// RUNNING ON BOTH PROCESSES (Rank 0 and Rank 1)

if (rank == 0) {
    // Rank 0 tries to send first, then receive
    MPI_Send(send_buffer, LARGE_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
    MPI_Recv(recv_buffer, LARGE_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
else if (rank == 1) {
    // Rank 1 tries to send first, then receive
    MPI_Send(send_buffer, LARGE_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Recv(recv_buffer, LARGE_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
```

### Behind the Scenes: Why It Freezes
When the message size is large, the implementation switches away from the Eager protocol. Here is exactly what happens step-by-step:

1. **Rank 0** executes its `MPI_Send`. It sends a "Ready to Send" (RTS) handshake packet to Rank 1 and **blocks (halts execution)**, waiting for a "Clear to Send" (CTS) reply.
2. **Rank 1** simultaneously executes its `MPI_Send`. It sends an RTS packet to Rank 0 and **blocks (halts execution)**, waiting for a CTS reply.
3. **The Deadlock:** Rank 0 cannot move forward to execute its `MPI_Recv` (which would clear Rank 1's send) because it is stuck waiting on its own send. Rank 1 is in the exact same state.

*Both processes are trapped in line waiting for the other to pick up the phone, and neither can move to the next line to answer it.*

> **⚠️ The "Small Data" Trap:** If students test this broken code with small arrays (e.g., 5 integers), it might actually work! This is because small data uses the **Eager Protocol**, where the MPI runtime copies the data into an internal buffer and lets `MPI_Send` return immediately. When they scale their homework up to real production sizes, the code will suddenly freeze.


### How to Fix It (3 Solutions)

#### Solution A: Serialization (The Handshake Fix)
The easiest conceptual fix is to break the symmetry. Force one process to receive first while the other sends.

```c
if (rank == 0) {
    MPI_Send(send_buffer, LARGE_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
    MPI_Recv(recv_buffer, LARGE_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
} else if (rank == 1) {
    // Reverse the order on Rank 1!
    MPI_Recv(recv_buffer, LARGE_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Send(send_buffer, LARGE_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD);
}
```

#### Solution B: Use Non-Blocking Communication (`MPI_Isend`)
By changing the sends to non-blocking, both processes post their "Ready to Send" notice immediately and move on to the receive code without blocking.

```c
MPI_Request request;

if (rank == 0) {
    MPI_Isend(send_buffer, LARGE_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
    MPI_Recv(recv_buffer, LARGE_SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Wait(&request, MPI_STATUS_IGNORE); // Ensure send finished before reusing buffer
} else if (rank == 1) {
    MPI_Isend(send_buffer, LARGE_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
    MPI_Recv(recv_buffer, LARGE_SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Wait(&request, MPI_STATUS_IGNORE);
}
```

#### Solution C: Use `MPI_Sendrecv` (The Professional Choice)
MPI provides a built-in, highly optimized function explicitly designed to handle safe, simultaneous swapping without deadlocking.

```c
// Safe for both ranks to run simultaneously!
MPI_Sendrecv(send_buffer, LARGE_SIZE, MPI_INT, target_rank, 0,
             recv_buffer, LARGE_SIZE, MPI_INT, target_rank, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
```


## 4. The MPI progress engine

When using non-blocking communication (`MPI_Isend`, `MPI_Irecv`) or one-sided RMA (`MPI_Put`), they often assume that data moves across the network entirely on its own while their CPU does other math.

In reality, data transfer requires a mechanism called the **MPI Progress Engine**. Understanding how the engine makes "progress" is critical to write efficient, overlapped parallel code.


### What is the Progress Engine?
The progress engine is the internal subsystem within the MPI library (such as Open MPI or MPICH) responsible for:
1.  **Polling the network hardware** for incoming data or control packets.
2.  **Handling handshakes** (like the Rendezvous protocol's RTS/CTS).
3.  **Moving data** out of internal unexpected queues into user buffers.

The core rule students must learn is: **MPI code only moves data when the progress engine is actively kicked.**


### How the Engine Gets Kicked: Two Models

Depending on the cluster's network hardware and your MPI configuration, the progress engine operates in one of two modes:

#### 1. Polling (Manual Progress) — *The Standard Default*
In most standard HPC clusters, the MPI library does *not* spawn a background OS thread to manage the network. Instead, the progress engine runs on the **same CPU thread** as the student's application code.

*   **How it works:** The engine only advances when the application explicitly calls an MPI function.
*   **The Workflow:**
    1.  Student calls `MPI_Isend(...)`. The engine initializes the transfer and immediately returns control to the user code.
    2.  Student runs a heavy 10-second compute loop (e.g., matrix multiplication). **During these 10 seconds, zero network progress is made.** The network card sits completely idle because the CPU is 100% busy with math and never checks the network stack.
    3.  Student finally calls `MPI_Wait(...)` or `MPI_Test(...)`. This explicitly kicks the progress engine. The engine notices the network, finishes the handshake, and actually pumps the data across the fabric.
*   **The Catch:** True computation/communication overlap **does not happen automatically** in this mode.

#### 2. Asynchronous Progress (Threaded Progress) — *The Ideal Scenario*
If enabled via environment variables or supported directly by high-end network hardware (like InfiniBand or Omni-Path with hardware trigger engines), MPI can make progress asynchronously.

*   **How it works:** The MPI runtime handles progress using one of two methods:
    *   **Software-based:** Spawning a hidden, dedicated background helper thread on the OS to constantly run the progress engine loop.
    *   **Hardware-assisted (Triggered/Offloaded):** Offloading the progress logic entirely onto the Network Interface Card (NIC) processors (e.g., Mellanox/NVIDIA CORE-Direct).
*   **The Workflow:** While the student's main CPU thread is completely buried in that 10-second math loop, the background thread or the smart network card independently manages handshakes and pushes data.
*   **The Result:** When the student finally hits `MPI_Wait`, the transfer is already 100% finished. **True overlap is achieved.**


### Workshop Advice: How to Ensure Progress

If running on a cluster that defaults to the standard **Polling (Manual)** model, these two optimization tricks prevent the code from stalling:

#### Trick 1: Sprinkle `MPI_Test` Inside Heavy Loops
If a computation loop takes a long time, the data transfer will freeze
until the loop ends. One can manually "kick" the progress engine by calling `MPI_Test` inside the loop to check on the request handle without blocking.

```c
// Manually keeping the progress engine alive during math
for (int i = 0; i < massive_iterations; i++) {
    compute_subgrid(i);

    // Every 1000 iterations, quickly kick the engine
    if (i % 1000 == 0) {
        int flag;
        MPI_Test(&request, &flag, MPI_STATUS_IGNORE);
    }
}
MPI_Wait(&request, MPI_STATUS_IGNORE);
```

#### Trick 2: Enable Asynchronous Flags
Check if the MPI implementation supports turning on background threads via environment variables during `mpirun` or `srun` execution.

*   **For Open MPI:**
    ```bash
    export OMPI_MCA_opal_common_ucx_opal_mem_hooks=1
    mpirun --mca btl_openib_receive_queues ...
    ```
*   **For MPICH / Intel MPI:**
    ```bash
    export MPICH_ASYNC_PROGRESS=1
    export I_MPI_ASYNC_PROGRESS=1
    ```
    *(Note: Using software async progress threads usually requires reserving a CPU core via taskset or `PE=T` controls so the background thread doesn't fight the user's math thread for performance!)*


## 5. Diagnosing scaling results

When transitioning MPI applications from a single process to multiple
processes, or from a single node to multiple cluster nodes, "weird"
scaling behavior is often encountered. Common symptoms include:
*   Performance **dropping sharply** when crossing from 1 node to 2 nodes.
*   Speedups that **plateau** after a certain number of cores.
*   **Erratic fluctuations** in runtime where adding more cores makes the code slower.

Here is a systematic, step-by-step strategy to isolate and identify the root causes.


### Step 1: Isolate the Core Boundary (Internal vs. External Network)
The very first diagnostic test is determining *where* the performance scaling breaks down. Test your application across three specific configurations keeping the total process count identical (e.g., 16 processes):

1.  **Pure Intra-node:** Run all 16 processes on **1 single node** (using shared memory).
2.  **Inter-node Spread:** Run 16 processes spread across **2 nodes** (e.g., 8 per node).
3.  **Maximum Inter-node Spread:** Run 16 processes spread across **16 nodes** (1 process per node).

**What the results mean:**
*   If it scales fine on 1 node but falls off a cliff on 2 nodes, the bottleneck is **Network Latency or Bandwidth** (the code is spending too much time sending data across the physical network switch compared to shared memory).
*   If it scales poorly even on a single node, the bottleneck is local: **Memory Bandwidth starvation, NUMA effects, or bad process binding**.


### Step 2: Use MPI Profiling Tools (The "Where is Time Spent?" Test)
Don't guess what the code is doing. Use lightweight, low-overhead profiling tools to get a breakdown of Compute Time vs. Communication Time.

*   **The Quick & Dirty Tool: IPM (Integrated Performance Monitoring)**
    *   Adding IPM to a batch script is usually as simple as loading a module or preloading a library. It generates an HTML webpage showing exactly what percentage of runtime was spent inside `MPI_Wait`, `MPI_Allreduce`, or calculation loops.
*   **The Visualization Tool: Score-P / Vampir or Tau**
    *   These tools generate a timeline trace. One can visually if one process is sitting idle for milliseconds waiting for a lagging process to finish its loop (Load Imbalance).


### Step 3: Run the Diagnostic Checklist

If the profiling tools reveal that the code is stalling, investigate these 4 classic HPC scaling bottlenecks:

#### 1. The Amdahl’s Law Bottleneck (Serial Fraction)
*   **The Culprit:** A piece of the code is still running sequentially. For example, Rank 0 reads a massive input file or initializes a giant array while all other ranks sit in an `MPI_Barrier` waiting.
*   **How to check:** Measure the time spent *strictly* inside I/O or setup code as they increase the core count. If that time remains constant while compute time shrinks, it will quickly dominate the runtime.

#### 2. Load Imbalance
*   **The Culprit:** The work is unevenly divided. If Rank 0 gets a heavy boundary grid cell to compute while Rank 1 gets an empty cell, Rank 1 finishes instantly and sits idle. Your scaling is always bottlenecked by the *slowest single process*.
*   **How to check:** Print out the execution time of the compute loop on *every* rank. If the maximum time is vastly larger than the minimum time, they have a load balancing problem.

#### 3. Hardware Saturation (The Memory Wall)
*   **The Culprit:** Modern CPUs have multiple cores but share a limited number of memory channels. If an MPI code is highly **memory-bandwidth bound** (moving massive arrays in and out of RAM constantly), 4 processes might completely max out the node's memory bus. Adding a 5th or 6th process adds zero speed because they are all choking on RAM access.
*   **How to check:** Try running fewer processes per node but spacing them out across multiple nodes (e.g., instead of 32 processes on 1 node, try 16 processes on Node A and 16 on Node B). If performance shoots up, they were bottlenecked by the local memory architecture.

#### 4. The Eager-to-Rendezvous Threshold Shift
*   **The Culprit:** As mentioned in previous sections, small messages are fast (Eager protocol), large messages are slow (Rendezvous protocol).
*   **How to check:** If the scaling graph shows a sudden, bizarre spike in runtime at a specific problem size, they likely crossed the threshold where MPI stopped buffering messages locally and began forcing synchronous network handshakes.


### Summary Action Guide

| If the profile shows... | The root cause is likely: | Try this: |
| :--- | :--- | :--- |
| Huge time in `MPI_Barrier` or `MPI_Wait` | **Load Imbalance** or **Serial I/O** | Re-divide the loop domains evenly; look into parallel I/O (`MPI_File_open`). |
| Bad scaling on 1 node past 8–12 cores | **Memory Bandwidth Starvation** | Optimize cache reuse (loop tiling) or switch to a hybrid MPI+OpenMP model. |
| Sharp performance drop when moving to 2+ nodes | **Network Latency Sensitive** | Pack small individual messages into larger contiguous structures (or use `MPI_Type_vector`). |
| Erratic, non-deterministic performance waves | **Bad Process Binding / OS noise** | Ensure they are using the `--bind-to core` and `--map-by` flags properly so the OS isn't dragging processes around. |
