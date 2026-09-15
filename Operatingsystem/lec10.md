# Process Management: States, PCB, Schedulers, and Context Switching

---

## 1. What is a Process?

A **Process** is a program in active execution.
* **From Source Code to Process**:
  ```
  Source Code (.cpp) ──[Compiler/Linker]──► Executable File (on disk) ──[OS Loader]──► Process (in RAM)
  ```
* When an executable is invoked, the OS creates an execution context:
  1. Loads compiled machine instructions and static global data into memory.
  2. Allocates the **Call Stack** (for local variables, function arguments, and return addresses).
  3. Allocates the **Heap** (for runtime dynamic memory allocations).
  4. Sets up file descriptors for standard input, output, and error (`stdin`, `stdout`, `stderr`).
  5. Initializes CPU registers, including pointing the **Program Counter (PC)** to the entry point `main()`.

---

## 2. Process Control Block (PCB)

The **Process Control Block (PCB)** (also known as the Task Control Block) is the core OS data structure that stores all information needed to manage, schedule, and pause/resume a specific process.

```
+--------------------------------------------------+
|              Process Control Block (PCB)         |
+--------------------------------------------------+
| Process ID (PID) & Parent Process ID (PPID)      |
| Current State (New, Ready, Running, Waiting, ...) |
| Program Counter (Address of next instruction)     |
| CPU Registers (Accumulators, Index, Stack Pointer)|
| CPU Scheduling Info (Priority, Queue Pointers)   |
| Memory Management Info (Page Tables, Base/Limit) |
| Accounting Info (CPU time used, process limits)  |
| I/O Status Info (List of open files, I/O devices)|
+--------------------------------------------------+
```

When a process is paused during multitasking, the OS snapshots its exact CPU registers into its PCB so it can resume later without any loss of state.

---

## 3. Process Life Cycle: States and Transitions

Modern operating systems transition processes through a multi-state lifecycle:

```
                  [Admit]
      (New) ──────────────────► [ Ready Queue ]
                                     ▲     │
                     [Preemption /   │     │ [Scheduler
                     Quantum Expire] │     │  Dispatch]
                                     │     ▼
                                  (Running) ──[Exit]──► (Terminated)
                                     │
                                     │ [I/O or Event Wait]
                                     ▼
                                 (Waiting / Blocked)
                                     │
                                     │ [I/O Complete]
                                     └─────► (Ready)
```

### The 5 Standard States:
1. **New**: The process is being created and its PCB is being initialized.
2. **Ready**: The process is in main memory (RAM) waiting to be allocated CPU time by the scheduler.
3. **Running**: The process's instructions are currently executing on the physical CPU.
4. **Waiting (Blocked)**: The process cannot execute because it is waiting for an external event (e.g., keyboard input, disk read, or network packet).
5. **Terminated**: The process has finished execution and is waiting for its parent to reap its exit status.

### The 7-State Model (Swapped States):
When RAM is fully saturated, the OS temporarily swaps processes out to secondary storage (swap space):
* **Suspended Ready**: The process was ready to run, but was swapped out to disk to free RAM.
* **Suspended Blocked**: The process is waiting for an I/O event and has also been swapped out to disk.

---

## 4. Scheduling Queues

The operating system organizes processes in specialized linked lists called queues:
1. **Job Queue**: Contains all newly submitted processes residing in secondary memory before admission to RAM.
2. **Ready Queue**: Contains processes residing in RAM that are ready and waiting to receive CPU execution time.
3. **Device / Waiting Queue**: Contains processes blocked waiting for a specific hardware device (e.g., waiting for the disk controller to read a block).

---

## 5. Types of Operating System Schedulers

```
   Secondary Storage (Disk)               Main Memory (RAM)             CPU
+----------------------------+       +-------------------------+     +-------+
|         Job Queue          | =====>|       Ready Queue       | ===>| Running
+----------------------------+  LTS  +-------------------------+ STS +-------+
                                           ▲             │
                                           └==== MTS ====┘
                                            (Swap in/out)
```

| Scheduler | Common Name | Frequency of Execution | Primary Role |
|---|---|---|---|
| **Long-Term Scheduler (LTS)** | Job Scheduler | Infrequent (seconds/minutes) | Selects processes from disk and loads them into RAM. Controls the **Degree of Multiprogramming** (number of processes in memory). |
| **Short-Term Scheduler (STS)** | CPU Scheduler | Extremely frequent (every 10–100 ms) | Selects a process from the Ready Queue and allocates the CPU to it. Must be ultra-fast to minimize overhead. |
| **Medium-Term Scheduler (MTS)**| Swapper | Intermediate | Temporarily swaps inactive/overloaded processes out of RAM to disk (and back) to relieve memory pressure. |

---

## 6. The Dispatcher and Dispatcher Latency

* **Scheduler vs. Dispatcher**:
  * The **Scheduler** merely selects *which* process should run next from the ready queue.
  * The **Dispatcher** is the OS module that *actually gives control of the CPU* to the selected process.
* **Dispatcher Responsibilities**:
  1. Switching execution context (saving old registers, restoring new ones).
  2. Switching from kernel mode to user mode.
  3. Jumping to the exact instruction in the user program indicated by its Program Counter.
* **Dispatcher Latency**: The exact time interval required to stop one process and start another. Dispatcher latency is pure overhead and must be minimized.

---

## 7. Context Switching Step-by-Step

A **Context Switch** occurs when the CPU halts execution of one process and begins executing another:

```
Process P0                       OS Kernel                       Process P1
----------                       ---------                       ----------
Executing ...
Interrupt or Syscall  ----->
                             Save P0 state into PCB0
                             Update scheduling queues
                             Select P1 from Ready Queue
                             Load P1 state from PCB1
                      <----- Resume Execution
                                                                 Executing ...
```

1. An interrupt occurs (timer quantum expires) or a system call is made.
2. The kernel saves the current state (Program Counter, registers, stack pointer) of process $P_0$ into its PCB.
3. The kernel moves $P_0$ to the appropriate queue (Ready or Waiting).
4. The scheduler picks process $P_1$.
5. The kernel restores $P_1$'s context from its PCB into the CPU registers.
6. The CPU jumps to $P_1$'s restored Program Counter and resumes execution.

---

## 8. Inter-Process Communication (IPC)

Processes can be **Independent** (do not share data) or **Cooperative** (share data and state). Cooperative processes require **Inter-Process Communication (IPC)**:

```
       [ Shared Memory Model ]                  [ Message Passing Model ]
Process A                  Process B         Process A                  Process B
    \                          /                 │                          ▲
     \                        /                  │  send(msg)   receive(msg)│
      ▼                      ▼                   ▼                          │
    [ Common Shared Memory Region ]          [ OS Kernel Message Queue / Buffer ]
```

1. **Shared Memory**:
   * Processes map a common physical memory region into their virtual address spaces.
   * **Advantage**: Very fast (data is read/written at memory speed without copying through the kernel).
   * **Challenge**: Processes must synchronize access themselves (using semaphores/mutexes) to prevent race conditions.
2. **Message Passing**:
   * Processes communicate by sending and receiving structured messages through the OS kernel using system calls (`send()`, `recv()`).
   * **Advantage**: Safe and easy to implement across different machines in a distributed network.
   * **Disadvantage**: Slower due to kernel context switches and memory copying.
