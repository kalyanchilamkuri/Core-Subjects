# Process Synchronization: Critical Section, Race Conditions, and Peterson's Algorithm

When multiple threads or processes execute concurrently and share mutable resources (variables, memory, files, sockets), they must be coordinated. Without proper **synchronization**, unpredictable errors and data corruption occur.

---

## 1. Concrete Example of a Race Condition

A **Race Condition** occurs when multiple threads concurrently access and modify shared data, and the final outcome depends on the non-deterministic order in which the CPU schedules their execution.

### The Bank Account Problem:
Imagine a shared bank account initialized with \$100. Two threads simultaneously run:

```cpp
// Shared Variable:
int balance = 100;

// Thread 1: Deposit $50             // Thread 2: Withdraw $30
balance = balance + 50;              balance = balance - 30;
```

At the assembly level, `balance = balance + 50` is **not atomic**; it compiles into three distinct machine instructions:
1. `MOV R1, balance`  *(Read from memory into CPU register)*
2. `ADD R1, 50`       *(Add 50)*
3. `MOV balance, R1`  *(Write back to memory)*

### What Happens During a Bad Context Switch:
```
Time  Thread 1 (Deposit $50)            Thread 2 (Withdraw $30)         Memory (balance)
----  ----------------------            -----------------------         ----------------
 t1   1. MOV R1, balance (R1=100)                                           100
 t2   2. ADD R1, 50 (R1=150)                                                100
 ---  [ Context Switch to Thread 2! ]
 t3                                     1. MOV R2, balance (R2=100)         100
 t4                                     2. SUB R2, 30 (R2=70)               100
 t5                                     3. MOV balance, R2 (Writes 70!)      70
 ---  [ Context Switch back to Thread 1! ]
 t6   3. MOV balance, R1 (Writes 150!)                                      150!
```
* **Expected Final Balance**: $\$100 + \$50 - \$30 = \$120$.
* **Actual Result**: $\$150$ (Thread 2's withdrawal was completely erased!).
* This catastrophic loss of updates is the direct consequence of an unprotected **Critical Section**.

---

## 2. Structure of the Critical Section Problem

Any program accessing shared data is conceptually divided into four sections:

```
do {
    // 1. Entry Section: Request permission to enter
    [ acquire_lock() ]

        // 2. Critical Section: Read and write shared variables
        [ balance = balance + 50; ]

    // 3. Exit Section: Release lock and signal waiting threads
    [ release_lock() ]

    // 4. Remainder Section: Independent local computation
    [ do_unrelated_work(); ]

} while (true);
```

---

## 3. The Three Mandatory Criteria for Any Valid Solution

A synchronization solution is only valid if it guarantees all three conditions:

1. **Mutual Exclusion (Safety)**:
   * Only one process/thread can be executing inside its critical section at any given time.
2. **Progress (Liveness)**:
   * If no process is in the critical section and some processes want to enter, only processes currently in their Entry Section (not those in Remainder Section) can participate in deciding who enters next.
   * This decision **cannot be postponed indefinitely** (no deadlock).
3. **Bounded Waiting (Fairness)**:
   * There must exist a strict bound on the number of times other processes are allowed to enter their critical sections after a process has requested entry, before that request is granted.
   * **No process should starve forever**.

---

## 4. Hardware Synchronization Primitives

Modern processors provide atomic hardware instructions that execute in a single CPU cycle without interruption:

### A. Test-and-Set Instruction (Atomic Primitive)
```c
bool TestAndSet(bool *target) {
    bool rv = *target;
    *target = true; // Sets target to true and returns the old value atomically
    return rv;
}
```
* Can be used to build a **Spinlock**:
```c
bool lock = false;

void acquire() {
    while (TestAndSet(&lock)); // Busy-wait until lock was false
}

void release() {
    lock = false;
}
```

### B. Compare-and-Swap (CAS)
```c
int CompareAndSwap(int *value, int expected, int new_value) {
    int temp = *value;
    if (*value == expected)
        *value = new_value;
    return temp;
}
```
* Forms the core of **lock-free** data structures and atomic variables in modern programming languages (`std::atomic` in C++, `AtomicInteger` in Java).

---

## 5. Software Solution: Peterson's Algorithm (2 Processes)

Peterson's Algorithm is a classic software-based solution that guarantees **Mutual Exclusion, Progress, and Bounded Waiting** for strictly two processes ($P_0$ and $P_1$).

### Shared Variables:
```c
bool flag[2]; // flag[i] = true means Process i wants to enter Critical Section
int turn;     // turn = i means it is Process i's turn to enter
```

### Algorithm Code:

```c
// Code for Process P0:
void process_0() {
    while (true) {
        flag[0] = true;      // "I am interested in entering"
        turn = 1;            // "I politely offer the turn to Process 1"

        // Busy-wait if Process 1 is also interested AND it's Process 1's turn
        while (flag[1] == true && turn == 1);

        // --- CRITICAL SECTION ---
        // Access shared data safely!
        // ------------------------

        flag[0] = false;     // "I am done, leaving Critical Section"

        // --- REMAINDER SECTION ---
    }
}

// Code for Process P1:
void process_1() {
    while (true) {
        flag[1] = true;      // "I am interested in entering"
        turn = 0;            // "I politely offer the turn to Process 0"

        // Busy-wait if Process 0 is also interested AND it's Process 0's turn
        while (flag[0] == true && turn == 0);

        // --- CRITICAL SECTION ---
        // Access shared data safely!
        // ------------------------

        flag[1] = false;     // "I am done, leaving Critical Section"

        // --- REMAINDER SECTION ---
    }
}
```

### Proof of Correctness:
1. **Mutual Exclusion**: For both processes to enter simultaneously, both `flag[0] == true` and `flag[1] == true`. However, the variable `turn` can only hold one value (either `0` or `1`) at any physical moment. Hence, one process is guaranteed to be held in the `while` loop while the other executes.
2. **Progress**: If $P_1$ does not wish to enter (`flag[1] == false`), the loop condition `flag[1] == true && turn == 1` immediately evaluates to `false`, allowing $P_0$ to enter without delay.
3. **Bounded Waiting**: Because each process politely sets `turn` to the other process upon entry, $P_0$ cannot exit and immediately re-enter if $P_1$ is waiting; $P_1$ is guaranteed to get the very next turn.

### Why Software Algorithms Fail on Modern CPUs:
* Peterson's algorithm assumes strict sequential memory consistency.
* Modern multi-core processors reorder memory read/write instructions for pipeline performance and use store buffers. Without explicit **Memory Fences (Barriers)**, software-only synchronization can fail on modern hardware.
