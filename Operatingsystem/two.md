# Program, Process, and Thread

---

## 1. Core Definitions

### A. Program (Passive Entity)
* A **Program** is a static file containing machine instructions stored on non-volatile secondary storage (SSD/HDD), such as `notepad.exe` or `a.out`.
* It does not consume RAM or CPU cycles until it is loaded and invoked.

### B. Process (Active Entity)
* A **Process** is a program in active execution loaded into main memory (RAM).
* It represents an independent execution environment with its own private address space, OS credentials, and file descriptors.
* Two running instances of Google Chrome represent two distinct processes, each with separate memory allocations.

### C. Thread (Lightweight Unit of Execution)
* A **Thread** is the smallest schedulable sequence of instructions inside a process.
* Multiple threads belong to the same parent process and execute concurrently while sharing the process's address space.

---

## 2. Memory Anatomy of a Process

When an executable program is launched, the operating system creates a virtual address space partitioned into distinct segments:

```
+-----------------------------------+ High Memory (0xFFFFFFFF)
|               Stack               |  <-- Function calls, local variables (grows downward)
|                 |                 |
|                 v                 |
|                                   |
|                 ^                 |
|                 |                 |
|               Heap                |  <-- Dynamic allocations: malloc / new (grows upward)
+-----------------------------------+
|          BSS Segment              |  <-- Uninitialized global & static variables
+-----------------------------------+
|          Data Segment             |  <-- Initialized global & static variables
+-----------------------------------+
|          Text Segment             |  <-- Compiled binary machine instructions (read-only)
+-----------------------------------+ Low Memory (0x00000000)
```

1. **Text (Code) Segment**: Contains the compiled machine instructions. Marked as read-only to prevent self-modifying code.
2. **Data Segment**: Holds initialized global and static variables (e.g., `int count = 10;`).
3. **BSS Segment**: Holds uninitialized global and static variables (initialized to zero by default).
4. **Heap**: Memory dynamically allocated at runtime via `malloc()`, `calloc()`, or `new`.
5. **Stack**: Stores activation frames for active functions, including parameters, return addresses, and local variables.

---

## 3. What Threads Share vs. What is Private

Inside a multithreaded process, threads share common resources while maintaining their individual execution state:

```
               [ Single Process Memory Space ]
+--------------------------------------------------------------+
| Shared:  Text (Code) | Data (Globals) | Heap (Dynamic Mem)   |
| Shared:  Open File Descriptors | Sockets | Environment Vars  |
+--------------------------------------------------------------+
| Thread 1 Private:   | Thread 2 Private:   | Thread 3 Private:   |
| - Program Counter   | - Program Counter   | - Program Counter   |
| - CPU Registers     | - CPU Registers     | - CPU Registers     |
| - Private Stack     | - Private Stack     | - Private Stack     |
| - Thread ID (TID)   | - Thread ID (TID)   | - Thread ID (TID)   |
+--------------------------------------------------------------+
```

| Shared Across All Threads in a Process | Private to Each Individual Thread |
|---|---|
| **Text Segment** (code instructions) | **Program Counter (PC)** (next instruction to fetch) |
| **Data Segment** (global variables) | **CPU Registers** (temporary calculation values) |
| **Heap** (dynamically allocated memory) | **Thread Stack** (local variables, call frames) |
| **Open Files & Sockets** | **Thread ID (TID)** & scheduling priority |
| **Signal Handlers** | **Thread-Local Storage (TLS)** (if explicitly defined) |

---

## 4. Multitasking vs. Multithreading

| Feature | Multitasking | Multithreading |
|---|---|---|
| **Scope** | Manages multiple independent **processes**. | Manages multiple **threads** within one process. |
| **Memory Isolation** | Each process has its own protected memory space. | All threads share the same process memory space. |
| **Communication** | Requires **Inter-Process Communication (IPC)**: pipes, shared memory, sockets. | Direct communication via shared global memory / heap pointers. |
| **Context Switch Cost** | High (involves flushing TLB, switching page tables). | Low (retains page tables and CPU cache mappings). |
| **Fault Tolerance** | High: one process crash does not affect other processes. | Low: an unhandled memory fault in one thread can terminate the whole process. |
| **Real Example** | Running a web browser, code editor, and media player simultaneously. | In a word processor: one thread handles UI typing, another checks spellings, another auto-saves. |

---

## 5. Concurrency vs. Parallelism

* **Concurrency**: Managing multiple computations at the same time by interleaving execution. This works on a single-core CPU via rapid context switching.
  * *Focus*: Structuring a system into independently executing tasks.
* **Parallelism**: Executing multiple computations at the exact same physical moment across multiple CPU cores or processors.
  * *Focus*: High-speed throughput and computation distribution.

```
Concurrency (Single Core):
Core 1: [ Task A ] -> [ Task B ] -> [ Task A ] -> [ Task B ] (Interleaved)

Parallelism (Multi-Core):
Core 1: [ Task A -----------------------------------------> ]
Core 2: [ Task B -----------------------------------------> ] (Simultaneous)
```

---

## 6. Key Advantages & Trade-Offs of Threads

### Advantages:
1. **Responsiveness**: Interactive UIs do not freeze when performing long calculations or network fetches in background threads.
2. **Resource Efficiency**: Sharing memory avoids duplicating heavy process footprints.
3. **High Throughput on Multi-Core Hardware**: True parallel execution of computational tasks.
4. **Economical Context Switching**: Significantly lower CPU overhead during thread scheduling.

### Trade-Offs & Pitfalls:
1. **Race Conditions**: Unsynchronized concurrent access to shared memory leads to unpredictable state corruption.
2. **Deadlocks**: Competing locks can permanently freeze execution.
3. **Debugging Complexity**: Non-deterministic execution order makes reproducing race-related bugs difficult.