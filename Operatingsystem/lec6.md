# System Calls and Kernel Architecture

---

## 1. Dual-Mode Operation in Operating Systems

To protect the hardware, operating system, and other users from misbehaving programs, modern CPUs provide hardware support for **Dual-Mode Operation**:

```
+--------------------------------------------------------------+
|                         User Space                           |
|       [ Browser ]      [ Video Player ]     [ Compiler ]     |
|                   (Mode Bit = 1: User Mode)                  |
+--------------------------------------------------------------+
                           |           ^
        System Call (Trap) |           | Return from System Call
                           v           |
+--------------------------------------------------------------+
|                         Kernel Space                         |
|   [ Process Scheduler ]  [ Memory Manager ]  [ Device Drivers]|
|                  (Mode Bit = 0: Kernel Mode)                 |
+--------------------------------------------------------------+
|                         Hardware                             |
|          [ CPU ]         [ Memory (RAM) ]        [ Disks ]   |
+--------------------------------------------------------------+
```

### A. User Mode (Mode Bit = 1)
* Standard application programs (browsers, text editors, games) execute in user mode.
* The CPU restricts access to critical instructions: programs cannot directly read/write physical hardware, execute I/O instructions, or modify interrupt vector tables.
* If a program attempts a forbidden privileged instruction or accesses unauthorized memory, the CPU generates an exception (e.g., Segmentation Fault) and terminates the program without affecting the rest of the OS.

### B. Kernel Mode (Mode Bit = 0, also called Supervisor / Privileged Mode)
* The OS kernel and core device drivers execute in kernel mode.
* The CPU grants unrestricted access to all machine instructions, memory ranges, and peripheral registers.
* **Risk**: A crash or bug in kernel mode can panic the CPU and crash the entire machine (e.g., Blue Screen of Death in Windows or Kernel Panic in Linux).

---

## 2. Switching Between User Mode and Kernel Mode

A mode transition occurs through hardware-assisted **traps** (software interrupts) or **hardware interrupts**:

```
User Program               Hardware (CPU)               OS Kernel
------------               --------------               ---------
Calls read()      ----->   Trigger Trap (syscall)
                           Save program counter & registers
                           Set Mode Bit = 0 (Kernel)   -----> Look up Syscall Table
                                                              Execute sys_read()
                           Set Mode Bit = 1 (User)     <----- Return result
Returns to User   <-----   Restore program counter
```

1. **Trigger**: An application issues a system call (e.g., `read()` or `write()`).
2. **Trap**: The CPU executes a special trap instruction (`syscall` on x86-64), which saves current registers and switches the mode bit from `1` to `0`.
3. **Dispatch**: The CPU looks up the requested system call number in the **System Call Dispatch Table** stored in kernel memory.
4. **Execution**: The kernel executes the privileged routine safely after verifying user parameters.
5. **Return**: The kernel sets the return value, executes a return-from-trap instruction (`sysret`), which flips the mode bit back to `1` and resumes user code.

---

## 3. Shell vs. Kernel

* **The Shell (Command Interpreter)**:
  * An interface (CLI like `bash`/`zsh`/`PowerShell` or GUI like Windows Explorer) that receives human input, interprets commands, and requests services from the kernel.
  * The shell runs entirely in **user mode**.
* **The Kernel**:
  * The core software module running permanently in **kernel mode**.
  * It acts as the direct manager of all hardware devices and schedules process execution.

---

## 4. Primary Responsibilities of the OS Kernel

1. **Process Management**:
   * Creates, schedules, pauses, and terminates processes.
   * Maintains the **Process Control Block (PCB)** and switches execution contexts between processes.
2. **Memory Management**:
   * Manages physical RAM allocation and virtual address translation (Paging and Segmentation).
   * Ensures processes remain isolated and prevents one process from modifying another's memory.
3. **File System Management**:
   * Organizes blocks of raw disk storage into structured files, folders, and directories.
   * Manages metadata, file permissions, and access control lists (ACLs).
4. **Device Management (I/O Subsystem)**:
   * Communicates with physical peripherals using specialized **Device Drivers**.
   * Implements **Buffering**, **Caching**, and **Spooling** to reconcile speed differences between fast CPUs and slow mechanical/electrical I/O devices.
5. **Security and Protection**:
   * Enforces user access controls, authentication, and secure isolation between independent users.

---

## 5. Major Categories of System Calls

| Category | Primary Function | Common POSIX Examples | Common Windows API Examples |
|---|---|---|---|
| **Process Control** | Create, terminate, wait for processes | `fork()`, `exec()`, `exit()`, `wait()` | `CreateProcess()`, `ExitProcess()`, `WaitForSingleObject()` |
| **File Management** | Open, read, write, close, create files | `open()`, `read()`, `write()`, `close()` | `CreateFile()`, `ReadFile()`, `WriteFile()`, `CloseHandle()` |
| **Device Management**| Request, read, write, release devices | `ioctl()`, `read()`, `write()` | `DeviceIoControl()`, `SetConsoleMode()` |
| **Information Maintenance**| Get time, process info, system attributes | `getpid()`, `time()`, `alarm()` | `GetCurrentProcessId()`, `GetSystemTime()` |
| **Communication** | Inter-process messaging and networking | `pipe()`, `shmget()`, `socket()` | `CreatePipe()`, `CreateFileMapping()`, `socket()` |
| **Protection** | Manage file permissions and user rights | `chmod()`, `chown()`, `umask()` | `SetNamedSecurityInfo()`, `GetSecurityInfo()` |

---

## 6. Kernel Architectural Models

```
    Monolithic Kernel                       Microkernel
+-------------------------+         +-------------------------+
| User Applications (Ring 3)        | User Applications (Ring 3)
|                         |         | [File Sys]  [Drivers]   |  <-- In User Mode!
+-------------------------+         +-------------------------+
| [ VFS ]   [ Scheduler ] |         |        IPC Messaging    |
| [ Memory] [ Drivers ]   |         +-------------------------+
| (All in Kernel Mode Ring 0)       | Core Kernel (Ring 0)    |
|                         |         | [Basic Scheduler/Memory]|
+-------------------------+         +-------------------------+
```

### A. Monolithic Kernel (e.g., Linux, Unix)
* **Architecture**: All operating system services (Virtual File System, CPU Scheduler, Memory Management, Device Drivers) reside in a single large address space inside kernel mode.
* **Advantages**: High execution performance because internal subsystems communicate via fast direct C function calls.
* **Disadvantages**: A defect in any device driver can corrupt kernel memory and crash the entire operating system.

### B. Microkernel (e.g., QNX, Minix, Mach)
* **Architecture**: Only minimal essential primitives (minimal scheduling, memory page mapping, Inter-Process Communication) run in kernel mode. All file systems, networking stacks, and device drivers run as separate servers in user space.
* **Advantages**: High security and reliability. If a network driver crashes, it can be restarted in user space without crashing the OS.
* **Disadvantages**: Performance overhead due to repeated message passing and mode switches across the user-kernel boundary.

### C. Hybrid Kernel (e.g., Windows NT, macOS XNU)
* Combines the architectural modularity of a microkernel with the high execution performance of a monolithic kernel by running selected critical services in kernel mode.