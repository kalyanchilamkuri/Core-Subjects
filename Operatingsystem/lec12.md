# Orphan and Zombie Processes: Lifecycle, Differences, and Handling

---

## 1. Quick Refreshers

* **Degree of Multiprogramming**: The total number of active processes residing in main memory (RAM) at any given moment. Controlled by the Long-Term Scheduler (LTS).
* **Process Table**: An OS internal table in kernel space containing one entry for every active process, storing its PID, state, and pointers to its PCB.
* **Parent-Child Relationship**: When a process calls `fork()`, it becomes a **parent** to the resulting **child** process.

---

## 2. Orphan Process

### What is an Orphan Process?
An **Orphan Process** is a running process whose **parent process terminates or crashes before the child finishes execution**.

```
Before Parent Termination:
[ Parent Process (PID: 100) ] ───created───► [ Child Process (PID: 200) ]

Parent Exits First:
[ Parent (PID: 100) ] === Exits! ===
[ Child (PID: 200) ]  ─── Left running alone without a parent (Orphaned!)

Operating System Adoption:
[ systemd / init (PID: 1) ] ─── adopts ───► [ Child Process (PID: 200) ]
```

### What Happens to an Orphan?
1. The operating system does **not** terminate the orphan process. The child continues executing normally.
2. The operating system immediately re-parents the orphan by having the root ancestor process (**`systemd`** or `init`, PID 1) **adopt** it.
3. When the child process finally terminates, PID 1 automatically calls `wait()` on it, cleanly removing it from the system.

### C Code Simulating an Orphan Process:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        // Parent process terminates immediately
        printf("Parent (PID: %d) exiting...\n", getpid());
        exit(0);
    } else if (pid == 0) {
        // Child sleeps while parent exits, becoming an orphan
        printf("Child (PID: %d) initial parent PPID: %d\n", getpid(), getppid());
        sleep(5);
        
        // After waking up, the parent is dead and PID 1 has adopted it
        printf("Child (PID: %d) after sleep adopted by PPID: %d\n", getpid(), getppid());
    }

    return 0;
}
```

---

## 3. Zombie Process (Defunct Process)

### What is a Zombie Process?
A **Zombie Process** is a process that has **finished execution (called `exit()`), but its entry still remains in the OS Process Table** because its parent process has not yet read its exit status via the `wait()` system call.

```
Child Execution:
[ Child (PID: 200) ] ─── calls exit(0) ───► Frees RAM, code, heap, stack.

Kernel State:
[ Kernel Process Table ] ─── Keeps entry: PID 200, Exit Code 0, State: ZOMBIE (Z)

Parent Still Running:
[ Parent (PID: 100) ] ─── Busy doing other work, never called wait()!
```

### Why Do Zombie Processes Exist?
* When a child process terminates, the OS frees its memory, open files, and CPU resources.
* However, the OS **must retain the child's entry in the process table** to preserve its termination status (exit code).
* This allows the parent to later query whether the child succeeded, failed, or was terminated by a signal.
* Once the parent process calls `wait()` or `waitpid()`, the child's exit status is collected, and the zombie is **reaped** (purged from the process table).

### The Danger of Zombie Processes:
* A single zombie process consumes no CPU or RAM.
* However, because it occupies an entry in the OS Process Table, having thousands of un-reaped zombies leads to **PID Exhaustion**.
* Once all available Process IDs are exhausted, the operating system cannot spawn any new processes!

### C Code Simulating a Zombie Process:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        // Child exits immediately
        printf("Child (PID: %d) exiting now...\n", getpid());
        exit(0);
    } else {
        // Parent sleeps without calling wait(), leaving child as a zombie
        printf("Parent (PID: %d) sleeping for 30 seconds...\n", getpid());
        printf("Check terminal with: ps aux | grep 'Z'\n");
        sleep(30);
    }

    return 0;
}
```

---

## 4. How to Prevent and Clean Up Zombie Processes

1. **Call `wait()` or `waitpid()` in the Parent**:
   * Synchronously wait for the child to finish.
2. **Handle the `SIGCHLD` Signal Asynchronously**:
   * Whenever a child terminates, the kernel sends a `SIGCHLD` signal to the parent. The parent can register a signal handler that automatically calls `waitpid()` inside the handler:
     ```c
     signal(SIGCHLD, SIG_IGN); // Tells kernel to automatically reap dead children
     ```
3. **Kill the Parent**:
   * If a parent process is misbehaving and generating zombies without calling `wait()`, you cannot kill the zombies directly (they are already dead!).
   * **Solution**: Kill the parent process. Once the parent dies, all its zombie children become orphans and are adopted by `systemd` (PID 1), which immediately reaps them.

---

## 5. Comprehensive Comparison: Orphan vs. Zombie

| Feature | Orphan Process | Zombie Process |
|---|---|---|
| **Execution State** | Still alive and actively executing instructions. | Finished execution (dead/terminated). |
| **Parent State** | Parent has died/terminated. | Parent is still alive, but has not called `wait()`. |
| **Resources Used** | Consumes CPU, RAM, and system resources. | Consumes 0 CPU and 0 RAM; only occupies an entry in the Process Table. |
| **OS Adoption** | Adopted by `systemd` / `init` (PID 1). | Stays in state `Z` (Defunct) until parent calls `wait()` or parent dies. |
| **System Threat** | None (acts like a normal background daemon). | High if accumulated (leads to PID exhaustion). |
| **How to Resolve** | Runs to completion; reaped cleanly by PID 1. | Call `wait()` / `waitpid()` in parent, or terminate the parent process. |
