#include <iostream>
#include <cstdlib>
#include <ctime>

struct Node {
    int data;
    Node* next;
    Node(int val) : data(val), next(nullptr) {}
};

class LinkedList {
public:
    Node* head;
    int size;

    LinkedList() : head(nullptr), size(0) {}

    ~LinkedList() {
        Node* curr = head;
        while (curr) {
            Node* tmp = curr->next;
            delete curr;
            curr = tmp;
        }
    }

    // Operation 1: Append a new integer to the back
    void addBack(int val) {
        Node* newNode = new Node(val);
        if (!head) {
            head = newNode;
        } else {
            Node* curr = head;
            while (curr->next) curr = curr->next;
            curr->next = newNode;
        }
        size++;
    }

    // Operation 2: Delete a random node
    void deleteRandom() {
        if (!head) return;
        int idx = rand() % size;
        if (idx == 0) {
            Node* tmp = head;
            head = head->next;
            delete tmp;
        } else {
            Node* curr = head;
            for (int i = 0; i < idx - 1; i++) curr = curr->next;
            Node* tmp = curr->next;
            curr->next = tmp->next;
            delete tmp;
        }
        size--;
    }

    // Operation 3: Insert a new integer at a random position
    void insertRandom(int val) {
        int idx = rand() % (size + 1);
        Node* newNode = new Node(val);
        if (idx == 0) {
            newNode->next = head;
            head = newNode;
        } else {
            Node* curr = head;
            for (int i = 0; i < idx - 1; i++) curr = curr->next;
            newNode->next = curr->next;
            curr->next = newNode;
        }
        size++;
    }

    // Operation 4: Move a node from one random position to another
    void moveRandom() {
        if (size < 2) return;

        int from = rand() % size;
        int to   = rand() % size;
        if (from == to) to = (to + 1) % size;

        // Extract node at 'from'
        Node* fromNode = nullptr;
        if (from == 0) {
            fromNode = head;
            head = head->next;
        } else {
            Node* prev = head;
            for (int i = 0; i < from - 1; i++) prev = prev->next;
            fromNode = prev->next;
            prev->next = fromNode->next;
        }
        fromNode->next = nullptr;
        size--;

        // Insert at 'to'
        if (to > size) to = size;
        if (to == 0) {
            fromNode->next = head;
            head = fromNode;
        } else {
            Node* curr = head;
            for (int i = 0; i < to - 1; i++) curr = curr->next;
            fromNode->next = curr->next;
            curr->next = fromNode;
        }
        size++;
    }

    void print() const {
        Node* curr = head;
        int count = 0;
        while (curr) {
            std::cout << curr->data;
            if (curr->next) std::cout << " -> ";
            curr = curr->next;
            if (++count % 10 == 0 && curr) std::cout << "\n";
        }
        std::cout << "\n";
    }
};

int main() {
    srand((unsigned)time(nullptr));

    LinkedList list;
    int opCount = 0;

    std::cout << "Performing random operations until size reaches exactly 100...\n\n";

    while (list.size != 100) {
        int op;
        if (list.size == 0)      op = 0;
        else if (list.size < 10) op = rand() % 3;  // avoid move on tiny list
        else                      op = rand() % 4;

        int val = rand() % 1000;

        switch (op) {
            case 0:
                list.addBack(val);
                std::cout << "Op " << ++opCount
                          << ": AddBack(" << val
                          << ")  | Size: " << list.size << "\n";
                break;
            case 1:
                list.deleteRandom();
                std::cout << "Op " << ++opCount
                          << ": DeleteRandom()  | Size: " << list.size << "\n";
                break;
            case 2:
                list.insertRandom(val);
                std::cout << "Op " << ++opCount
                          << ": InsertRandom(" << val
                          << ")  | Size: " << list.size << "\n";
                break;
            case 3:
                list.moveRandom();
                std::cout << "Op " << ++opCount
                          << ": MoveRandom()  | Size: " << list.size << "\n";
                break;
        }
    }

    std::cout << "\n==============================================\n";
    std::cout << "  List reached exactly 100 nodes!\n";
    std::cout << "  Total operations performed: " << opCount << "\n";
    std::cout << "==============================================\n\n";
    std::cout << "Final 100-node list:\n";
    list.print();
    std::cout << "\nVerified size: " << list.size << "\n";

    return 0;
}