# Process Creation, Termination, and System Calls (`fork`, `exec`, `wait`, `exit`)

---

## 1. The Process Hierarchy Tree

In UNIX-like operating systems, processes are organized into a strict hierarchical tree:
* When the operating system boots, the kernel creates the root process: **`systemd`** (or `init`), which is assigned **Process ID 1 (PID 1)**.
* Every subsequent process in the system is created by an existing process through a parent-child relationship:
  * Each process has a unique identifier called **PID (Process ID)**.
  * Each process also tracks its creator through **PPID (Parent Process ID)**.

```
                    [ systemd (PID: 1) ]
                        /          \
                       /            \
          [ sshd (PID: 104) ]    [ login (PID: 210) ]
                 │                      │
          [ bash (PID: 320) ]    [ bash (PID: 450) ]
                 │
          [ ls (PID: 615) ]
```

---

## 2. The `fork()` System Call

The **`fork()`** system call is used by a parent process to create a brand-new child process.

### How `fork()` Works:
* It creates an **exact duplicate** of the calling parent process.
* The child process receives a copy of the parent’s address space (code, data, heap, stack, and open file descriptors).
* Both processes continue execution from the instruction immediately following the `fork()` call!

### Return Values of `fork()`:
Because `fork()` is called once but returns in **two separate processes**, it returns different values:
1. **To the Child Process**: Returns **`0`**.
2. **To the Parent Process**: Returns the **PID of the newly created child** (a positive integer).
3. **On Failure**: Returns **`-1`** to the parent (no child is created).

### Example Code:
```c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        // Error handling
        printf("Fork failed!\n");
        return 1;
    } else if (pid == 0) {
        // Child process execution block
        printf("I am the CHILD process! PID: %d, Parent PID: %d\n", getpid(), getppid());
    } else {
        // Parent process execution block
        printf("I am the PARENT process! PID: %d, Created Child PID: %d\n", getpid(), pid);
    }

    return 0;
}
```

### Formula for Number of Processes Created:
If a program calls `fork()` sequentially $n$ times without conditional branches:
$$\text{Total Processes} = 2^n$$
$$\text{Child Processes Created} = 2^n - 1$$

---

## 3. Copy-on-Write (COW) Optimization

In early operating systems, `fork()` physically copied all memory pages from parent to child, which was extremely slow and wasteful.

Modern operating systems use **Copy-on-Write (COW)**:
1. Immediately after `fork()`, parent and child **share the exact same physical memory pages in RAM** marked as *read-only*.
2. As long as both processes only read from memory, no physical copying occurs.
3. If either process attempts to **modify (write)** a memory page, the CPU hardware raises a protection fault.
4. The kernel intercepts the fault, creates a duplicate of only that specific 4 KB page for the writing process, marks it writable, and resumes execution.
5. This makes `fork()` virtually instantaneous!

---

## 4. The `exec()` Family of System Calls

While `fork()` duplicates the current process, **`exec()`** replaces the entire process memory image with a completely new program:

```
Process Memory Before exec():            Process Memory After exec("ls"):
+---------------------------+            +---------------------------+
| Stack (my_program.c)      |            | Stack (ls binary)         |
| Heap                      |    ====>   | Heap                      |
| Data                      |            | Data                      |
| Code: my_program.c        |            | Code: /bin/ls instructions|
+---------------------------+            +---------------------------+
       (Same PID is retained; only memory and code are replaced!)
```

### How `exec()` Works:
* It loads an executable binary file from disk into the process's address space.
* It completely wipes the existing code, stack, and heap, replacing them with the new program.
* **Important**: The PID remains identical. If `exec()` succeeds, it never returns to the original program code because the original code was overwritten!

### Example: Running a Child Program with `fork()` + `exec()`
```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        // Inside child: replace current image with "ls -l"
        char *args[] = {"/bin/ls", "-l", NULL};
        execvp(args[0], args);
        
        // This line only executes if execvp fails
        printf("execvp failed!\n");
    } else {
        // Inside parent: wait for child to complete
        wait(NULL);
        printf("Child process finished execution.\n");
    }

    return 0;
}
```

---

## 5. The `wait()` and `waitpid()` System Calls

When a parent process spawns a child, it often needs to wait for the child to finish before continuing:

* **`wait(&status)`**:
  * Suspends the calling parent process until **any** one of its child processes terminates.
  * Stores the child's exit status code in `status`.
  * Returns the PID of the terminated child.
* **`waitpid(pid, &status, options)`**:
  * Suspends the parent until a **specific** child with the given `pid` terminates.
  * Allows non-blocking checks using the `WNOHANG` flag.

---

## 6. The `exit()` System Call

A process terminates execution by calling **`exit(status_code)`**:
1. All allocated memory (heap, stack), file descriptors, and open connections are reclaimed by the kernel.
2. The process is removed from active CPU scheduling.
3. The process's exit code (e.g., `0` for success, non-zero for error) is retained in the OS Process Table so the parent can inspect it using `wait()`.
4. Until the parent calls `wait()`, the terminated child remains in the **Zombie state**.

---

## 7. Summary: The Complete Process Creation Cycle

| System Call | Purpose | Return Value |
|---|---|---|
| **`fork()`** | Spawns a child duplicate of the calling process | `0` to child, child PID to parent |
| **`exec()`** | Overwrites the calling process with a new program | Does not return on success; `-1` on error |
| **`wait()`** | Blocks parent until a child process terminates | PID of terminated child |
| **`exit()`** | Ends process execution and reports exit status to OS | Does not return |
