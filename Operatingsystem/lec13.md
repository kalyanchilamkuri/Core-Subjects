# CPU Scheduling Fundamentals, Scheduling Metrics, and FCFS

**CPU Scheduling** is the process by which the operating system decides which runnable process in the Ready Queue is allocated the CPU when the processor becomes idle.

---

## 1. The CPU-I/O Burst Cycle

Process execution consists of a recurring cycle of **CPU execution (computation)** and **I/O wait (disk, network, user input)**:

```
[ CPU Burst ] ──► [ I/O Burst ] ──► [ CPU Burst ] ──► [ I/O Burst ] ──► Terminated
```
* **CPU-Bound Process**: Spends the vast majority of its execution time performing mathematical calculations (long CPU bursts, infrequent I/O bursts).
* **I/O-Bound Process**: Spends the vast majority of its time waiting for I/O operations (very short CPU bursts, frequent I/O bursts).

---

## 2. Preemptive vs. Non-Preemptive Scheduling

```
           Non-Preemptive                                   Preemptive
+-----------------------------------+           +-----------------------------------+
| Process gets CPU -> Runs until it |           | Process gets CPU -> OS can take it|
| terminates or voluntarily does I/O|           | back if time slice expires or     |
| (Cannot be interrupted by OS)     |           | higher-priority process arrives!  |
+-----------------------------------+           +-----------------------------------+
```

| Criteria | Non-Preemptive Scheduling | Preemptive Scheduling |
|---|---|---|
| **CPU Control** | Process holds the CPU until it completes or requests I/O. | OS can forcibly suspend the running process at any time. |
| **Overhead** | Low (fewer context switches). | Higher (frequent context switches and queue updates). |
| **Starvation** | Long jobs can cause high waiting times for short jobs. | Prevents starvation through time slices or dynamic priorities. |
| **Responsiveness**| Poor for interactive time-sharing systems. | High responsiveness; ideal for modern desktop/server OSes. |
| **Examples** | FCFS, Non-Preemptive SJF, Non-Preemptive Priority. | Round Robin (RR), SRTF, Preemptive Priority. |

---

## 3. Core Scheduling Performance Metrics

The OS evaluates scheduling algorithms using the following criteria:

1. **CPU Utilization**: The percentage of time the CPU is actively executing user tasks (Target: 40% to 90%).
2. **Throughput**: The number of processes completed per unit of time (e.g., processes per second).
3. **Turnaround Time (TAT)**: The total elapsed time from when a process is submitted until it completely finishes.
   $$\text{Turnaround Time (TAT)} = \text{Completion Time (CT)} - \text{Arrival Time (AT)}$$
4. **Waiting Time (WT)**: The total amount of time a process spends waiting in the Ready Queue.
   $$\text{Waiting Time (WT)} = \text{Turnaround Time (TAT)} - \text{Burst Time (BT)}$$
5. **Response Time (RT)**: The elapsed time from process arrival until its **very first CPU execution slice**.
   $$\text{Response Time (RT)} = \text{First Time CPU Allocated} - \text{Arrival Time (AT)}$$

---

## 4. FCFS (First-Come, First-Served) Scheduling

### Algorithm Principle:
* The process that requests the CPU first (earliest Arrival Time) is allocated the CPU first.
* Implemented using a simple **FIFO (First-In, First-Out) Queue**.
* Strictly **Non-Preemptive**: Once a process gets the CPU, it runs until its CPU burst is completed.

---

### Step-by-Step Numerical Example

Consider four processes arriving as follows:

| Process | Arrival Time (AT) | Burst Time (BT) |
|:---:|:---:|:---:|
| **P1** | 0 | 24 |
| **P2** | 1 | 3 |
| **P3** | 2 | 3 |

#### Gantt Chart:
```
+---------------------------------------+-------+-------+
|                  P1                   |  P2   |  P3   |
+---------------------------------------+-------+-------+
0                                       24      27      30
```

#### Calculation Table:

| Process | AT | BT | Completion Time (CT) | Turnaround Time (CT - AT) | Waiting Time (TAT - BT) | Response Time |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **P1** | 0 | 24 | 24 | $24 - 0 = 24$ | $24 - 24 = 0$ | $0 - 0 = 0$ |
| **P2** | 1 | 3  | 27 | $27 - 1 = 26$ | $26 - 3 = 23$ | $24 - 1 = 23$ |
| **P3** | 2 | 3  | 30 | $30 - 2 = 28$ | $28 - 3 = 25$ | $27 - 2 = 25$ |

* **Average Turnaround Time (Avg TAT)**: $\frac{24 + 26 + 28}{3} = \frac{78}{3} = 26 \text{ units}$
* **Average Waiting Time (Avg WT)**: $\frac{0 + 23 + 25}{3} = \frac{48}{3} = 16 \text{ units}$

---

## 5. The Convoy Effect

### What is the Convoy Effect?
The **Convoy Effect** is a major disadvantage of FCFS where a single CPU-heavy process with a large burst time arrives first, forcing all subsequent short, I/O-heavy processes to wait behind it in the ready queue.

### Why is it Harmful?
* Like a slow-moving heavy truck on a one-lane highway with sports cars stuck behind it, all fast processes are delayed.
* While the slow process is running on the CPU, I/O devices remain idle.
* When the slow process finally performs I/O, all short processes quickly finish CPU work and queue up at the I/O devices, leaving the CPU idle.
* This results in **poor CPU utilization and poor device utilization**.

### Contrast: What if Short Jobs Arrived First?
If the arrival order was **P2, P3, then P1**:
```
Gantt Chart:
+-------+-------+---------------------------------------+
|  P2   |  P3   |                  P1                   |
+-------+-------+---------------------------------------+
0       3       6                                       30
```
* Waiting Time for P2 = 0
* Waiting Time for P3 = 3
* Waiting Time for P1 = 6
* **New Average Waiting Time**: $\frac{0 + 3 + 6}{3} = 3 \text{ units}$ (compared to $16$ units before!).

---

## 6. FCFS Pros and Cons Summary

| Advantages | Disadvantages |
|---|---|
| Simplest scheduling algorithm to understand and write. | Non-preemptive nature causes the **Convoy Effect**. |
| Deterministic; zero scheduling logic overhead. | Unpredictable and high average waiting times. |
| No complex priority computations. | Completely unsuited for interactive time-sharing systems. |
