#include <iostream>
#include <cstdlib>
#include <ctime>

struct Node {
    int value;
    Node* next;
};

// ---------- Utility: basic list operations (raw pointers only) ----------

static int listLength(Node* head) {
    int n = 0;
    for (Node* cur = head; cur != nullptr; cur = cur->next) ++n;
    return n;
}

static Node* nodeAt(Node* head, int index) {
    // Precondition: 0 <= index < length
    Node* cur = head;
    for (int i = 0; i < index; ++i) cur = cur->next;
    return cur;
}

static void pushFront(Node*& head, int value) {
    Node* n = new Node{value, head};
    head = n;
}

static void insertAt(Node*& head, int index, int value) {
    // Inserts BEFORE position index, where index is in [0, length]
    // index == 0 => push front
    if (index <= 0 || head == nullptr) {
        pushFront(head, value);
        return;
    }

    // Find node at index-1, then splice
    Node* prev = nodeAt(head, index - 1);
    Node* n = new Node{value, prev->next};
    prev->next = n;
}

static void deleteAt(Node*& head, int index) {
    // Deletes node at index, where index is in [0, length-1]
    if (head == nullptr) return;

    if (index <= 0) {
        Node* doomed = head;
        head = head->next;
        delete doomed;
        return;
    }

    Node* prev = nodeAt(head, index - 1);
    Node* doomed = prev->next;
    if (doomed == nullptr) return; // safety
    prev->next = doomed->next;
    delete doomed;
}

static void moveNode(Node*& head, int fromIndex, int toIndex) {
    // Removes node at fromIndex and reinserts it so it becomes
    // the node BEFORE the current toIndex position in the *resulting* list.
    // Indices are interpreted as in a typical "erase then insert" operation.

    int len = listLength(head);
    if (len <= 1) return;
    if (fromIndex < 0) fromIndex = 0;
    if (fromIndex >= len) fromIndex = len - 1;
    if (toIndex < 0) toIndex = 0;
    if (toIndex > len) toIndex = len; // insertion can be at end

    if (fromIndex == toIndex || fromIndex + 1 == toIndex) {
        // Moving to same spot (or right after itself) is effectively no-op
        return;
    }

    // 1) Detach the node at fromIndex
    Node* moving = nullptr;

    if (fromIndex == 0) {
        moving = head;
        head = head->next;
    } else {
        Node* prevFrom = nodeAt(head, fromIndex - 1);
        moving = prevFrom->next;
        prevFrom->next = moving->next;
    }
    moving->next = nullptr;

    // After removal, list length is len-1.
    // If we removed an earlier index, the destination shifts left by 1.
    if (fromIndex < toIndex) toIndex -= 1;

    // 2) Insert moving at toIndex in the shortened list
    if (toIndex <= 0 || head == nullptr) {
        moving->next = head;
        head = moving;
        return;
    }

    int newLen = len - 1;
    if (toIndex >= newLen) {
        // Insert at end
        Node* tail = nodeAt(head, newLen - 1);
        tail->next = moving;
        moving->next = nullptr;
        return;
    }

    Node* prevTo = nodeAt(head, toIndex - 1);
    moving->next = prevTo->next;
    prevTo->next = moving;
}

static void clearList(Node*& head) {
    while (head != nullptr) {
        Node* doomed = head;
        head = head->next;
        delete doomed;
    }
}

static void printList(Node* head) {
    std::cout << "[";
    for (Node* cur = head; cur != nullptr; cur = cur->next) {
        std::cout << cur->value;
        if (cur->next) std::cout << " -> ";
    }
    std::cout << "]\n";
}

// ---------- Random helpers (no STL containers) ----------

static int randInt(int lo, int hi) {
    // inclusive range; assumes lo <= hi
    return lo + (std::rand() % (hi - lo + 1));
}

// ---------- Main: perform random operations until exactly 100 nodes ----------

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    Node* head = nullptr;

    const int TARGET = 100;
    int ops = 0;

    // Keep doing random operations until the list has exactly 100 nodes.
    // We bias operations depending on current length so we converge.
    while (true) {
        int len = listLength(head);
        if (len == TARGET) break;

        // Choose operation:
        // 1) add new int to front
        // 2) delete random item
        // 3) insert new int at random position
        // 4) move node from one position to another
        int op = 1;

        if (len == 0) {
            // Must add if empty
            op = (randInt(0, 1) == 0) ? 1 : 3;
        } else if (len < TARGET) {
            // Prefer inserts when below target (but still allow delete/move sometimes)
            int r = randInt(1, 100);
            if (r <= 35) op = 1;
            else if (r <= 80) op = 3;
            else if (r <= 90) op = 4;
            else op = 2;
        } else {
            // len > TARGET: prefer deletes
            int r = randInt(1, 100);
            if (r <= 70) op = 2;
            else if (r <= 90) op = 4;
            else op = 3; // sometimes insert to avoid getting stuck in patterns
        }

        if (op == 1) {
            int value = randInt(-100000, 100000);
            pushFront(head, value);
        } else if (op == 2) {
            int idx = randInt(0, len - 1);
            deleteAt(head, idx);
        } else if (op == 3) {
            // Insert at random position in [0, len] (can insert at end)
            int idx = randInt(0, len);
            int value = randInt(-100000, 100000);
            insertAt(head, idx, value);
        } else if (op == 4) {
            if (len >= 2) {
                int fromIdx = randInt(0, len - 1);
                int toIdx = randInt(0, len); // insertion position (can be end)
                moveNode(head, fromIdx, toIdx);
            }
        }

        ++ops;

        // Safety stop (should not trigger, but prevents infinite loops if edited later)
        if (ops > 2000000) {
            std::cerr << "Too many operations; aborting.\n";
            break;
        }
    }

    int finalLen = listLength(head);
    std::cout << "Done after " << ops << " operations.\n";
    std::cout << "Final length: " << finalLen << "\n";
    // Optional: print the list (100 nodes is fine)
    printList(head);

    clearList(head);
    return 0;
}