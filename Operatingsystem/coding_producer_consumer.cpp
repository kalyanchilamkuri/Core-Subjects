/**
 * ============================================================================
 * OS CODING PROBLEM 2: PRODUCER-CONSUMER (BOUNDED BUFFER) SYNCHRONIZATION
 * ============================================================================
 * 
 * ----------------------------------------------------------------------------
 * 1. THEORY & CONCEPTS
 * ----------------------------------------------------------------------------
 * - WHAT IS THE BOUNDED BUFFER PROBLEM?
 *   In concurrent operating systems, one or more "Producer" tasks produce data
 *   items and place them into a shared, fixed-capacity memory buffer.
 *   Simultaneously, one or more "Consumer" tasks retrieve items from the buffer
 *   and process them.
 * 
 * - THE THREE FUNDAMENTAL SYNCHRONIZATION GOALS:
 *   1. MUTUAL EXCLUSION: Only one thread may modify buffer pointers at any given
 *      instant (prevents race conditions and corruption).
 *   2. PREVENT BUFFER OVERFLOW: If the buffer is full, producers must block/wait
 *      until a consumer removes an item and frees a slot.
 *   3. PREVENT BUFFER UNDERFLOW: If the buffer is empty, consumers must block/wait
 *      until a producer inserts a new item.
 * 
 * - CLASSICAL SEMAPHORE COORDINATION (DIJKSTRA'S APPROACH):
 *   We use three semaphores to coordinate access:
 *   * Semaphore mutex = 1: Binary semaphore for mutual exclusion.
 *   * Semaphore empty = N: Counting semaphore tracking vacant slots.
 *   * Semaphore full = 0:  Counting semaphore tracking filled items.
 * 
 * - THE DEADLOCK TRAP (CRITICAL INTERVIEW QUESTION):
 *   Question: "Why must wait(empty) come BEFORE wait(mutex) in the producer?"
 *   Answer:
 *     If swapped:
 *       wait(mutex); // Producer locks buffer
 *       wait(empty); // Producer waits because buffer is full -> GOES TO SLEEP!
 *     Now, the producer holds 'mutex' while sleeping. The consumer arrives to
 *     consume, calls wait(mutex), and blocks!
 *     Both threads are frozen forever -> DEADLOCK!
 *     RULE: Always acquire resource counting semaphores BEFORE mutual exclusion locks!
 * 
 * ----------------------------------------------------------------------------
 * 2. COMPLETE C++ CODE IMPLEMENTATION
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <vector>
#include <string>

using namespace std;

// ============================================================================
// PART A: SEMAPHORE IMPLEMENTATION (CORE OS PRIMITIVE)
// ============================================================================
class Semaphore {
private:
    int value;

public:
    Semaphore(int initial) : value(initial) {}

    // wait() or P() operation: attempts to acquire resource
    bool wait() {
        if (value <= 0) {
            return false; // In OS, thread is blocked and queued
        }
        value--;
        return true;
    }

    // signal() or V() operation: releases resource
    void signal() {
        value++;
    }

    int getValue() const {
        return value;
    }
};

// ============================================================================
// PART B: BOUNDED CIRCULAR BUFFER
// ============================================================================
class BoundedBuffer {
private:
    vector<int> buffer;
    int capacity;
    int head;  // Insert pointer (producer)
    int tail;  // Remove pointer (consumer)
    int count; // Number of items currently stored

    Semaphore mutexLock; // Binary semaphore (1 = unlocked, 0 = locked)
    Semaphore emptySlots;// Counting semaphore for empty slots
    Semaphore fullSlots; // Counting semaphore for filled items

public:
    BoundedBuffer(int cap)
        : capacity(cap), head(0), tail(0), count(0),
          mutexLock(1), emptySlots(cap), fullSlots(0) {
        buffer.resize(capacity, 0);
    }

    // Producer logic using semaphores
    bool produce(int item, int producerId) {
        // Step 1: Check if empty slot is available
        if (!emptySlots.wait()) {
            cout << "[Producer " << producerId << "] BLOCKED! Buffer is FULL (Capacity " 
                 << capacity << "). Must wait for consumer.\n";
            return false;
        }

        // Step 2: Acquire exclusive mutex lock
        if (!mutexLock.wait()) {
            // Rollback empty slot if mutex contested
            emptySlots.signal();
            return false;
        }

        // --- CRITICAL SECTION ---
        buffer[head] = item;
        head = (head + 1) % capacity; // Circular wrap
        count++;

        cout << "[Producer " << producerId << "] Produced item: " << item 
             << " | Buffer Count: " << count << "/" << capacity << "\n";
        // ------------------------

        // Step 3: Release mutex lock
        mutexLock.signal();

        // Step 4: Signal that an item is now ready
        fullSlots.signal();
        return true;
    }

    // Consumer logic using semaphores
    bool consume(int &outItem, int consumerId) {
        // Step 1: Check if an item is available
        if (!fullSlots.wait()) {
            cout << "  [Consumer " << consumerId << "] BLOCKED! Buffer is EMPTY. Must wait for producer.\n";
            return false;
        }

        // Step 2: Acquire exclusive mutex lock
        if (!mutexLock.wait()) {
            fullSlots.signal();
            return false;
        }

        // --- CRITICAL SECTION ---
        outItem = buffer[tail];
        tail = (tail + 1) % capacity; // Circular wrap
        count--;

        cout << "  [Consumer " << consumerId << "] Consumed item: " << outItem 
             << " | Buffer Count: " << count << "/" << capacity << "\n";
        // ------------------------

        // Step 3: Release mutex lock
        mutexLock.signal();

        // Step 4: Signal that a slot has become free
        emptySlots.signal();
        return true;
    }

    void printBuffer() const {
        cout << "  --> Current Buffer Slots: [ ";
        for (int i = 0; i < capacity; ++i) {
            cout << buffer[i] << " ";
        }
        cout << "]\n";
    }
};

// ============================================================================
// 3. MAIN SIMULATION & INTERLEAVED EXECUTION TEST
// ============================================================================

int main() {
    cout << "=====================================================\n";
    cout << "  PRODUCER-CONSUMER BOUNDED BUFFER IMPLEMENTATION    \n";
    cout << "=====================================================\n";

    BoundedBuffer bb(3); // Fixed capacity = 3

    cout << "\n--- SCENARIO 1: Normal Production ---\n";
    bb.produce(101, 1);
    bb.produce(102, 1);
    bb.produce(103, 1);
    bb.printBuffer();

    cout << "\n--- SCENARIO 2: Buffer Overflow Prevention ---\n";
    cout << "Producer attempts to insert a 4th item into full buffer:\n";
    bb.produce(104, 2); // Must block!

    cout << "\n--- SCENARIO 3: Consumer Frees Space ---\n";
    int item;
    bb.consume(item, 1);
    bb.consume(item, 2);
    bb.printBuffer();

    cout << "\n--- SCENARIO 4: Producer Resumes ---\n";
    cout << "Producer can now safely insert item 104:\n";
    bb.produce(104, 2);
    bb.printBuffer();

    cout << "\n--- SCENARIO 5: Buffer Underflow Prevention ---\n";
    bb.consume(item, 1);
    bb.consume(item, 2);
    bb.printBuffer();
    cout << "Consumer attempts to read from empty buffer:\n";
    bb.consume(item, 3); // Must block!

    return 0;
}

/*
--------------------------------------------------------------------------------
4. STANDARD FAANG / LEETCODE C++11 MULTITHREADED SYNTAX REFERENCE:
--------------------------------------------------------------------------------
For platforms supporting C++11 threads, the standard implementation uses:

#include <mutex>
#include <condition_variable>

class ThreadSafeBoundedBuffer {
    std::mutex mtx;
    std::condition_variable not_full;
    std::condition_variable not_empty;
    std::vector<int> buf;
    int cap, count = 0, head = 0, tail = 0;

public:
    ThreadSafeBoundedBuffer(int c) : cap(c), buf(c) {}

    void produce(int val) {
        std::unique_lock<std::mutex> lock(mtx);
        // Wait while buffer is full (handles spurious wakeups)
        not_full.wait(lock, [this]() { return count < cap; });
        
        buf[head] = val;
        head = (head + 1) % cap;
        count++;

        not_empty.notify_one(); // Wake up one consumer
    }

    int consume() {
        std::unique_lock<std::mutex> lock(mtx);
        // Wait while buffer is empty
        not_empty.wait(lock, [this]() { return count > 0; });

        int val = buf[tail];
        tail = (tail + 1) % cap;
        count--;

        not_full.notify_one(); // Wake up one producer
        return val;
    }
};

--------------------------------------------------------------------------------
5. STEP-BY-STEP DRY RUN TRACE
--------------------------------------------------------------------------------

Initial State:
- Buffer Capacity = 3, count = 0, head = 0, tail = 0.
- emptySlots = 3, fullSlots = 0, mutexLock = 1.

Step 1: Producer 1 calls produce(101)
  - emptySlots.wait(): emptySlots decrements (3 -> 2). (Success)
  - mutexLock.wait(): acquires lock (1 -> 0).
  - buffer[0] = 101, head = 1, count = 1.
  - mutexLock.signal(): releases lock (0 -> 1).
  - fullSlots.signal(): fullSlots increments (0 -> 1).

Step 2: Producer 1 calls produce(102) and produce(103)
  - Successfully inserts into slots 1 and 2.
  - count = 3. emptySlots = 0, fullSlots = 3.

Step 3: Producer 2 calls produce(104)
  - emptySlots.wait() checks: emptySlots is 0!
  - Evaluates to false -> Producer 2 BLOCKED! Buffer overflow prevented!

Step 4: Consumer 1 calls consume()
  - fullSlots.wait(): fullSlots decrements (3 -> 2). (Success)
  - mutexLock.wait(): acquires lock (1 -> 0).
  - item = buffer[0] (101), tail = 1, count = 2.
  - mutexLock.signal(): releases lock (0 -> 1).
  - emptySlots.signal(): emptySlots increments (0 -> 1).
  - Now emptySlots > 0, so blocked Producer 2 is unblocked and can proceed!
*/
