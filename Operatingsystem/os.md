# Operating Systems: Complete Master Course & Revision Notes

---

## Table of Contents
1. [Module 1: Introduction & Evolution of Operating Systems](#module-1-introduction--evolution-of-operating-systems)
2. [Module 2: System Architecture, Dual-Mode & System Calls](#module-2-system-architecture-dual-mode--system-calls)
3. [Module 3: Hardware Architecture & Memory Hierarchy](#module-3-hardware-architecture--memory-hierarchy)
4. [Module 4: Process Management & Lifecycle](#module-4-process-management--lifecycle)
5. [Module 5: CPU Scheduling Algorithms](#module-5-cpu-scheduling-algorithms)
6. [Module 6: Threads, Concurrency & Synchronization](#module-6-threads-concurrency--synchronization)
7. [Module 7: Classical Synchronization Problems](#module-7-classical-synchronization-problems)
8. [Module 8: Deadlocks & Handling Strategies](#module-8-deadlocks--handling-strategies)
9. [Module 9: Memory Management & Virtual Memory](#module-9-memory-management--virtual-memory)

---

## Module 1: Introduction & Evolution of Operating Systems

### 1. What is an Operating System?
An **Operating System (OS)** is system software that manages computer hardware resources, provides an abstraction layer over low-level devices, and provides common services for application software.
* **Without an OS**: Every application would need custom code to talk directly to memory chips, disk controllers, and screen pixels. One buggy application could read or overwrite private data belonging to another program or monopolize the CPU permanently.
* **Key Roles**:
  * **Resource Allocator**: Decides how CPU time, memory space, file storage, and I/O devices are allocated among competing processes.
  * **Control Program**: Governs execution of user applications to prevent errors and improper use of the system.
  * **Hardware Abstraction**: Presents raw hardware (registers, interrupts, disk tracks) as convenient abstractions (files, directories, sockets, windows).

### 2. Evolution and Types of Operating Systems
* **Single-Process OS**: Only one program resides in memory and runs at a time (e.g., early MS-DOS). If a program crashes, the entire machine freezes.
* **Batch Operating System**: Gathers similar jobs into batches and executes them sequentially without user interaction (e.g., payroll processing). Uses **Spooling** (Simultaneous Peripheral Operations On-Line) to buffer jobs between disks and slower devices.
* **Multiprogramming OS**: Holds multiple programs in RAM simultaneously. When the active job waits for I/O, the CPU switches to another job to maximize CPU utilization.
* **Multitasking (Time-Sharing) OS**: Logical extension of multiprogramming. Allocates tiny time slices (quantum) to each active program in round-robin fashion, creating the illusion that multiple programs run simultaneously.
* **Multiprocessing OS**: Employs two or more physical CPUs or cores sharing a memory bus (Symmetric Multiprocessing - SMP), achieving true parallel execution.
* **Distributed OS**: Coordinates a cluster of independent, networked computers to appear to users as a single unified computing system.
* **Real-Time Operating System (RTOS)**: Guarantees deterministic response within strict deadlines:
  * **Hard RTOS**: Missing a deadline causes total system catastrophe (e.g., car airbags, medical pacemakers).
  * **Soft RTOS**: Missing a deadline degrades quality of service but does not break system safety (e.g., video streaming).

---

## Module 2: System Architecture, Dual-Mode & System Calls

### 1. Dual-Mode Operation (Hardware Protection)
Modern CPUs provide hardware support to isolate operating system operations from user code:
* **User Mode (Mode Bit = 1)**: Applications run with restricted privileges. Privileged instructions (like direct port I/O, modifying page tables, or clearing interrupt flags) are hardware-disabled.
* **Kernel Mode (Mode Bit = 0, Supervisor Mode)**: The OS kernel runs with unrestricted privileges, executing all machine instructions and accessing physical devices.

### 2. System Calls (The User $\leftrightarrow$ Kernel Gateway)
A **System Call** is the programmatic mechanism an application uses to request services from the OS kernel:
1. User application prepares parameters and executes a trap instruction (`syscall` / `int 0x80`).
2. Hardware switches the Mode Bit from `1` to `0` and transfers execution to the **System Call Handler** in the kernel.
3. The kernel validates parameters, executes the requested service, and restores user registers.
4. Hardware flips the Mode Bit back to `1` and resumes user code.

* **Key Categories**:
  * *Process Control*: `fork()`, `exec()`, `wait()`, `exit()`
  * *File Management*: `open()`, `read()`, `write()`, `close()`
  * *Device Management*: `ioctl()`, `read()`, `write()`
  * *Communication*: `pipe()`, `shmget()`, `socket()`

### 3. Kernel Architectures
* **Monolithic Kernel (e.g., Linux, Unix)**: All core OS services (VFS, scheduler, drivers, memory management) run together inside kernel space. Fast, but a bug in a driver can crash the entire system.
* **Microkernel (e.g., QNX, Minix)**: Only absolute essentials (IPC, basic scheduling, virtual memory paging) run in kernel space; drivers and file systems run as user-space servers. Highly resilient, but slightly slower due to IPC message overhead.
* **Hybrid Kernel (e.g., Windows NT, macOS XNU)**: Combines microkernel modularity with monolithic execution speed.

### 4. The Booting Process
$$\text{Power On} \longrightarrow \text{CPU Reset Vector} \longrightarrow \text{BIOS/UEFI} \longrightarrow \text{POST} \longrightarrow \text{Bootloader (MBR/GPT)} \longrightarrow \text{Kernel Loaded} \longrightarrow \text{PID 1 (systemd)} \longrightarrow \text{Login}$$

---

## Module 3: Hardware Architecture & Memory Hierarchy

### 1. 32-Bit vs. 64-Bit Architecture
* **32-Bit System**: 32-bit registers, 32-bit memory pointers $\implies 2^{32} = 4\text{ GB}$ maximum addressable RAM.
* **64-Bit System**: 64-bit registers, 64-bit memory pointers $\implies 2^{64} \approx 16\text{ Exabytes}$ addressable RAM.
* **Performance**: 64-bit architectures double general-purpose registers (from 8 to 16 in x86-64), process 8-byte integers in a single instruction cycle, and offer hardware-enforced security features (NX bit, ASLR).

### 2. The Memory Hierarchy Pyramid
```
[ CPU Registers ]        <-- < 1 ns, Bytes, Ultra-Fast
  [ L1/L2/L3 Cache ]     <-- 1-15 ns, Megabytes, SRAM
    [ Main RAM ]         <-- 60-100 ns, Gigabytes, DRAM (Volatile)
      [ NVMe/SATA SSD ]  <-- 10-50 microseconds, Terabytes, Flash (Non-Volatile)
        [ HDD / Tapes ]  <-- 5-15 milliseconds, Terabytes/Petabytes
```
* **Why it works**: **Locality of Reference**:
  * *Temporal Locality*: Data accessed recently is likely to be accessed again soon (loops, functions).
  * *Spatial Locality*: Data near recently accessed memory is likely to be accessed soon (arrays, sequential instructions).

---

## Module 4: Process Management & Lifecycle

### 1. Process Memory Layout
```
+-----------------------------------+ High Address
| Stack (Local vars, Call frames)   | (Grows downwards)
|               |                   |
|               v                   |
|               ^                   |
|               |                   |
| Heap (Dynamic memory: malloc/new) | (Grows upwards)
+-----------------------------------+
| BSS (Uninitialized global vars)   |
+-----------------------------------+
| Data (Initialized global vars)    |
+-----------------------------------+
| Text (Compiled machine code)      | (Read-Only)
+-----------------------------------+ Low Address
```

### 2. Process Control Block (PCB)
The OS data structure that records everything about a process: PID, PPID, Process State, Program Counter, CPU Registers, Memory Limits (Page Tables), Scheduling Priority, and Open Files.

### 3. Process States & Transitions
* **New**: Process is being created.
* **Ready**: Resides in RAM, waiting for CPU allocation.
* **Running**: Instructions executing on CPU.
* **Waiting (Blocked)**: Waiting for an external I/O or synchronization event.
* **Terminated**: Finished execution; PCB retained until exit status is reaped.

### 4. Schedulers
* **Long-Term Scheduler (LTS / Job Scheduler)**: Selects jobs from disk to load into RAM; controls the Degree of Multiprogramming.
* **Short-Term Scheduler (STS / CPU Scheduler)**: Selects an active process from the Ready Queue to execute on the CPU.
* **Medium-Term Scheduler (MTS / Swapper)**: Swaps processes out to disk to alleviate RAM saturation.

### 5. Orphan vs. Zombie Processes
* **Orphan Process**: Parent process terminates while the child is still executing. The child is adopted by `systemd` (PID 1), which reaps it cleanly upon termination.
* **Zombie Process**: Child process has terminated (`exit()`), but its entry remains in the Process Table because its parent has not yet collected its exit code using `wait()`. Excessive zombies cause **PID exhaustion**.

---

## Module 5: CPU Scheduling Algorithms

### 1. Scheduling Criteria
* **Arrival Time (AT)**: When the process enters the Ready Queue.
* **Burst Time (BT)**: CPU execution time required.
* **Completion Time (CT)**: When execution finishes.
* **Turnaround Time (TAT)**: $\text{TAT} = \text{CT} - \text{AT}$.
* **Waiting Time (WT)**: $\text{WT} = \text{TAT} - \text{BT}$.
* **Response Time (RT)**: Time from arrival to first CPU allocation.

### 2. Core Scheduling Algorithms
* **FCFS (First-Come, First-Served)**: Non-preemptive. Simple, but vulnerable to the **Convoy Effect** where long jobs block short jobs.
* **SJF (Shortest Job First)**: Non-preemptive. Optimal for minimum average waiting time, but future burst times must be estimated and long jobs can starve.
* **SRTF (Shortest Remaining Time First)**: Preemptive SJF. Preempts running process if a newly arrived job has shorter remaining burst time.
* **Priority Scheduling**: Allocates CPU based on priority values. Vulnerable to starvation, which is solved by **Aging** (incrementing priority of waiting jobs over time).
* **Round Robin (RR)**: Preemptive time-sharing using a fixed **Time Quantum (TQ)**. Ensures fairness and rapid response.
* **Multilevel Feedback Queue (MLFQ)**: Uses multiple queues with differing priorities and time quanta. Processes dynamically move between queues based on observed CPU/IO behavior.

---

## Module 6: Threads, Concurrency & Synchronization

### 1. Threads vs. Processes
* Threads share: Code segment, Data segment, Heap, and Open Files.
* Each thread retains private: Thread ID, Program Counter, CPU Registers, and Call Stack.
* **Thread Context Switch**: Extremely fast; avoids flushing TLB and switching page tables.

### 2. The Critical Section Problem
A segment of code where shared resources are accessed. Any correct synchronization solution must satisfy three mandatory conditions:
1. **Mutual Exclusion**: Only one process/thread inside the critical section at a time.
2. **Progress**: Selection of who enters next cannot be postponed indefinitely.
3. **Bounded Waiting**: There is a finite bound on how many times others can enter before a waiting process is admitted (no starvation).

### 3. Peterson's Algorithm (2 Processes)
```c
flag[0] = true;
turn = 1;
while (flag[1] && turn == 1); // Busy-wait
// --- CRITICAL SECTION ---
flag[0] = false;
```
Guarantees Mutual Exclusion, Progress, and Bounded Waiting for two processes.

### 4. Mutex vs. Semaphore
* **Mutex**: Mutual exclusion lock with strict ownership. Only the thread that acquired the lock can release it.
* **Counting Semaphore**: Integer counter initialized to $N$ with atomic operations:
  * `wait(S)` / $P(S)$: Decrements $S$; blocks if $S \le 0$.
  * `signal(S)` / $V(S)$: Increments $S$; wakes up one waiting thread.

---

## Module 7: Classical Synchronization Problems

### 1. Bounded Buffer (Producer-Consumer)
* Buffer size = $N$.
* Semaphores: `mutex = 1`, `empty = N`, `full = 0`.
* **Important**: Producer must execute `wait(empty)` before `wait(mutex)` to prevent deadlock!

### 2. Readers-Writers Problem
* Multiple readers can read simultaneously; writers require exclusive access.
* **Reader Priority**: First reader locks resource, last reader unlocks. Can cause **Writer Starvation**.
* **Writer Priority**: Once a writer is queued, incoming readers are blocked to prevent writer starvation.

### 3. Dining Philosophers Problem
* 5 philosophers, 5 forks, each needs 2 forks to eat.
* If all pick up left fork simultaneously $\implies$ **Deadlock by Circular Wait**.
* **Solutions**: Pick up forks atomically; allow at most 4 philosophers at the table; or have odd philosophers pick left first and even pick right first.

---

## Module 8: Deadlocks & Handling Strategies

### 1. The 4 Coffman Conditions
Deadlock occurs if and only if all four conditions hold simultaneously:
1. **Mutual Exclusion**: Non-shareable resource.
2. **Hold and Wait**: Holding at least one resource while waiting for another.
3. **No Preemption**: Resources cannot be forcibly taken away.
4. **Circular Wait**: Closed loop of dependencies among processes.

### 2. Deadlock Handling Approaches
1. **Deadlock Prevention**: Invalidate at least one Coffman condition (e.g., impose strict global resource ordering to break circular wait).
2. **Deadlock Avoidance**: Grant resource requests only if the resulting state is safe (**Banker's Algorithm**).
3. **Deadlock Detection & Recovery**: Run cycle detection algorithms on Wait-For Graphs and recover by process termination or resource preemption.
4. **Ostrich Algorithm**: Ignore the problem; standard for general-purpose OSes.

---

## Module 9: Memory Management & Virtual Memory

### 1. Logical vs. Physical Address Space
* **Logical Address**: Generated by CPU during execution.
* **Physical Address**: Actual location in physical RAM.
* **Memory Management Unit (MMU)**: Hardware that maps logical addresses to physical addresses at runtime using base and limit registers.

### 2. Paging & Address Translation
* Logical memory is divided into fixed-size blocks called **Pages**.
* Physical RAM is divided into blocks of identical size called **Frames**.
* **Address Breakdown**: $\text{Logical Address} = (\text{Page Number } p, \text{ Offset } d)$.
* Completely eliminates **External Fragmentation**, though slight **Internal Fragmentation** remains on the final page.
* **Page Table**: Maps page numbers $p$ to frame numbers $f$.
* **TLB (Translation Lookaside Buffer)**: Ultra-fast CPU hardware cache for page table translations.

### 3. Page Faults & Virtual Memory
* **Virtual Memory**: Gives applications the illusion of a massive contiguous address space, allowing programs larger than physical RAM to execute.
* **Page Fault**: Hardware interrupt raised when a program accesses a virtual page that is not currently mapped into physical RAM (valid bit = 0).
* **Handling Steps**: Trap to kernel $\rightarrow$ allocate free frame $\rightarrow$ fetch page from swap space on disk $\rightarrow$ update page table valid bit $\rightarrow$ restart instruction.

### 4. Page Replacement Algorithms
* **FIFO (First-In, First-Out)**: Replaces oldest loaded page. Suffers from **Belady's Anomaly** (increasing frame count can increase page faults).
* **LRU (Least Recently Used)**: Replaces the page not accessed for the longest time. Stack algorithm; optimal practical performance.
* **Optimal Algorithm**: Replaces page that will not be accessed for the longest time in future. Theoretical benchmark.
* **Thrashing**: Occurs when a system spends more time servicing page faults and swapping than executing code. Resolved by the **Working Set Model** and controlling the degree of multiprogramming.
