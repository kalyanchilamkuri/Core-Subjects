# Synchronization Primitives: Condition Variables and Semaphores

---

## 1. Condition Variables

A **Condition Variable** is a synchronization primitive that enables a thread to suspend execution (**sleep without consuming CPU cycles**) until a specific application condition evaluates to true, signaled by another thread.

### Why Must Condition Variables Be Used With a Mutex?
1. The shared variable being tested (e.g., `queue.empty()` or `bool ready`) is accessed by multiple threads and requires mutual exclusion.
2. The `wait()` operation **atomically** releases the acquired mutex lock and places the calling thread into a sleep queue.
3. When the thread is signaled and awakened, it **atomically re-acquires the mutex lock** before `wait()` returns.

```
Waiting Thread (Consumer):
1. Acquire Mutex Lock.
2. Check Condition. If false:
   -> Call cv.wait(lock).
   -> Atomically releases lock and sleeps.
   -> [Woken up by notify_one()]
   -> Reacquires lock automatically.
3. Execute critical work with lock held.
4. Release Mutex Lock.
```

### The Spurious Wakeup Problem:
In multithreaded systems, an operating system thread may occasionally wake up even if no explicit signal was sent (known as a **spurious wakeup**), or another thread may consume the resource before the newly awakened thread runs.  
**Rule**: Always wrap `wait()` in a **`while` loop** or pass a predicate lambda, never an `if` statement:

```cpp
// Correct:
while (!ready) {
    cv.wait(lock);
}

// Or using standard C++ predicate:
cv.wait(lock, [] { return ready; });
```

### Complete Modern C++ Example:
```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void worker() {
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "[Worker] Waiting for data to be ready...\n";
    
    // Atomically releases lock and sleeps until ready == true
    cv.wait(lock, [] { return ready; });
    
    std::cout << "[Worker] Condition met! Processing data now...\n";
}

void sender() {
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate preparation work
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
        std::cout << "[Sender] Data prepared! Notifying worker...\n";
    }
    
    cv.notify_one(); // Wake up one waiting thread
}

int main() {
    std::thread t1(worker);
    std::thread t2(sender);

    t1.join();
    t2.join();
    return 0;
}
```

---

## 2. Semaphores: The Classical Signaling Primitive

Introduced by Edsger Dijkstra, a **Semaphore** is an integer variable that, apart from initialization, is accessed only through two standard atomic operations:

1. **`wait(S)`** (also historically termed **$P(S)$**, from Dutch *proberen* "to test"):
   * Decrements the semaphore value $S$.
   * If $S \le 0$, the calling thread is blocked and placed on the waiting queue.
2. **`signal(S)`** (also historically termed **$V(S)$**, from Dutch *verhogen* "to increment"):
   * Increments the semaphore value $S$.
   * If any threads are blocked on the waiting queue, one thread is removed from the wait queue and placed onto the Ready Queue.

---

### Non-Busy Waiting (OS-Level Implementation)
To eliminate wasteful spinlocks (busy-waiting), modern operating systems implement semaphores using block/wakeup routines:

```c
struct semaphore {
    int value;
    struct process_node *wait_queue; // Linked list of blocked processes
};

void wait(semaphore *S) {
    S->value--;
    if (S->value < 0) {
        // Add this process to S->wait_queue
        block(); // Changes state to WAITING and yields CPU
    }
}

void signal(semaphore *S) {
    S->value++;
    if (S->value <= 0) {
        // Remove process P from S->wait_queue
        wakeup(P); // Moves process P to READY queue
    }
}
```

---

## 3. Types of Semaphores

### A. Binary Semaphore (Value can only be 0 or 1)
* Initialized to `1`.
* Functions identically to a mutual exclusion lock, restricting access to one process at a time.

### B. Counting Semaphore (Value can be any non-negative integer $N$)
* Initialized to $N$, representing $N$ available instances of a finite resource (e.g., 3 database connections or 2 printer ports).
* Up to $N$ threads can enter simultaneously before incoming threads block.

### C++ Counting Semaphore Implementation:
```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

class CountingSemaphore {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;

public:
    CountingSemaphore(int initial_value) : count(initial_value) {}

    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return count > 0; });
        count--;
    }

    void signal() {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }
};

CountingSemaphore sem(2); // Allow at most 2 concurrent threads

void worker(int id) {
    sem.wait();
    std::cout << "Thread " << id << " acquired resource (Active)\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "Thread " << id << " releasing resource (Done)\n";
    sem.signal();
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 1; i <= 5; ++i) {
        threads.emplace_back(worker, i);
    }
    for (auto &t : threads) t.join();
    return 0;
}
```

---

## 4. Mutex vs. Semaphore vs. Condition Variable

| Primitive | Category | Key Purpose | Can Multiple Enter? | Ownership Rule |
|---|---|---|:---:|---|
| **Mutex** | Locking | Protect a shared critical resource | ❌ No (strictly 1) | **Strictly Owned**: Only the thread that locked the mutex can unlock it. |
| **Counting Semaphore** | Signaling | Manage access to $N$ finite resources |  Yes (up to $N$) | **No Ownership**: Any thread can call `signal()` to wake waiting threads. |
| **Condition Variable** | Notification | Sleep until an application condition becomes true | Dependent on predicate | Always paired with an underlying Mutex lock. |
