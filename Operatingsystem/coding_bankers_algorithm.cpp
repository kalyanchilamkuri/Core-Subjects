/**
 * ============================================================================
 * OS CODING PROBLEM 3: BANKER'S ALGORITHM (DEADLOCK AVOIDANCE)
 * ============================================================================
 * 
 * ----------------------------------------------------------------------------
 * 1. THEORY & CONCEPTS
 * ----------------------------------------------------------------------------
 * - WHAT IS DEADLOCK AVOIDANCE?
 *   Unlike Deadlock Prevention (which restricts how resources can be requested),
 *   Deadlock Avoidance dynamically monitors every resource request at runtime.
 *   The operating system simulates what the state of the system would be IF the
 *   request were granted. If the resulting state is guaranteed to be "Safe",
 *   the request is approved. If not, the process must wait.
 * 
 * - SAFE STATE VS. UNSAFE STATE:
 *   * Safe State: A state is safe if there exists at least one "Safe Sequence"
 *     <P0, P1, ..., Pn-1> such that every process can eventually obtain its
 *     maximum needed resources, finish execution, and return its resources.
 *   * Unsafe State: A state where no safe sequence exists.
 *     (Unsafe does NOT mean deadlock has occurred immediately, but the OS
 *     can no longer prevent processes from potentially requesting maximum
 *     resources and triggering a deadlock).
 * 
 * - THE MATHEMATICAL DATA STRUCTURES:
 *   Let N = Number of processes, M = Number of resource types (A, B, C...).
 *   1. Available[M]: Number of available instances of each resource type.
 *   2. Max[N][M]: Maximum demand of each process during its lifetime.
 *   3. Allocation[N][M]: Resources currently allocated to each process.
 *   4. Need[N][M]: Remaining resources each process will need:
 *      Need[i][j] = Max[i][j] - Allocation[i][j]
 * 
 * - THE RESOURCE REQUEST ALGORITHM:
 *   When process P_i requests Request[M] resources:
 *   Step 1: If Request[j] > Need[i][j] -> Error (exceeded declared claim).
 *   Step 2: If Request[j] > Available[j] -> Process must wait (not enough free).
 *   Step 3: Pretend to allocate:
 *           Available -= Request
 *           Allocation[i] += Request
 *           Need[i] -= Request
 *   Step 4: Run Safety Algorithm:
 *           - If Safe -> Permanently grant the request.
 *           - If Unsafe -> Rollback changes; Process P_i must wait!
 * 
 * ----------------------------------------------------------------------------
 * 2. COMPLETE C++ CODE IMPLEMENTATION
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <vector>

using namespace std;

class BankersAlgorithm {
private:
    int numProcesses;
    int numResources;
    vector<int> available;
    vector<vector<int>> maxMatrix;
    vector<vector<int>> allocation;
    vector<vector<int>> need;

    // Helper: Check if vector A <= vector B element-wise
    bool isLessOrEqual(const vector<int> &a, const vector<int> &b) {
        for (int j = 0; j < numResources; ++j) {
            if (a[j] > b[j]) return false;
        }
        return true;
    }

public:
    BankersAlgorithm(int p, int r,
                     const vector<int> &avail,
                     const vector<vector<int>> &maxM,
                     const vector<vector<int>> &alloc)
        : numProcesses(p), numResources(r), available(avail), maxMatrix(maxM), allocation(alloc) {
        
        // Calculate Need Matrix: Need = Max - Allocation
        need.resize(numProcesses, vector<int>(numResources));
        for (int i = 0; i < numProcesses; ++i) {
            for (int j = 0; j < numResources; ++j) {
                need[i][j] = maxMatrix[i][j] - allocation[i][j];
            }
        }
    }

    // Safety Algorithm: Checks if current state is safe and outputs sequence
    bool isSafe(vector<int> &safeSequence) {
        safeSequence.clear();
        vector<int> work = available;
        vector<bool> finish(numProcesses, false);

        int count = 0;
        while (count < numProcesses) {
            bool found = false;

            for (int i = 0; i < numProcesses; ++i) {
                if (!finish[i] && isLessOrEqual(need[i], work)) {
                    // Process i can complete! Reclaim its allocated resources:
                    for (int j = 0; j < numResources; ++j) {
                        work[j] += allocation[i][j];
                    }
                    finish[i] = true;
                    safeSequence.push_back(i);
                    found = true;
                    count++;
                }
            }

            // If in an entire pass no process could be satisfied -> Unsafe!
            if (!found) {
                return false;
            }
        }
        return true;
    }

    // Resource Request Routine
    bool requestResources(int pid, const vector<int> &request) {
        cout << "\n>>> Process P" << pid << " requests: [ ";
        for (int val : request) cout << val << " ";
        cout << "]\n";

        // Check 1: Does request exceed declared maximum need?
        for (int j = 0; j < numResources; ++j) {
            if (request[j] > need[pid][j]) {
                cout << "[ERROR] Process P" << pid 
                     << " exceeded its declared maximum claim! Request denied.\n";
                return false;
            }
        }

        // Check 2: Are requested resources currently available?
        for (int j = 0; j < numResources; ++j) {
            if (request[j] > available[j]) {
                cout << "[WAIT] Insufficient resources currently available. Process P" 
                     << pid << " must wait.\n";
                return false;
            }
        }

        // Step 3: Pretend to allocate resources (trial allocation)
        for (int j = 0; j < numResources; ++j) {
            available[j] -= request[j];
            allocation[pid][j] += request[j];
            need[pid][j] -= request[j];
        }

        // Step 4: Run safety algorithm on the trial state
        vector<int> safeSeq;
        if (isSafe(safeSeq)) {
            cout << "[SUCCESS] Request granted safely! New Safe Sequence: < ";
            for (int p : safeSeq) cout << "P" << p << " ";
            cout << ">\n";
            return true;
        } else {
            cout << "[DENIED] Granting request would lead to an UNSAFE STATE! Rolling back...\n";
            // Rollback changes
            for (int j = 0; j < numResources; ++j) {
                available[j] += request[j];
                allocation[pid][j] -= request[j];
                need[pid][j] += request[j];
            }
            return false;
        }
    }

    void printState() {
        cout << "\n--- CURRENT SYSTEM STATE ---\n";
        cout << "Process | Allocation | Max Claim  | Need (Max-Alloc)\n";
        for (int i = 0; i < numProcesses; ++i) {
            cout << "  P" << i << "    | ";
            for (int j = 0; j < numResources; ++j) cout << allocation[i][j] << " ";
            cout << "     | ";
            for (int j = 0; j < numResources; ++j) cout << maxMatrix[i][j] << " ";
            cout << "   | ";
            for (int j = 0; j < numResources; ++j) cout << need[i][j] << " ";
            cout << "\n";
        }
        cout << "Available Resources: [ ";
        for (int val : available) cout << val << " ";
        cout << "]\n";
    }
};

// ----------------------------------------------------------------------------
// 3. MAIN SIMULATION
// ----------------------------------------------------------------------------

int main() {
    cout << "=====================================================\n";
    cout << "           BANKER'S ALGORITHM DEMONSTRATION          \n";
    cout << "=====================================================\n";

    int numP = 5; // P0, P1, P2, P3, P4
    int numR = 3; // Resources A, B, C

    vector<int> available = {3, 3, 2};

    vector<vector<int>> maxMatrix = {
        {7, 5, 3}, // P0
        {3, 2, 2}, // P1
        {9, 0, 2}, // P2
        {2, 2, 2}, // P3
        {4, 3, 3}  // P4
    };

    vector<vector<int>> allocation = {
        {0, 1, 0}, // P0
        {2, 0, 0}, // P1
        {3, 0, 2}, // P2
        {2, 1, 1}, // P3
        {0, 0, 2}  // P4
    };

    BankersAlgorithm banker(numP, numR, available, maxMatrix, allocation);
    banker.printState();

    // 1. Initial Safety Check
    vector<int> initialSeq;
    if (banker.isSafe(initialSeq)) {
        cout << "\nInitial State is SAFE! Safe Sequence: < ";
        for (int p : initialSeq) cout << "P" << p << " ";
        cout << ">\n";
    } else {
        cout << "\nInitial State is UNSAFE!\n";
    }

    // 2. Test Safe Request: P1 requests [1, 0, 2]
    banker.requestResources(1, {1, 0, 2});

    // 3. Test Denied/Unsafe Request: P4 requests [3, 3, 0] (more than available)
    banker.requestResources(4, {3, 3, 0});

    return 0;
}

/*
--------------------------------------------------------------------------------
4. DETAILED CODE EXPLANATION
--------------------------------------------------------------------------------
1. Initialization:
   - Need Matrix is automatically derived upon construction:
     Need[i][j] = Max[i][j] - Allocation[i][j].
   - If a process asks for resources beyond its need, it is flagged as an error.

2. Safety Check (isSafe):
   - Vector 'work' is initialized with 'available'.
   - Vector 'finish' keeps track of processes simulated to completion.
   - The loop continually scans for an unfinished process whose need can be
     satisfied by available work (need[i] <= work).
   - Once found, we pretend it executes and terminates, adding its allocation
     back to work: work += allocation[i].
   - If all processes complete, a Safe Sequence exists!

3. Resource Request (requestResources):
   - Validates that Request <= Need (avoids illegal requests).
   - Validates that Request <= Available (avoids over-allocation).
   - Trial allocation: Temporarily modifies Available, Allocation, and Need.
   - Calls isSafe():
     * If true: State remains safe -> Keep allocation permanently.
     * If false: State becomes unsafe -> Rollback changes and return false.

--------------------------------------------------------------------------------
5. STEP-BY-STEP DRY RUN TRACE
--------------------------------------------------------------------------------

Initial Data:
Processes: P0, P1, P2, P3, P4
Resources: A=10, B=5, C=7
Total Allocated: A=7, B=2, C=5 -> Available = (3, 3, 2).

Need Matrix Calculation:
P0: (7, 5, 3) - (0, 1, 0) = (7, 4, 3)
P1: (3, 2, 2) - (2, 0, 0) = (1, 2, 2)
P2: (9, 0, 2) - (3, 0, 2) = (6, 0, 0)
P3: (2, 2, 2) - (2, 1, 1) = (0, 1, 1)
P4: (4, 3, 3) - (0, 0, 2) = (4, 3, 1)

Safety Algorithm Execution:
- Work = (3, 3, 2), Finish = [F, F, F, F, F]

Pass 1:
- P0: Need (7, 4, 3) <= Work (3, 3, 2)? NO.
- P1: Need (1, 2, 2) <= Work (3, 3, 2)? YES!
  P1 finishes: Work = (3, 3, 2) + Alloc(2, 0, 0) = (5, 3, 2).
  Finish[1] = TRUE. Safe Seq = < P1 >.
- P2: Need (6, 0, 0) <= Work (5, 3, 2)? NO.
- P3: Need (0, 1, 1) <= Work (5, 3, 2)? YES!
  P3 finishes: Work = (5, 3, 2) + Alloc(2, 1, 1) = (7, 4, 3).
  Finish[3] = TRUE. Safe Seq = < P1, P3 >.
- P4: Need (4, 3, 1) <= Work (7, 4, 3)? YES!
  P4 finishes: Work = (7, 4, 3) + Alloc(0, 0, 2) = (7, 4, 5).
  Finish[4] = TRUE. Safe Seq = < P1, P3, P4 >.

Pass 2:
- P0: Need (7, 4, 3) <= Work (7, 4, 5)? YES!
  P0 finishes: Work = (7, 4, 5) + Alloc(0, 1, 0) = (7, 5, 5).
  Finish[0] = TRUE. Safe Seq = < P1, P3, P4, P0 >.
- P2: Need (6, 0, 0) <= Work (7, 5, 5)? YES!
  P2 finishes: Work = (7, 5, 5) + Alloc(3, 0, 2) = (10, 5, 7).
  Finish[2] = TRUE. Safe Seq = < P1, P3, P4, P0, P2 >.

All finish[i] == TRUE.
Final Safe Sequence: < P1, P3, P4, P0, P2 >. State is SAFE!
*/
