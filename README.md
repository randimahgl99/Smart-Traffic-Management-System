Project Proposal: Smart Traffic Management System
Group No: 04
Name: Neshadi Hirunika R.K, Rajakaruna R.H.M.K.M, Randima H.G.L
Registration Number: EG/2020/4093, EG/2020/4128, EG/2020/4148
1. Introduction
The aim of this project is to simulate a Smart Traffic Management System where multiple intersections, vehicles, and traffic signals operate concurrently in real time. Urban traffic networks involve large-scale data processing and rapid decision-making, making them suitable for highperformance computing applications.
To improve performance and scalability, the simulation will utilize parallel programming models including OpenMP, POSIX Threads (Pthreads), MPI, and CUDA, along with a hybrid parallel approach.
2. Methodology

Shared Memory Parallelism – OpenMP

OpenMP will be used to implement shared memory parallelism on multi core systems. Traffic operations such as vehicle updates, congestion evaluation, and signal control at different intersections will be executed concurrently using multiple threads. Since all threads share the same memory space, the system can efficiently utilize CPU cores and reduce execution time.

Distributed Memory Parallelism – MPI

MPI will be used to distribute different regions of the traffic network across multiple computing nodes in a distributed environment. Each MPI process will handle a subset of intersections or road segments independently. Communication between processes will be performed to exchange vehicle counts, congestion levels, and signal timing information, ensuring a consistent and synchronized global traffic state.

Thread-Level Parallelism – POSIX Threads

POSIX Threads will be used to create and manage threads manually within a process for handling intersections or groups of vehicles concurrently. Each thread will execute traffic-related tasks independently while sharing common traffic data structures. Synchronization mechanisms such as mutexes and locks will be applied to prevent race conditions and maintain data consistency.

GPU Acceleration – CUDA

CUDA will be utilized to offload computationally intensive operations, such as large-scale vehicle updates and congestion calculations, to the GPU. Thousands of GPU threads can execute these operations simultaneously, significantly increasing processing speed. This allows the simulation to handle large traffic networks more efficiently compared to CPU-only execution.

Hybrid Approach (MPI + OpenMP)

The hybrid approach combines MPI (distributed memory) and OpenMP (shared memory) to improve performance of the traffic simulation. The traffic network is divided among multiple MPI processes, where each process handles a subset of intersections independently. This enables execution across multiple nodes.

Within each MPI process, OpenMP is used to parallelize intersection-level operations such as vehicle updates, signal changes, and congestion calculation using multiple CPU threads. MPI handles communication between processes to exchange boundary traffic data and maintain global consistency.

This approach reduces execution time by exploiting both inter-node and intra-node parallelism, making it suitable for large-scale traffic simulations.
Input and Output.
Inputs: Traffic signals, Vehicle demand, Network
Outputs: performance metrics (waiting time, travel time, queue length, throughput), signal behavior data,