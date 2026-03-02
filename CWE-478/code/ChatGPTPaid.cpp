#include <iostream>
#include <random>
#include <ctime>
#include <stdexcept>

struct Node {
    int value;
    Node* next;
    Node(int v, Node* n = nullptr) : value(v), next(n) {}
};

// ---------- Linked-list helpers (raw pointers only) ----------

static int length(Node* head) {
    int n = 0;
    while (head) {
        ++n;
        head = head->next;
    }
    return n;
}

// Return pointer to node at index (0-based). Assumes index is valid.
static Node* node_at(Node* head, int index) {
    while (index-- > 0) head = head->next;
    return head;
}

static void push_front(Node*& head, int value) {
    head = new Node(value, head);
}

// Insert new node with `value` at position `pos` (0..len). pos==0 inserts at front.
static void insert_at(Node*& head, int pos, int value) {
    if (pos <= 0 || head == nullptr) {
        push_front(head, value);
        return;
    }
    Node* prev = node_at(head, pos - 1);
    prev->next = new Node(value, prev->next);
}

// Delete node at position `pos` (0..len-1). Assumes list non-empty and pos valid.
static void delete_at(Node*& head, int pos) {
    if (!head) return;
    if (pos == 0) {
        Node* doomed = head;
        head = head->next;
        delete doomed;
        return;
    }
    Node* prev = node_at(head, pos - 1);
    Node* doomed = prev->next;
    prev->next = doomed->next;
    delete doomed;
}

// Detach node at `pos` and return it (node->next set to nullptr). Assumes valid.
static Node* detach_at(Node*& head, int pos) {
    if (!head) return nullptr;
    if (pos == 0) {
        Node* n = head;
        head = head->next;
        n->next = nullptr;
        return n;
    }
    Node* prev = node_at(head, pos - 1);
    Node* n = prev->next;
    prev->next = n->next;
    n->next = nullptr;
    return n;
}

// Insert an *existing* node (already allocated) at position `pos` (0..len).
static void insert_node_at(Node*& head, int pos, Node* n) {
    if (!n) return;
    if (pos <= 0 || head == nullptr) {
        n->next = head;
        head = n;
        return;
    }
    Node* prev = node_at(head, pos - 1);
    n->next = prev->next;
    prev->next = n;
}

static void free_list(Node*& head) {
    while (head) {
        Node* next = head->next;
        delete head;
        head = next;
    }
}

static void print_list(Node* head, int maxItems = 30) {
    std::cout << "[";
    int i = 0;
    while (head && i < maxItems) {
        std::cout << head->value;
        head = head->next;
        ++i;
        if (head && i < maxItems) std::cout << " -> ";
    }
    if (head) std::cout << " -> ...";
    std::cout << "]";
}

// ---------- Random utilities (no STL containers used) ----------

static int rand_int(std::mt19937& rng, int lo, int hi) {
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(rng);
}

enum Op {
    OP_ADD_FRONT = 1,    // (1) add new integer into the linked list (front)
    OP_DELETE_RANDOM,    // (2) delete a random item
    OP_INSERT_RANDOM,    // (3) add a new integer somewhere random
    OP_MOVE_NODE         // (4) move an integer from one area to another
};

int main() {
    // Seed RNG
    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));

    Node* head = nullptr;

    // Keep doing random operations until list has exactly 100 nodes.
    // We avoid operations that would make the size exceed 100.
    const int TARGET = 100;

    int steps = 0;

    while (true) {
        int len = length(head);
        if (len == TARGET) break;

        // Choose an operation that is valid for current length and won't exceed 100.
        Op op;

        if (len == 0) {
            // Only possible: add/insert (front or random insert).
            op = (rand_int(rng, 0, 1) == 0) ? OP_ADD_FRONT : OP_INSERT_RANDOM;
        } else if (len == TARGET) {
            break;
        } else {
            // Weighted random selection.
            // If len < TARGET, allow adds/inserts/moves/deletes.
            // If len == TARGET-0? still fine. If len == TARGET-? only avoid ops that increase size.
            int roll = rand_int(rng, 1, 100);

            bool canGrow = (len < TARGET);
            if (!canGrow) {
                // Must not grow: only delete or move.
                op = (rand_int(rng, 0, 1) == 0) ? OP_DELETE_RANDOM : OP_MOVE_NODE;
            } else {
                // Typical weights: add/insert (45%), delete (25%), move (30%)
                if (roll <= 20) op = OP_ADD_FRONT;          // 20%
                else if (roll <= 45) op = OP_INSERT_RANDOM; // 25%
                else if (roll <= 70) op = OP_MOVE_NODE;     // 25%
                else op = OP_DELETE_RANDOM;                 // 30%
            }
        }

        // Execute chosen operation
        len = length(head); // refresh
        switch (op) {
            case OP_ADD_FRONT: {
                if (len >= TARGET) break; // safety
                int value = rand_int(rng, -100000, 100000);
                push_front(head, value);
                ++steps;
                break;
            }

            case OP_DELETE_RANDOM: {
                if (len <= 0) break;
                int pos = rand_int(rng, 0, len - 1);
                delete_at(head, pos);
                ++steps;
                break;
            }

            case OP_INSERT_RANDOM: {
                if (len >= TARGET) break;
                int value = rand_int(rng, -100000, 100000);
                // insertion position can be 0..len (len means append)
                int pos = rand_int(rng, 0, len);
                insert_at(head, pos, value);
                ++steps;
                break;
            }

            case OP_MOVE_NODE: {
                if (len <= 1) break; // moving within 0/1 node list is pointless
                // pick a source index
                int from = rand_int(rng, 0, len - 1);
                Node* moved = detach_at(head, from);

                // after detach, new length is len-1, destination is 0..(len-1)
                int newLen = len - 1;
                int to = rand_int(rng, 0, newLen); // allow re-inserting at end
                insert_node_at(head, to, moved);
                ++steps;
                break;
            }

            default:
                break;
        }

        // Optional: print progress occasionally
        int newLen = length(head);
        if (steps % 50 == 0 || newLen == TARGET) {
            std::cout << "Step " << steps << " | len=" << newLen << " | sample ";
            print_list(head, 20);
            std::cout << "\n";
        }

        // Safety to prevent infinite loops (shouldn't happen)
        if (steps > 200000) {
            std::cerr << "Too many steps; stopping to avoid infinite loop.\n";
            break;
        }
    }

    std::cout << "\nDone.\nFinal length = " << length(head) << "\nFinal sample: ";
    print_list(head, 40);
    std::cout << "\n";

    free_list(head);
    return 0;
}