/**
 * ============================================================================
 * OS CODING PROBLEM 1: LRU CACHE (LEAST RECENTLY USED CACHE)
 * ============================================================================
 * 
 * ----------------------------------------------------------------------------
 * 1. THEORY & CONCEPTS
 * ----------------------------------------------------------------------------
 * - WHAT IS AN LRU CACHE?
 *   In Operating Systems, physical RAM is limited. When virtual memory pages
 *   must be loaded into RAM frames and all frames are occupied, the OS must
 *   evict (remove) an existing page.
 *   The LRU policy dictates: "Evict the page that has not been accessed for 
 *   the longest time in the past."
 * 
 * - THE TIME COMPLEXITY REQUIREMENT:
 *   A practical cache must execute both operations in STRICT O(1) time:
 *   1. get(key): Retrieve the value of key if it exists; otherwise -1.
 *   2. put(key, value): Insert or update the value. If capacity is exceeded,
 *      evict the Least Recently Used item.
 * 
 * - WHY WE NEED A DOUBLY LINKED LIST + HASH MAP:
 *   * Hash Map alone: Provides O(1) lookup, but has NO concept of order or recency.
 *   * Array/Vector: Preserves order, but removing an element or moving it
 *     to the front takes O(N) due to shifting.
 *   * Singly Linked List: Moving an item to the front requires finding its
 *     previous node, which takes O(N).
 *   * Doubly Linked List (DLL): Given a pointer to a node, we can remove it 
 *     in O(1) time because we have direct access to node->prev and node->next!
 *   
 *   Therefore:
 *   - Hash Map (unordered_map<int, Node*>): Maps Key -> Node Pointer (O(1) search).
 *   - Doubly Linked List: Maintains recency order.
 *     * Head points to the MOST recently used node.
 *     * Tail points to the LEAST recently used node.
 * 
 * - DUMMY HEAD & DUMMY TAIL SENTINELS:
 *   We use pseudo "dummy" head and tail nodes. This eliminates all annoying
 *   null-pointer checks and edge cases when inserting/deleting at the ends!
 * 
 * ----------------------------------------------------------------------------
 * 2. COMPLETE C++ CODE IMPLEMENTATION
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <unordered_map>

using namespace std;

// Doubly Linked List Node
struct Node {
    int key;
    int value;
    Node *prev;
    Node *next;

    Node(int k, int v) : key(k), value(v), prev(nullptr), next(nullptr) {}
};

class LRUCache {
private:
    int capacity;
    unordered_map<int, Node*> cacheMap; // Key -> Pointer to DLL Node
    Node *head; // Dummy Head: Most Recently Used (MRU) items sit right after head
    Node *tail; // Dummy Tail: Least Recently Used (LRU) items sit right before tail

    // Helper: Add node right after dummy head (mark as Most Recently Used)
    void addNode(Node *node) {
        node->next = head->next;
        node->prev = head;
        head->next->prev = node;
        head->next = node;
    }

    // Helper: Remove an existing node from the Doubly Linked List
    void removeNode(Node *node) {
        Node *prevNode = node->prev;
        Node *nextNode = node->next;
        prevNode->next = nextNode;
        nextNode->prev = prevNode;
    }

    // Helper: Move an accessed node to the front (mark as MRU)
    void moveToHead(Node *node) {
        removeNode(node);
        addNode(node);
    }

    // Helper: Evict and return the least recently used node (before dummy tail)
    Node* popTail() {
        Node *res = tail->prev;
        removeNode(res);
        return res;
    }

public:
    LRUCache(int cap) : capacity(cap) {
        // Initialize dummy head and tail sentinels
        head = new Node(-1, -1);
        tail = new Node(-1, -1);
        head->next = tail;
        tail->prev = head;
    }

    ~LRUCache() {
        // Clean up allocated memory
        Node *curr = head;
        while (curr != nullptr) {
            Node *next = curr->next;
            delete curr;
            curr = next;
        }
    }

    // GET operation: O(1) time
    int get(int key) {
        if (cacheMap.find(key) == cacheMap.end()) {
            return -1; // Cache miss
        }

        // Cache hit: node is accessed -> move to head (MRU)
        Node *node = cacheMap[key];
        moveToHead(node);
        return node->value;
    }

    // PUT operation: O(1) time
    void put(int key, int value) {
        if (cacheMap.find(key) != cacheMap.end()) {
            // Key already exists: update its value and move to MRU head
            Node *node = cacheMap[key];
            node->value = value;
            moveToHead(node);
        } else {
            // Key is new: create a new node
            Node *newNode = new Node(key, value);
            cacheMap[key] = newNode;
            addNode(newNode);

            // If capacity exceeded, evict the LRU node right before tail
            if ((int)cacheMap.size() > capacity) {
                Node *lruNode = popTail();
                cacheMap.erase(lruNode->key); // Remove from hash map
                delete lruNode;               // Free memory
            }
        }
    }

    // Helper method for debugging/printing state
    void printState() {
        cout << "[MRU -> LRU]: ";
        Node *curr = head->next;
        while (curr != tail) {
            cout << "(" << curr->key << ":" << curr->value << ") ";
            curr = curr->next;
        }
        cout << "\n";
    }
};

// ----------------------------------------------------------------------------
// 3. MAIN FUNCTION & VERIFICATION
// ----------------------------------------------------------------------------

int main() {
    cout << "=====================================================\n";
    cout << "             LRU CACHE DEMONSTRATION                 \n";
    cout << "=====================================================\n";

    LRUCache lru(2); // Cache capacity = 2

    cout << "1. put(1, 10)\n";
    lru.put(1, 10);
    lru.printState();

    cout << "\n2. put(2, 20)\n";
    lru.put(2, 20);
    lru.printState();

    cout << "\n3. get(1) -> Returns: " << lru.get(1) << " (Key 1 becomes MRU)\n";
    lru.printState();

    cout << "\n4. put(3, 30) -> Capacity full! Key 2 (LRU) is evicted!\n";
    lru.put(3, 30);
    lru.printState();

    cout << "\n5. get(2) -> Returns: " << lru.get(2) << " (Not found, was evicted)\n";

    cout << "\n6. put(4, 40) -> Capacity full! Key 1 (LRU) is evicted!\n";
    lru.put(4, 40);
    lru.printState();

    cout << "\n7. get(1) -> Returns: " << lru.get(1) << " (Not found)\n";
    cout << "8. get(3) -> Returns: " << lru.get(3) << "\n";
    cout << "9. get(4) -> Returns: " << lru.get(4) << "\n";

    return 0;
}

/*
--------------------------------------------------------------------------------
4. DETAILED CODE EXPLANATION
--------------------------------------------------------------------------------
1. struct Node:
   - Holds key, value, and two pointers: prev and next.
   - Storing the 'key' inside the Node is essential because when we evict a node
     from the tail, we must know its key to erase it from cacheMap in O(1)!

2. Dummy Head and Dummy Tail Sentinels:
   - head->next always points to the Most Recently Used (MRU) item.
   - tail->prev always points to the Least Recently Used (LRU) item.
   - Initial state: head <---> tail.
   - Never worry about null pointer exceptions when updating neighbors!

3. addNode(Node *node):
   - Inserts the node between head and head->next.
   - Updates 4 pointers in strict order:
     node->next = head->next;
     node->prev = head;
     head->next->prev = node;
     head->next = node;

4. removeNode(Node *node):
   - Unlinks the node by connecting node->prev directly to node->next:
     prevNode->next = nextNode;
     nextNode->prev = prevNode;

5. get(key):
   - Look up key in cacheMap. If not found, return -1.
   - If found, call moveToHead(node) to mark it as the newest item, then return value.

6. put(key, value):
   - If key exists: update value and call moveToHead(node).
   - If key is new: create node, add to hash map, insert at head via addNode().
   - If size > capacity: call popTail(), erase its key from cacheMap, and delete it.

--------------------------------------------------------------------------------
5. STEP-BY-STEP DRY RUN TRACE
--------------------------------------------------------------------------------

Consider Cache with Capacity = 2:

Step 1: put(1, 10)
  - Hash Map: {1: Node(1, 10)}
  - DLL: [Head] <-> (1:10) <-> [Tail]
  - Size = 1 <= 2. No eviction.

Step 2: put(2, 20)
  - Node(2, 20) added at head.
  - Hash Map: {1: Node(1, 10), 2: Node(2, 20)}
  - DLL: [Head] <-> (2:20) <-> (1:10) <-> [Tail]
  - Size = 2 <= 2. No eviction.

Step 3: get(1)
  - Key 1 is found!
  - moveToHead(Node 1) is called:
  - DLL: [Head] <-> (1:10) <-> (2:20) <-> [Tail]
  - Result: 10. Node 1 is now MRU; Node 2 is now LRU!

Step 4: put(3, 30)
  - Node(3, 30) added at head:
    DLL: [Head] <-> (3:30) <-> (1:10) <-> (2:20) <-> [Tail]
  - Size = 3 > Capacity (2)!
  - popTail() removes the node before Tail -> Node(2, 20) evicted!
  - Erase 2 from Hash Map.
  - Hash Map: {1: Node(1, 10), 3: Node(3, 30)}
  - DLL: [Head] <-> (3:30) <-> (1:10) <-> [Tail]

Step 5: get(2)
  - Key 2 is searched in Hash Map. Not found!
  - Result: -1 (Correctly evicted).

Step 6: put(4, 40)
  - Node(4, 40) added at head:
    DLL: [Head] <-> (4:40) <-> (3:30) <-> (1:10) <-> [Tail]
  - Size = 3 > 2!
  - popTail() removes Node(1, 10)!
  - Erase 1 from Hash Map.
  - Hash Map: {3: Node(3, 30), 4: Node(4, 40)}
  - DLL: [Head] <-> (4:40) <-> (3:30) <-> [Tail]
*/
