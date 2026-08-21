# Supercomputing Foundations: Accelerated Edition
<!-- 4 hours -->

*Edgar A. León*, Lawrence Livermore National Laboratory<br>
*Suzanne Parete-Koon*, Oak Ridge National Laboratory<br>
*Jane E. Herriman*, Lawrence Livermore National Laboratory<br>
*Tony Ramirez*, Oak Ridge National Laboratory<br>
*Alan Longcoy*, Oak Ridge National Laboratory<br>

## Description

This accelerated workshop provides an interactive introduction to High-Performance Computing (HPC), combining foundational concepts with hands-on experience in parallel programming and scientific data analysis. Attendees will explore the basics of supercomputers, learn essential Linux skills, and gain experience with parallel programming using MPI. Through guided exercises, participants will build and run serial and parallel programs, compare their execution, understand how to manage jobs in an HPC environment, and apply these skills to a real-world AI and data analytics workflow.

The workshop concludes with a hands-on module that demonstrates how HPC enables modern data science and AI. Participants will work with real-world power outage data collected through the U.S. Department of Energy's EAGLE-I system, operated by Oak Ridge National Laboratory. Using parallel data processing with MPI, attendees will prepare and analyze a large dataset, visualize outage patterns, and apply the K-means clustering algorithm to identify similarities across geographic regions and seasons. By interpreting these clusters and relating them to seasonal weather trends, participants will develop hypotheses about the underlying causes of outages while gaining experience applying machine learning techniques to a real scientific dataset.

Hands-on exercises are conducted on a supercomputer, providing participants with practical experience using real HPC systems throughout the workshop. By the end of the workshop, attendees will understand the fundamentals of HPC, parallel programming, and how these technologies enable modern scientific computing and AI-enabled data analytics. 

## Learning Objectives

By the end of this workshop, participants will be able to:

* Navigate and use a Linux-based HPC environment.
* Compile and execute parallel applications using MPI.
* Understand the fundamentals of distributed-memory parallel programming.
* Process and analyze a scientific dataset using parallel computing techniques.
* Apply an introductory machine learning algorithm, K-means clustering, to identify patterns in real-world data.
* Explain how HPC, data analytics, and AI work together to address scientific and engineering challenges.

## Audience and Requirements

Designed for undergraduate students, graduate students, faculty, and researchers seeking an introduction to HPC and its growing role in scientific computing and data science.

Attendees will need a laptop equipped with Wi-Fi, a shell terminal, and the `ssh` program. Participants will be provided accounts to access an HPC cluster used for demonstrations and hands-on exercises.



## Schedule 

<center>

| Begin | End | Topic |
|-:|-:|:-|
| 08:00 | 08:15 | Paperwork | 
| 08:15 | 08:30 | Introduction to Supercomputing |
| 08:30 | 09:15 | Module 1: Linux Basics | 
| 09:15 | 10:15 | Module 2: Parallel Computing with MPI |
| *10:15* | *10:30* | *Break* |
| 10:30 | 12:00 | Module 3: AI-Enabled Scientific Data Analysis |

</center>



## Notebook

1. UNIX Basics.<br>
   *Learn to use an HPC system.*

   Learn the essentials of working in Linux systems, which form the backbone of most HPC environments. This module introduces participants to foundational UNIX concepts, including navigating the file system, managing and searching files, and performing basic edits with the text editor `vim`. By the end, participants will be comfortable moving around a Linux system and performing the core tasks required for effective HPC use.

2. [Parallel Computing with MPI](https://github.com/suzannepk/mpi_parallel).<br>
   *Learn to write parallel programs.*

   Learn how to develop parallel applications using MPI (Message Passing Interface), a foundational programming model in high-performance computing. Through hands-on examples, participants will transform serial programs into parallel implementations, explore data distribution and inter-process communication, and gain practical experience executing applications across multiple processes.

3. [AI-Enabled Scientific Data Analysis](https://github.com/suzannepk/K_means.git).<br>
   *Apply your HPC skills to a realistic scientific data analytics and machine learning problem.*

   Discover how HPC enables modern data science and machine learning through the analysis of real-world scientific data. Participants will work with outage data from the U.S. Department of Energy's EAGLE-I system, using MPI to preprocess and organize a large dataset before applying visualization techniques and the K-means clustering algorithm to identify patterns in outage behavior across regions and seasons.

   Participants will interpret the resulting clusters, formulate hypotheses about the environmental and seasonal factors influencing power outages, and explore how computational methods support energy resilience and disaster response.

   This capstone module demonstrates how parallel programming, scientific data analysis, and machine learning come together to solve real-world problems using HPC.

