# The Producer-Consumer Problem (Bounded Buffer)

The **Producer-Consumer Problem** (also called the **Bounded-Buffer Problem**) is a classic multi-process synchronization problem that models how threads share a fixed-size queue or buffer.

---

## 1. Problem Architecture

```
                    Fixed-Size Shared Buffer (Size = N)
              +---+---+---+---+---+---+---+---+
[ Producer ] ──►| 1 | 2 | 3 |   |   |   |   |   |──► [ Consumer ]
              +---+---+---+---+---+---+---+---+
                Occupied Slots: full = 3
                Available Slots: empty = N - 3
```

* **The Producer**: Generates data items and attempts to insert them into the buffer.
* **The Consumer**: Removes data items from the buffer and processes them.
* **The Shared Buffer**: A finite storage queue of capacity $N$.

### The Three Synchronization Challenges:
1. **Prevent Overflow**: The producer must not attempt to add an item if the buffer is completely full ($empty = 0$).
2. **Prevent Underflow**: The consumer must not attempt to remove an item if the buffer is completely empty ($full = 0$).
3. **Mutual Exclusion**: Only one thread (either one producer or one consumer) may manipulate the buffer pointers at any given instant to prevent race conditions.

---

## 2. Semaphore-Based Solution

We use three semaphores to coordinate access:
1. **`semaphore mutex = 1`**: A binary semaphore providing mutual exclusion over buffer inserts/removals.
2. **`semaphore empty = N`**: A counting semaphore tracking the count of **vacant slots** in the buffer.
3. **`semaphore full = 0`**: A counting semaphore tracking the count of **occupied slots** in the buffer.

---

### Producer Algorithm
```c
do {
    // 1. Produce an item locally
    item = produce_item();

    // 2. Wait until at least one slot is empty
    wait(empty);

    // 3. Acquire exclusive lock on the buffer
    wait(mutex);

    // --- CRITICAL SECTION ---
    insert_into_buffer(item);
    // ------------------------

    // 4. Release lock on the buffer
    signal(mutex);

    // 5. Notify consumer that a new item is available
    signal(full);

} while (true);
```

---

### Consumer Algorithm
```c
do {
    // 1. Wait until at least one item is present
    wait(full);

    // 2. Acquire exclusive lock on the buffer
    wait(mutex);

    // --- CRITICAL SECTION ---
    item = remove_from_buffer();
    // ------------------------

    // 3. Release lock on the buffer
    signal(mutex);

    // 4. Notify producer that a slot has become free
    signal(empty);

    // 5. Consume the item locally
    consume_item(item);

} while (true);
```

---

## 3. Top Interview Question: Why Does Semaphore Order Matter?

*What happens if you accidentally swap the order of the `wait()` calls in the Producer?*

```c
// DANGEROUS DEADLOCK-PRONE CODE:
wait(mutex);  // Swapped!
wait(empty);  // Swapped!
```

### The Deadlock Scenario:
1. Suppose the buffer is completely full ($empty = 0$).
2. The producer executes `wait(mutex)` and successfully acquires the lock.
3. The producer then executes `wait(empty)`. Since $empty = 0$, the producer **blocks and goes to sleep while still holding the mutex lock**!
4. The consumer arrives to remove an item. It attempts to execute `wait(mutex)`, but the mutex is held by the sleeping producer!
5. **Deadlock**: The consumer cannot free space because it cannot get the mutex; the producer cannot release the mutex because it is waiting for the consumer to free space. Both threads freeze forever.
* **Rule**: Always acquire resource counting semaphores (`empty`/`full`) **before** acquiring mutual exclusion locks (`mutex`).

---

## 4. Complete C++ Implementation with Condition Variables

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

template <typename T>
class BoundedBuffer {
private:
    std::queue<T> buffer;
    size_t capacity;
    std::mutex mtx;
    std::condition_variable not_full;
    std::condition_variable not_empty;

public:
    BoundedBuffer(size_t cap) : capacity(cap) {}

    void produce(const T &item) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait while buffer is full
        not_full.wait(lock, [this]() { return buffer.size() < capacity; });

        buffer.push(item);
        std::cout << "[Producer] Inserted item: " << item << " | Buffer size: " << buffer.size() << "\n";

        // Signal waiting consumers
        not_empty.notify_one();
    }

    T consume() {
        std::unique_lock<std::mutex> lock(mtx);

        // Wait while buffer is empty
        not_empty.wait(lock, [this]() { return !buffer.empty(); });

        T item = buffer.front();
        buffer.pop();
        std::cout << "[Consumer] Removed item: " << item << " | Buffer size: " << buffer.size() << "\n";

        // Signal waiting producers
        not_full.notify_one();
        return item;
    }
};

int main() {
    BoundedBuffer<int> bb(5); // Buffer capacity = 5

    std::thread producer([&]() {
        for (int i = 1; i <= 10; ++i) {
            bb.produce(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });

    std::thread consumer([&]() {
        for (int i = 1; i <= 10; ++i) {
            bb.consume();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    producer.join();
    consumer.join();
    return 0;
}
```
