# Deadlocks: The Four Coffman Conditions, Resource Allocation Graphs, and Prevention

A **Deadlock** is an operating system state in which two or more processes are permanently blocked because each is holding a resource that another process needs, and neither can proceed.

```
       [ Resource 1 ] ◄──── (held by) ──── [ Process A ]
             ▲                                    │
             │                                (requests)
         (requests)                               │
             │                                    ▼
       [ Process B ] ──── (held by) ────► [ Resource 2 ]
```

> **Real-World Analogy: Traffic Gridlock**
> Four cars enter a 4-way intersection simultaneously. Each car blocks the car behind it while waiting for the car in front to move. None of the cars can advance, and none can reverse. The intersection is deadlocked!

---

## 1. The Four Necessary Coffman Conditions

E.G. Coffman proved in 1971 that a deadlock can occur **if and only if all four of the following conditions hold simultaneously**:

### 1. Mutual Exclusion
* At least one resource must be non-shareable. Only one process can hold the resource at any given time.
* If another process requests that resource, the requesting process must wait until it is released.

### 2. Hold and Wait
* A process must currently be holding at least one resource and simultaneously waiting to acquire additional resources that are held by other processes.

### 3. No Preemption
* Resources cannot be forcibly taken away from a process while it is using them.
* A resource can only be released voluntarily by the process holding it after completing its task.

### 4. Circular Wait
* A closed circular chain of processes exists:
  $$\{P_0, P_1, P_2, \dots, P_n\}$$
  where $P_0$ is waiting for a resource held by $P_1$, $P_1$ is waiting for a resource held by $P_2$, and $P_n$ is waiting for a resource held by $P_0$.

---

## 2. Resource Allocation Graphs (RAG)

A **Resource Allocation Graph (RAG)** is a directed graph used to model and detect deadlocks:
* **Vertices (V)**:
  * Processes: Represented by circles ($P_1, P_2$).
  * Resource Types: Represented by boxes ($R_1, R_2$) containing dots representing the number of resource instances.
* **Edges (E)**:
  * **Request Edge ($P_i \to R_j$)**: Process $P_i$ is currently waiting for an instance of resource $R_j$.
  * **Assignment Edge ($R_j \to P_i$)**: An instance of resource $R_j$ has been allocated to process $P_i$.

```
Single-Instance Resource Deadlock:
    ( P1 ) ───────► [ R1 (•) ]
      ▲                 │
      │                 │ (Allocated to P2)
      │                 ▼
   [ R2 (•) ] ◄─────── ( P2 )
  (Allocated to P1)
        A cycle with single-instance resources GUARANTEES deadlock!
```

### Cycle Rules in RAG:
1. **If the graph contains NO cycles**: The system is **guaranteed to be deadlock-free**.
2. **If the graph contains a cycle**:
   * **Single instance per resource type**: A cycle is a **necessary and sufficient condition** for deadlock (Deadlock exists!).
   * **Multiple instances per resource type**: A cycle is a **necessary but NOT sufficient condition** (Deadlock *might* exist; depends on whether other processes holding instances can complete and release them).

---

## 3. The Four Deadlock Handling Strategies

1. **Deadlock Prevention**: Eliminate the possibility of deadlocks by designing system rules that guarantee at least one Coffman condition can never hold.
2. **Deadlock Avoidance**: Dynamically evaluate resource requests at runtime and allocate them only if the resulting state remains safe (e.g., **Banker's Algorithm**).
3. **Deadlock Detection and Recovery**: Allow deadlocks to occur, periodically detect them using algorithms, and resolve them by terminating processes or preempting resources.
4. **Ostrich Algorithm**: Ignore the problem completely. Used by most general-purpose OSes (Linux, Windows, macOS) because true deadlocks are rare and proactive mechanisms impose heavy performance overhead.

---

## 4. Deadlock Prevention: Breaking the 4 Conditions

To prevent deadlocks, we ensure that at least one of the four Coffman conditions is structurally invalidated:

### A. Invalidate Mutual Exclusion
* **Strategy**: Make all resources shareable.
* **Reality**: Feasible for read-only files, but physically impossible for inherently exclusive resources (e.g., printers, write-locks, tape drives).

### B. Invalidate Hold and Wait
* **Protocol 1 (Conservative All-or-Nothing)**: A process must request and receive **all required resources upfront** before beginning execution. If even one is unavailable, it receives none.
  * *Disadvantage*: Terrible resource utilization; resources reserved for late in the process sit idle.
* **Protocol 2**: A process can request new resources only when it currently holds zero resources (must release everything before asking for more).

### C. Invalidate No Preemption
* **Strategy**: If process $P_1$ holds resources and requests resource $R$ which is currently allocated to waiting process $P_2$, the OS **forcibly snatches $R$ from $P_2$** and gives it to $P_1$.
* **Reality**: Feasible for CPU state and memory pages (which can be saved and restored cleanly), but unsuitable for stateful peripherals like database transactions or printers midway through a print job.

### D. Invalidate Circular Wait (Most Practical & Widely Used!)
* **Havender's Linear Ordering Strategy**:
  1. Assign a globally unique positive integer index to every resource type:
     $$F: R \rightarrow \mathbb{N} \quad (\text{e.g., } F(\text{Disk}) = 1, \; F(\text{Printer}) = 2, \; F(\text{Database}) = 3)$$
  2. Enforce the strict rule: **A process can only request resources in strictly increasing numerical order**.
  3. If a process holds resource with index $k$, it can only request resources with index $> k$.
* **Proof**: A cycle requires a dependency chain with decreasing indices, which is impossible under strictly ascending order. **Circular wait is mathematically eliminated!**

---

## 5. Deadlock vs. Starvation vs. Livelock

| Concept | What It Is | CPU Utilization | Progression |
|---|---|:---:|---|
| **Deadlock** | Processes permanently blocked waiting on each other's resources. | 0% (Processes sleeping) | Zero forward progress forever. |
| **Starvation** | A runnable process is indefinitely delayed because higher-priority tasks always cut in line. | Active | Other processes progress; the victim is starved. |
| **Livelock** | Processes continuously change state in reaction to each other without accomplishing work. | **100%** (Active CPU spinning) | Zero forward progress despite rapid activity. |
