# Supercomputing Foundations: Extended Edition
<!-- 6.5 hours -->

*Edgar A. León*, Lawrence Livermore National Laboratory<br>
*Suzanne Parete-Koon*, Oak Ridge National Laboratory<br>
*Jane E. Herriman*, Lawrence Livermore National Laboratory<br>
*Tony Ramirez*, Oak Ridge National Laboratory<br>
*Alan Longcoy*, Oak Ridge National Laboratory<br>


## Description

This workshop provides an interactive introduction to High-Performance Computing (HPC), combining foundational concepts with hands-on experience in parallel programming, hardware-aware application execution, and AI-enabled scientific data analysis.
Attendees will explore the basics of supercomputers, learn essential Linux skills, and gain experience with parallel programming using MPI. Through guided exercises, participants will build and run serial and parallel programs, analyze differences in execution, understand how to manage jobs in an HPC environment, and apply these skills to a real-world AI and data analytics workflow.

The workshop also introduces modern compute node architecture and topology. Participants will learn how processors, memory, GPUs, and high-speed interconnects are organized within HPC systems, and how application performance depends on effectively mapping computational workloads to available hardware resources. Using tools such as `hwloc`, attendees will explore concepts including hardware locality, processor affinity, NUMA memories, and resource discovery.

Building on this foundation, participants will learn techniques for launching and mapping parallel applications using Slurm, including constructing CPU affinities and assigning GPUs to MPI processes. The workshop emphasizes practical strategies for improving application performance through efficient resource placement and minimizing unnecessary data movement.
These concepts provide the foundation for understanding not only how to write parallel programs, but also how to execute them efficiently on modern supercomputers.

The workshop includes a hands-on module that demonstrates how HPC enables modern data science via machine learning and AI. Participants will work with real-world power outage data collected through the U.S. Department of Energy's EAGLE-I system, operated by Oak Ridge National Laboratory. Using parallel data processing with MPI, attendees will prepare and analyze a large dataset, visualize outage patterns, and apply the K-means clustering algorithm to identify similarities across geographic regions and seasons. By interpreting these clusters and relating them to seasonal weather trends, participants will develop hypotheses about the underlying causes of outages while gaining experience applying machine learning techniques to a real scientific dataset.

Hands-on exercises are conducted on a supercomputer, providing participants with practical experience using real HPC systems throughout the workshop.
By the end of the workshop, attendees will understand the fundamentals of HPC, parallel programming, hardware topology, resource affinity, and locality-aware application execution, as well as how these technologies enable modern scientific computing, machine learning, and data analytics.


## Learning Objectives

By the end of this workshop, participants will be able to:

* Navigate and use a Linux-based HPC environment.
* Compile and execute parallel applications.
* Understand the fundamentals of MPI programming.
* Describe the architecture and topology of modern HPC systems.
* Use hardware topology information to efficiently map applications to computing resources.
* Launch parallel applications using Slurm.
* Process and analyze a scientific dataset using parallel computing techniques.
* Apply an introductory machine learning algorithm, K-means clustering, to identify patterns in real-world data.
* Explain how HPC, data analytics, and AI work together to address scientific and engineering challenges.

## Audience and Requirements

Designed for undergraduate students, graduate students, faculty, and researchers seeking an introduction to HPC and its growing role in scientific computing and data science.

Attendees will need a laptop equipped with Wi-Fi, a shell terminal,
  and the `ssh` program. Users will be provided accounts
  to access a cluster environment required for
  demonstrations and hands-on exercises.

<!--
* Attendees should have a working knowledge of Unix-like systems. For
  example, they should know how to navigate a filesystem and launch
  applications from the command line.

* Attendees will also need some familiarity with high-level parallel
  programming concepts. For example, attendees should be comfortable
  with terms like thread, process, and GPU, but do not need experience
  writing parallel programs.
-->


## Schedule

<center>

| Begin | End | Topic |
|-:|-:|:-|
| 08:00 | 08:15 | Paperwork |
| 08:15 | 08:30 | Introduction to Supercomputing |
| 08:30 | 09:30 | Module 1: Linux Basics |
| 09:30 | 10:00 | Module 2: Parallel Computing with MPI |
| *10:00* | *10:15* | *Break* |
| 10:15 | 10:45 | Module 2: Continued |
| 10:45 | 11:30 | Module 3: AI-Enabled Scientific Data Analysis |
| *11:30* | *12:00* | *Lunch* |
| 12:00 | 12:45 | Module 3: Continued |
| 12:45 | 13:45 | Module 4: Computing Architecture and Topology |
| 13:45 | 14:00 | Module 5: Hardware Affinity for Applications |
| *14:00* | *14:15* | *Break* |
| 14:15 | 15:00 | Module 5: Continued |

</center>


<!--
## AWS Cluster

Accounts: `user5`, `user6`, ..., `user35`

Password:

```
ssh user5@

source /home/tutorial/scripts/user-env.sh

srun -N1 -n1 mpi
```
-->

## Notebook

<!--
<br>
<p align="center">
   <img src="../figures/sierra.png" width="750"/>
</p>
-->

1. Linux Basics.<br>*Learn to use an HPC system.*

   Learn the essentials of working in Linux systems, which form the backbone of most HPC environments. This module introduces participants to foundational Linux concepts, including navigating the file system, managing and searching files, and performing basic edits with the popular text editor `vim`. By the end, participants will be comfortable moving around a Linux system and handling core tasks needed for effective HPC use.


1. [Parallel Computing with MPI](https://github.com/suzannepk/mpi_parallel).<br>*Learn to write parallel programs.*

   Learn how to develop parallel applications using MPI (Message Passing Interface), a foundational tool in high-performance computing. This module introduces the principles of parallel thinking and guides participants through the transformation of serial code into parallel implementations using MPI. Through hands-on examples you’ll explore data distribution, rank-based processing, and communication overhead, while gaining practical experience running programs across multiple processes. By the end, you'll understand how to analyze performance, improve efficiency, and apply these techniques to more complex real-world problems.

1. [AI-Enabled Scientific Data Analysis](https://github.com/suzannepk/K_means.git).<br>
*Apply parallel programming and machine learning to address a scientific data analytics problem.*<br>
   [Slides](k-means-final.pdf).

   Discover how HPC enables modern data science and machine learning through the analysis of real-world scientific data. Participants will work with outage data from the U.S. Department of Energy's EAGLE-I system, which monitors electric power disruptions across the United States.

   Using MPI, attendees will preprocess and organize a large dataset before applying visualization techniques and the K-means clustering algorithm to identify patterns in outage behavior across regions and seasons. Participants will interpret the resulting clusters, formulate hypotheses about the environmental and seasonal factors influencing power outages, and explore how computational methods support energy resilience and disaster response.

   This module demonstrates how HPC, parallel programming, and machine learning complement one another to solve scientific and engineering problems.

1. [Computing Architecture and Topology](topology-module.md).<br>
*Learn how HPC hardware is organized.*

   Learn how to identify the compute and memory components of a
   compute node using `hwloc`. A precise understanding of the hardware
   resources is needed to map an application to the machine
   efficiently. This includes identifying the node's GPUs, cores,
   hardware threads, cache hierarchy, NUMA domains, and network
   interfaces. Furthermore, attendees will be introduced to locality,
   will identify local hardware resources, and will select resources
   using affinity masks.

1. Hardware Affinity for Applications.<br>
*Learn how to use that hardware efficiently.*

   Learn how to use the resource manager to map a parallel
   program to the
   hardware at runtime when submitting a job. Attendees will learn to
   construct CPU-based mappings using low-level and high-level
   abstractions as well as GPU-based mappings. Attendees will learn to
   manage CPU and GPU affinity concurrently to take advantage of local
   resources and reduce data movement.

   1. [Mapping Processes to the Hardware](map-cpu.md)

   1. [Adding in GPU kernels](map-gpu.md)
