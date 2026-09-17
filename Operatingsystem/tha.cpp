/**
 * ============================================================================
 * OPERATING SYSTEMS & SYSTEM DESIGN - CORE ALGORITHMS & INTERVIEW REFERENCE
 * ============================================================================
 * 
 * This file contains:
 * 1. Practical C++ implementations of core OS algorithms:
 *    - FCFS Scheduling
 *    - Banker's Deadlock Avoidance Algorithm
 *    - LRU (Least Recently Used) Page Replacement Algorithm
 * 2. High-Yield Interview Study Guide on:
 *    - OS Fundamentals & Memory Management
 *    - High-Level & Low-Level System Design (HLD/LLD)
 *    - SOLID Principles & UML Class Relationships
 */

#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <string>

using namespace std;

// ============================================================================
// 1. CPU SCHEDULING: FIRST-COME, FIRST-SERVED (FCFS)
// ============================================================================

struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
};

void runFCFS(vector<Process> &processes) {
    // Sort processes by arrival time
    sort(processes.begin(), processes.end(), [](const Process &a, const Process &b) {
        return a.arrivalTime < b.arrivalTime;
    });

    int currentTime = 0;
    double totalTAT = 0, totalWT = 0;

    cout << "\n--- Running FCFS Scheduling Simulation ---\n";
    for (auto &p : processes) {
        if (currentTime < p.arrivalTime) {
            currentTime = p.arrivalTime; // CPU sits idle until process arrives
        }
        currentTime += p.burstTime;
        p.completionTime = currentTime;
        p.turnaroundTime = p.completionTime - p.arrivalTime;
        p.waitingTime = p.turnaroundTime - p.burstTime;

        totalTAT += p.turnaroundTime;
        totalWT += p.waitingTime;

        cout << "Process P" << p.id 
             << " | CT: " << p.completionTime 
             << " | TAT: " << p.turnaroundTime 
             << " | WT: " << p.waitingTime << "\n";
    }

    cout << "Average Turnaround Time: " << (totalTAT / processes.size()) << "\n";
    cout << "Average Waiting Time:    " << (totalWT / processes.size()) << "\n";
}

// ============================================================================
// 2. DEADLOCK AVOIDANCE: BANKER'S ALGORITHM
// ============================================================================

bool runBankersAlgorithm(int numProcesses, int numResources,
                         const vector<int> &available,
                         const vector<vector<int>> &maxNeed,
                         const vector<vector<int>> &allocation) {
    vector<int> work = available;
    vector<bool> finish(numProcesses, false);
    vector<int> safeSequence;

    // Calculate Need matrix: Need[i][j] = Max[i][j] - Allocation[i][j]
    vector<vector<int>> need(numProcesses, vector<int>(numResources));
    for (int i = 0; i < numProcesses; ++i) {
        for (int j = 0; j < numResources; ++j) {
            need[i][j] = maxNeed[i][j] - allocation[i][j];
        }
    }

    cout << "\n--- Running Banker's Algorithm Safety Check ---\n";

    // Safety check loop
    int count = 0;
    while (count < numProcesses) {
        bool found = false;
        for (int i = 0; i < numProcesses; ++i) {
            if (!finish[i]) {
                bool canAllocate = true;
                for (int j = 0; j < numResources; ++j) {
                    if (need[i][j] > work[j]) {
                        canAllocate = false;
                        break;
                    }
                }

                if (canAllocate) {
                    // Simulate process completion and resource reclamation
                    for (int j = 0; j < numResources; ++j) {
                        work[j] += allocation[i][j];
                    }
                    safeSequence.push_back(i);
                    finish[i] = true;
                    found = true;
                    count++;
                }
            }
        }

        if (!found) {
            cout << "System is in an UNSAFE STATE! (Deadlock risk detected)\n";
            return false;
        }
    }

    cout << "System is in a SAFE STATE! Safe Sequence: < ";
    for (int p : safeSequence) {
        cout << "P" << p << " ";
    }
    cout << ">\n";
    return true;
}

// ============================================================================
// 3. VIRTUAL MEMORY: LRU (LEAST RECENTLY USED) PAGE REPLACEMENT
// ============================================================================

int runLRUPageReplacement(const vector<int> &pages, int frameCapacity) {
    unordered_set<int> currentFrames;
    unordered_map<int, int> lastUsedTime; // page -> last accessed time index
    int pageFaults = 0;

    cout << "\n--- Running LRU Page Replacement Simulation ---\n";
    for (int i = 0; i < (int)pages.size(); ++i) {
        int page = pages[i];

        // If page is not in memory -> Page Fault
        if (currentFrames.find(page) == currentFrames.end()) {
            pageFaults++;
            if ((int)currentFrames.size() == frameCapacity) {
                // Find least recently used page
                int lruPage = -1, oldestTime = i + 1;
                for (int p : currentFrames) {
                    if (lastUsedTime[p] < oldestTime) {
                        oldestTime = lastUsedTime[p];
                        lruPage = p;
                    }
                }
                currentFrames.erase(lruPage);
                cout << "[Step " << i << "] Page Fault: Evicted " << lruPage 
                     << " -> Loaded Page " << page << "\n";
            } else {
                cout << "[Step " << i << "] Page Fault: Loaded Page " << page << "\n";
            }
            currentFrames.insert(page);
        } else {
            cout << "[Step " << i << "] Page Hit: Page " << page << " already in RAM\n";
        }
        lastUsedTime[page] = i; // Update access timestamp
    }

    cout << "Total Page Faults: " << pageFaults << " out of " << pages.size() << " references.\n";
    return pageFaults;
}

// ============================================================================
// 4. MAIN DEMONSTRATION
// ============================================================================

int main() {
    cout << "=========================================================\n";
    cout << "  Operating Systems & System Design Reference Executable\n";
    cout << "=========================================================\n";

    // 1. FCFS Scheduling Demo
    vector<Process> procList = {
        {1, 0, 24, 0, 0, 0},
        {2, 1, 3,  0, 0, 0},
        {3, 2, 3,  0, 0, 0}
    };
    runFCFS(procList);

    // 2. Banker's Algorithm Demo (5 processes, 3 resources)
    int n = 5, m = 3;
    vector<int> available = {3, 3, 2};
    vector<vector<int>> maxNeed = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };
    vector<vector<int>> allocation = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };
    runBankersAlgorithm(n, m, available, maxNeed, allocation);

    // 3. LRU Page Replacement Demo (3 frames)
    vector<int> pageReferences = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    runLRUPageReplacement(pageReferences, 3);

    return 0;
}

/*
================================================================================
                    INTERVIEW STUDY & REVISION REFERENCE
================================================================================

1. CORE OS DEFINITIONS:
- Program: Static binary file stored on secondary disk.
- Process: Program in active execution with isolated virtual address space.
- Thread: Smallest schedulable unit of CPU execution inside a process; shares memory.
- Multitasking: Executing multiple processes concurrently via rapid CPU time-sharing.
- Multithreading: Executing multiple threads within a process to improve concurrency.
- Context Switch: Saving the state of a running process/thread and loading another.
- Memory Management Unit (MMU): Hardware translating virtual addresses to physical RAM.
- Virtual Memory: Gives programs the illusion of large, contiguous memory space.
- Paging: Dividing memory into fixed-size Pages (virtual) and Frames (physical).
- Page Fault: Interrupt raised when an instruction accesses a page not currently in RAM.
- Thrashing: System spending more time swapping pages to/from disk than executing code.

2. SYSTEM DESIGN & ARCHITECTURE (HLD & LLD):
- System Design: Defining the architecture, modules, and interfaces to meet scale.
- High-Level Design (HLD): Macro architecture (Clients -> Load Balancer -> App Servers -> Cache -> DB).
- Low-Level Design (LLD): Micro implementation (Class diagrams, design patterns, schemas).
- Load Balancers:
  * L4: Operates at Transport Layer (TCP/UDP); routes traffic based on IP and Port.
  * L7: Operates at Application Layer; routes based on HTTP URLs, Cookies, Headers.
- Single Point of Failure (SPOF): A component whose outage brings down the entire system;
  mitigated with active-active or active-passive redundancy and automated health checks.

3. SOLID PRINCIPLES:
- S: Single Responsibility Principle (SRP)
     A class should have one, and only one, reason to change.
- O: Open/Closed Principle (OCP)
     Software entities should be open for extension, but closed for modification.
- L: Liskov Substitution Principle (LSP)
     Subtypes must be substitutable for their base types without altering correctness.
- I: Interface Segregation Principle (ISP)
     Clients should not be forced to depend upon methods they do not use.
- D: Dependency Inversion Principle (DIP)
     High-level modules should not depend on low-level modules; both should depend on abstractions.

4. UML RELATIONSHIPS:
- Association: Generic relationship where objects interact (e.g., Teacher and Student).
- Aggregation: "Has-a" relationship where the child can exist independently of parent (weak ownership).
- Composition: "Part-of" relationship where child lifetime is bound to parent (strong ownership).
- Dependency: A temporary "uses" relationship (e.g., method parameter).
================================================================================
*/
