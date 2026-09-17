# Deadlock Avoidance and the Banker's Algorithm

**Deadlock Avoidance** is a dynamic strategy where the operating system inspects every resource request at runtime and only grants the request if it guarantees that the system will remain in a **Safe State**.

---

## 1. Safe State vs. Unsafe State vs. Deadlock

```
+---------------------------------------------------------------+
|                       State Space                             |
|  +---------------------------------------------------------+  |
|  |                    Safe States                          |  |
|  |           (Guaranteed Deadlock-Free)                    |  |
|  +---------------------------------------------------------+  |
|                                                               |
|  +---------------------------------------------------------+  |
|  |                   Unsafe States                         |  |
|  |       (Risk of Deadlock; May or May Not Deadlock)       |  |
|  |      +-------------------------------------------+      |  |
|  |      |             Deadlocked States             |      |  |
|  |      +-------------------------------------------+      |  |
|  +---------------------------------------------------------+  |
+---------------------------------------------------------------+
```

* **Safe State**: A state is safe if there exists at least one **Safe Sequence** $\langle P_0, P_1, P_2, \dots, P_{n-1} \rangle$ of processes such that, for each process $P_i$, the maximum resources $P_i$ still needs can be satisfied by the currently available resources plus the resources already held by all preceding processes $P_0, \dots, P_{i-1}$.
* **Unsafe State**: A state where no safe sequence exists.  
  * **Critical Distinction**: An unsafe state is **not** necessarily a deadlock! However, once in an unsafe state, the OS can no longer prevent processes from potentially requesting maximum resources and causing a deadlock.
* **Deadlock State**: A state where circular wait has occurred and no forward progress is possible.

---

## 2. Banker's Algorithm: Data Structures

Named because it mimics a small town banker who never commits cash loans in a way that prevents fulfilling all credit limits:

Let $n$ = number of processes, and $m$ = number of distinct resource types.

1. **`Available[m]`**: A 1D array of length $m$.  
   `Available[j] = k` means there are $k$ unallocated instances of resource type $R_j$ currently free.
2. **`Max[n][m]`**: A 2D array of size $n \times m$.  
   `Max[i][j] = k` means process $P_i$ may request at most $k$ instances of resource type $R_j$ over its lifetime.
3. **`Allocation[n][m]`**: A 2D array of size $n \times m$.  
   `Allocation[i][j] = k` means process $P_i$ is currently holding $k$ instances of resource type $R_j$.
4. **`Need[n][m]`**: A 2D array of size $n \times m$ representing remaining needs:
   $$\text{Need}[i][j] = \text{Max}[i][j] - \text{Allocation}[i][j]$$

---

## 3. The Safety Algorithm Step-by-Step

Used to determine whether the current state of a system is safe:

1. **Initialize Working Vectors**:
   * Let `Work` be an array of length $m$, initialized to `Available`.
   * Let `Finish` be a boolean array of length $n$, initialized to `false` for all processes.
2. **Find an Eligible Process**:
   * Search for an index $i$ such that:
     * `Finish[i] == false`
     * `Need[i] <= Work` (for all resource types)
   * If no such $i$ exists, go to **Step 4**.
3. **Simulate Execution & Reclamation**:
   * Assume process $P_i$ finishes its work and releases all its allocated resources:
     $$\text{Work} = \text{Work} + \text{Allocation}[i]$$
     $$\text{Finish}[i] = \text{true}$$
   * Append $P_i$ to the safe sequence and return to **Step 2**.
4. **Evaluate System Safety**:
   * If `Finish[i] == true` for **all** processes, then the system is in a **Safe State**, and the ordered list of processes forms a valid **Safe Sequence**.
   * If any `Finish[i] == false`, the system is in an **Unsafe State**.

---

## 4. Comprehensive Numerical Example

Suppose a system has **5 Processes ($P_0, P_1, P_2, P_3, P_4$)** and **3 Resource Types (A, B, C)** with total instances: **A = 10, B = 5, C = 7**.

### Current Snapshot:

| Process | Allocation (A B C) | Max (A B C) | Need = Max - Alloc (A B C) | Available (A B C) |
|:---:|:---:|:---:|:---:|:---:|
| **P0** | 0 1 0 | 7 5 3 | **7 4 3** | **3 3 2** |
| **P1** | 2 0 0 | 3 2 2 | **1 2 2** | |
| **P2** | 3 0 2 | 9 0 2 | **6 0 0** | |
| **P3** | 2 1 1 | 2 2 2 | **0 1 1** | |
| **P4** | 0 0 2 | 4 3 3 | **4 3 1** | |

*(Total Allocated: $A=7, B=2, C=5$. Available = Total - Allocated = $(10-7, 5-2, 7-5) = \mathbf{(3, 3, 2)}$.)*

---

### Step-by-Step Safety Execution:

* **Initial State**: $\text{Work} = (3, 3, 2)$. All $\text{Finish} = \text{false}$.

1. **Check P0**: $\text{Need}(7, 4, 3) \le \text{Work}(3, 3, 2)$? ❌ **No**. Skip P0 for now.
2. **Check P1**: $\text{Need}(1, 2, 2) \le \text{Work}(3, 3, 2)$?  **Yes!**
   * P1 can run and complete.
   * $\text{Work} = \text{Work} + \text{Alloc}(2, 0, 0) = (3+2, 3+0, 2+0) = \mathbf{(5, 3, 2)}$.
   * $\text{Finish}[P1] = \text{true}$. Safe sequence: $\langle P_1 \rangle$.
3. **Check P3**: $\text{Need}(0, 1, 1) \le \text{Work}(5, 3, 2)$?  **Yes!**
   * P3 can run and complete.
   * $\text{Work} = (5, 3, 2) + (2, 1, 1) = \mathbf{(7, 4, 3)}$.
   * $\text{Finish}[P3] = \text{true}$. Safe sequence: $\langle P_1, P_3 \rangle$.
4. **Check P4**: $\text{Need}(4, 3, 1) \le \text{Work}(7, 4, 3)$?  **Yes!**
   * P4 runs and completes.
   * $\text{Work} = (7, 4, 3) + (0, 0, 2) = \mathbf{(7, 4, 5)}$.
   * $\text{Finish}[P4] = \text{true}$. Safe sequence: $\langle P_1, P_3, P_4 \rangle$.
5. **Check P0**: $\text{Need}(7, 4, 3) \le \text{Work}(7, 4, 5)$?  **Yes!**
   * P0 runs and completes.
   * $\text{Work} = (7, 4, 5) + (0, 1, 0) = \mathbf{(7, 5, 5)}$.
   * $\text{Finish}[P0] = \text{true}$. Safe sequence: $\langle P_1, P_3, P_4, P_0 \rangle$.
6. **Check P2**: $\text{Need}(6, 0, 0) \le \text{Work}(7, 5, 5)$?  **Yes!**
   * P2 runs and completes.
   * $\text{Work} = (7, 5, 5) + (3, 0, 2) = \mathbf{(10, 5, 7)}$.
   * $\text{Finish}[P2] = \text{true}$. Safe sequence: $\langle P_1, P_3, P_4, P_0, P_2 \rangle$.

**Result**: All processes finished!  
**Safe Sequence Found**: $\mathbf{\langle P_1, P_3, P_4, P_0, P_2 \rangle}$. The system is in a **Safe State**.

---

## 5. Resource-Request Algorithm

When process $P_i$ requests a vector of resources $\text{Request}_i$:
1. If $\text{Request}_i > \text{Need}_i \implies$ **Error**: Process exceeded its declared maximum claim.
2. If $\text{Request}_i > \text{Available} \implies$ $P_i$ must **wait** (insufficient resources free).
3. **Pretend Allocation**:
   $$\text{Available} = \text{Available} - \text{Request}_i$$
   $$\text{Allocation}_i = \text{Allocation}_i + \text{Request}_i$$
   $$\text{Need}_i = \text{Need}_i - \text{Request}_i$$
4. Run the **Safety Algorithm**:
   * If safe $\implies$ Grant the request permanently.
   * If unsafe $\implies$ Rollback the pretend allocation; $P_i$ must wait.

---

## 6. Practical Limitations of Banker's Algorithm

While mathematically elegant, Banker's Algorithm is rarely used in modern general-purpose operating systems because:
1. **Requires Advance Knowledge**: Processes must state their absolute maximum resource needs upfront, which is impossible for interactive or long-running applications.
2. **Fixed Number of Resources & Processes**: Assumes the number of processes and resources remains constant; in real OSes, users spawn and terminate processes dynamically.
3. **Computational Overhead**: Checking safety matrix arithmetic on every single resource request introduces massive CPU latency.
