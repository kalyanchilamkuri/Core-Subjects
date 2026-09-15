# Advanced CPU Scheduling Algorithms: SJF, SRTF, Priority, Round Robin, MLQ, and MLFQ

---

## 1. Shortest Job First (SJF) - Non-Preemptive

### Principle:
* The process with the **smallest CPU Burst Time (BT)** is selected and scheduled next.
* If two processes have equal burst times, FCFS is used as a tie-breaker.
* **Non-Preemptive**: Once a process is allocated the CPU, it runs to completion without interruption.

### Key Advantages:
* **Mathematically Proven Optimal**: SJF guarantees the **minimum average waiting time** for any given set of stationary processes.

### Key Disadvantages:
* **Cannot Predict Future Burst Times**: In general-purpose OSes, the kernel cannot know beforehand how long a user process will run.
  * *Approximation*: The OS estimates future burst times using **Exponential Smoothing**:
    $$\tau_{n+1} = \alpha t_n + (1 - \alpha)\tau_n$$
    *(where $t_n$ is the actual recent burst time, $\tau_n$ is the predicted history, and $0 \le \alpha \le 1$)*.
* **Starvation**: If short jobs continuously arrive, long jobs may wait indefinitely and starve.

---

## 2. Shortest Remaining Time First (SRTF) - Preemptive SJF

### Principle:
* Preemptive variation of SJF.
* Whenever a new process arrives in the Ready Queue, the CPU scheduler compares the **remaining burst time** of the currently running process with the burst time of the newly arrived process.
* If the new process requires less CPU time than the current process has remaining, the running process is **preempted** and returned to the ready queue.

### Example Tracing:

| Process | Arrival Time (AT) | Burst Time (BT) |
|:---:|:---:|:---:|
| **P1** | 0 | 8 |
| **P2** | 1 | 4 |
| **P3** | 2 | 9 |
| **P4** | 3 | 5 |

#### Execution Trace & Gantt Chart:
* At $t = 0$: Only P1 is present. P1 runs.
* At $t = 1$: P2 arrives ($BT = 4$). P1 has 7 units remaining ($7 > 4$). **P1 is preempted**, P2 runs!
* At $t = 2$: P3 arrives ($BT = 9$). P2 has 3 units left ($3 < 9$). P2 continues.
* At $t = 3$: P4 arrives ($BT = 5$). P2 has 2 units left ($2 < 5$). P2 continues until completion at $t = 5$.
* At $t = 5$: Remaining jobs are P1 (7), P4 (5), P3 (9). P4 has the shortest remaining time $\rightarrow$ P4 runs until $t = 10$.
* At $t = 10$: P1 (7) runs until $t = 17$.
* At $t = 17$: P3 (9) runs until $t = 26$.

```
Gantt Chart:
+---+---------------+---------------+-----------------------+-------------------+
|P1 |      P2       |      P4       |          P1           |        P3         |
+---+---------------+---------------+-----------------------+-------------------+
0   1               5               10                      17                  26
```

---

## 3. Priority Scheduling

### Principle:
* A priority integer is associated with each process. The CPU is allocated to the process with the **highest priority** (standard convention in OS design: **lower number = higher priority**).
* Can be implemented as either **Preemptive** or **Non-Preemptive**.

### The Starvation Problem & The "Aging" Solution:
* **Indefinite Blocking (Starvation)**: Low-priority processes may wait in the ready queue forever if a constant stream of higher-priority processes keeps arriving.
  * *(Historical Fact: When MIT's IBM 7094 mainframe was shut down in 1973, they discovered a low-priority process that had been waiting in the queue since 1967!)*
* **Solution: Aging**:
  * The operating system gradually increments the priority of waiting processes at fixed time intervals (e.g., increase priority by 1 every 60 seconds).
  * Eventually, any long-waiting process ascends to the highest priority level and executes.

---

## 4. Round Robin (RR) Scheduling

### Principle:
* Specifically designed for interactive **time-sharing systems**.
* The scheduler treats the Ready Queue as a circular FIFO queue.
* Each process receives a small, fixed unit of CPU time called a **Time Quantum (TQ)** (typically 10 to 50 ms).
* Once the quantum expires, the running process is **preempted** and placed at the tail of the Ready Queue.

```
       [ Ready Queue (Circular FIFO) ]
           ┌─────────────────────┐
           ▼                     │
    [ P1 ] ──► [ P2 ] ──► [ P3 ] ┘
                 │
                 ▼ [Runs for Time Quantum]
               [ CPU ]
```

### The Impact of Time Quantum Selection:

```
If Time Quantum is Huge (TQ -> ∞):
- Becomes identical to FCFS.
- Poor response time for interactive programs.

If Time Quantum is Extremely Tiny (TQ -> 0):
- Known as "Processor Sharing".
- CPU spends more time switching contexts than doing actual work!
```

* **Best Practice**: Set the Time Quantum such that roughly **80% of CPU bursts are shorter than the quantum**, minimizing context-switch overhead while maintaining snappy responsiveness.

---

## 5. Multilevel Queue (MLQ) Scheduling

### Principle:
* The Ready Queue is partitioned into multiple separate queues based on process characteristics:

```
[ Queue 1: System Processes ]      ──► (Highest Priority, e.g., RR with TQ=5ms)
[ Queue 2: Interactive Processes ] ──► (Medium Priority, e.g., RR with TQ=15ms)
[ Queue 3: Batch Processes ]       ──► (Lowest Priority, e.g., FCFS)
```

* **Fixed Priority Preemption**: No process in Queue 2 can run unless Queue 1 is completely empty.
* **Limitation**: **Rigid & Inflexible**. A process is permanently assigned to one queue at creation time. If higher-priority queues remain active, lower queues starve completely.

---

## 6. Multilevel Feedback Queue (MLFQ) Scheduling

### Principle:
* An intelligent, adaptive evolution of MLQ where **processes can dynamically move between queues** based on their observed runtime behavior:

```
Queue 0 (TQ = 8 ms)   ──► [ New Process Enters ]
                            │ (If exceeds 8 ms without I/O)
                            ▼
Queue 1 (TQ = 16 ms)  ──► [ CPU-Bound Process Demoted ]
                            │ (If exceeds 16 ms without I/O)
                            ▼
Queue 2 (FCFS)        ──► [ Long Background Batch Job ]
                            │
                            ▲ (Aging: If process waits too long, promote UP!)
```

### How MLFQ Balances the System:
1. **Favors Interactive I/O-Bound Jobs**: Interactive jobs yield the CPU quickly before their quantum expires, remaining in top-priority queues with rapid response times.
2. **Penalizes Long CPU-Bound Jobs**: Computation-heavy jobs consume their entire time quantum and are progressively demoted to lower queues with larger time slices.
3. **Prevents Starvation with Aging**: Processes that spend excessive time waiting in bottom queues are automatically promoted to higher queues.
4. **Real-World Impact**: MLFQ forms the architectural foundation for modern CPU schedulers in Unix, Linux, and Windows NT.

---

## 7. Comprehensive Summary & Comparison Matrix

| Algorithm | Preemptive? | Optimal WT? | Starvation Risk? | Convoy Effect? | Primary Use Case |
|---|:---:|:---:|:---:|:---:|---|
| **FCFS** | No | No | No | **Yes** | Batch workloads |
| **SJF (Non-Preemptive)** | No | Yes (Non-preemptive) | Yes | No | Theoretical benchmark |
| **SRTF (Preemptive SJF)**| Yes | **Yes (Absolute Optimal)**| Yes | No | Short-burst optimization |
| **Priority Scheduling** | Both | No | Yes (solved by Aging)| No | Urgent real-time tasks |
| **Round Robin (RR)** | Yes | No | **No** | No | Desktop time-sharing |
| **Multilevel Queue** | Yes | No | Yes | Yes (in low queue)| Static separation |
| **Multilevel Feedback** | Yes | Approximates Optimal | **No (via Aging)** | No | **Modern General-Purpose OS** |
