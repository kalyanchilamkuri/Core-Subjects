# The Readers-Writers Problem: Reader Priority, Writer Priority, and Fair Solutions

The **Readers-Writers Problem** is a foundational synchronization challenge that occurs when multiple concurrent threads access a shared dataset (such as a database or file).

---

## 1. The Core Synchronization Rules

| Scenario | Allowed Concurrently? | Rationale |
|---|:---:|---|
| **Multiple Readers** |  **Allowed** | Reading does not modify data; no race conditions occur. |
| **Reader + Writer** | ❌ **Forbidden** | A reader could read partially written, corrupted, or stale data. |
| **Multiple Writers** | ❌ **Forbidden** | Concurrent writes cause race conditions and data corruption. |

---

## 2. Variant 1: First Readers-Writers Problem (Reader Priority)

### Invariant:
* No reader should wait simply because a writer is waiting.
* If at least one reader is currently reading, any newly arriving reader is allowed to begin reading immediately.

### Shared Variables:
```c
semaphore mutex = 1; // Protects the shared integer readCount
semaphore wrt = 1;   // Protects the shared data resource (exclusive access)
int readCount = 0;   // Number of readers currently reading
```

### Algorithm:

#### Reader Process:
```c
void reader() {
    while (true) {
        // --- Entry Section ---
        wait(mutex);
        readCount++;
        if (readCount == 1) {
            wait(wrt); // First reader locks the resource against all writers
        }
        signal(mutex);

        // --- Critical Section: READING ---
        read_shared_database();
        // ---------------------------------

        // --- Exit Section ---
        wait(mutex);
        readCount--;
        if (readCount == 0) {
            signal(wrt); // Last reader releases the resource, allowing writers
        }
        signal(mutex);

        do_other_work();
    }
}
```

#### Writer Process:
```c
void writer() {
    while (true) {
        // --- Entry Section ---
        wait(wrt); // Acquire exclusive lock on the database

        // --- Critical Section: WRITING ---
        write_shared_database();
        // ---------------------------------

        // --- Exit Section ---
        signal(wrt); // Release exclusive lock

        do_other_work();
    }
}
```

### The Starvation Pitfall:
* **Writer Starvation**: If a stream of readers keeps arriving so that `readCount` never drops to 0, waiting writers will starve indefinitely.

---

## 3. Variant 2: Second Readers-Writers Problem (Writer Priority)

### Invariant:
* Once a writer declares an intention to write, **no new incoming readers are permitted to begin reading**.
* Prevents writer starvation by guaranteeing writers get access as soon as currently active readers finish.

### Shared Variables:
```c
semaphore rmutex = 1; // Protects readCount
semaphore wmutex = 1; // Protects writeCount
semaphore readTry = 1;// Allows writers to block incoming readers
semaphore resource = 1;// Exclusive lock on the actual data
int readCount = 0;
int writeCount = 0;
```

### Algorithm:

#### Reader Process:
```c
void reader() {
    while (true) {
        wait(readTry);    // Check if writers have blocked incoming readers
        wait(rmutex);
        readCount++;
        if (readCount == 1)
            wait(resource); // First reader locks the resource
        signal(rmutex);
        signal(readTry);  // Release turnstile for other readers

        // --- READING CRITICAL SECTION ---
        read_data();

        wait(rmutex);
        readCount--;
        if (readCount == 0)
            signal(resource); // Last reader unlocks resource for writers
        signal(rmutex);
    }
}
```

#### Writer Process:
```c
void writer() {
    while (true) {
        wait(wmutex);
        writeCount++;
        if (writeCount == 1)
            wait(readTry); // First writer blocks incoming readers!
        signal(wmutex);

        wait(resource); // Wait until active readers finish and acquire lock
        
        // --- WRITING CRITICAL SECTION ---
        write_data();

        signal(resource);

        wait(wmutex);
        writeCount--;
        if (writeCount == 0)
            signal(readTry); // Last writer re-opens turnstile for readers
        signal(wmutex);
    }
}
```

---

## 4. Modern C++ Standard Solution: `std::shared_mutex`

Modern C++ (C++17 onwards) natively provides read-write locks through `std::shared_mutex`:

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <chrono>

std::shared_mutex rw_lock;
int shared_data = 0;

void reader(int id) {
    // Shared lock allows multiple concurrent readers
    std::shared_lock<std::shared_mutex> lock(rw_lock);
    std::cout << "[Reader " << id << "] Read value: " << shared_data << "\n";
}

void writer(int id, int new_val) {
    // Unique lock guarantees exclusive access (blocks all readers & writers)
    std::unique_lock<std::shared_mutex> lock(rw_lock);
    shared_data = new_val;
    std::cout << "[Writer " << id << "] Updated value to: " << new_val << "\n";
}

int main() {
    std::thread w1(writer, 1, 42);
    std::thread r1(reader, 1);
    std::thread r2(reader, 2);
    std::thread w2(writer, 2, 99);
    std::thread r3(reader, 3);

    w1.join(); r1.join(); r2.join(); w2.join(); r3.join();
    return 0;
}
```

---

## 5. Summary Comparison

| Strategy | Priority Rule | Advantages | Disadvantages |
|---|---|---|---|
| **Reader Priority** | Existing reading blocks writers | Maximum reading throughput | Writers can starve indefinitely |
| **Writer Priority** | Waiting writers block new readers | Timely data freshness | Readers can starve under continuous writes |
| **Fair (FIFO / Turnstile)** | Requests served in arrival order | Zero starvation for either readers or writers | Slightly lower reader concurrency |
| **`std::shared_mutex`** | Implementation-defined fairness | Clean RAII syntax, standard library support | Platform-dependent scheduling details |
