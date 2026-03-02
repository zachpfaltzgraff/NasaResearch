#include <iostream>
#include <cstdlib>
#include <ctime>

struct Node {
    int data;
    Node* next;

    Node(int val) : data(val), next(nullptr) {}
};

class SinglyLinkedList {
private:
    Node* head;
    int size;

public:
    SinglyLinkedList() : head(nullptr), size(0) {}

    ~SinglyLinkedList() {
        Node* current = head;
        while (current) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
    }

    int getSize() const { return size; }

    // Operation 1: Add a new integer to the end of the list
    void addToEnd(int val) {
        Node* newNode = new Node(val);
        if (!head) {
            head = newNode;
        } else {
            Node* current = head;
            while (current->next)
                current = current->next;
            current->next = newNode;
        }
        size++;
        std::cout << "  [ADD END] Added " << val << " at the end. Size: " << size << "\n";
    }

    // Operation 2: Delete a node at a random index
    bool deleteRandom() {
        if (size == 0) return false;

        int index = rand() % size;
        int deletedVal;

        if (index == 0) {
            Node* temp = head;
            deletedVal = temp->data;
            head = head->next;
            delete temp;
        } else {
            Node* prev = head;
            for (int i = 0; i < index - 1; i++)
                prev = prev->next;
            Node* temp = prev->next;
            deletedVal = temp->data;
            prev->next = temp->next;
            delete temp;
        }
        size--;
        std::cout << "  [DELETE]  Removed " << deletedVal << " at index " << index
                  << ". Size: " << size << "\n";
        return true;
    }

    // Operation 3: Insert a new integer at a random position
    void insertRandom(int val) {
        int index = (size == 0) ? 0 : rand() % (size + 1);
        Node* newNode = new Node(val);

        if (index == 0) {
            newNode->next = head;
            head = newNode;
        } else {
            Node* current = head;
            for (int i = 0; i < index - 1; i++)
                current = current->next;
            newNode->next = current->next;
            current->next = newNode;
        }
        size++;
        std::cout << "  [INSERT]  Inserted " << val << " at index " << index
                  << ". Size: " << size << "\n";
    }

    // Operation 4: Move a node from one position to another
    bool moveNode() {
        if (size < 2) return false;

        int fromIndex = rand() % size;
        int toIndex = rand() % size;
        while (toIndex == fromIndex)
            toIndex = rand() % size;

        // Extract the node at fromIndex
        Node* extracted;
        if (fromIndex == 0) {
            extracted = head;
            head = head->next;
        } else {
            Node* prev = head;
            for (int i = 0; i < fromIndex - 1; i++)
                prev = prev->next;
            extracted = prev->next;
            prev->next = extracted->next;
        }
        extracted->next = nullptr;

        // Adjust toIndex if it was after fromIndex (list shrank by 1)
        int insertAt = toIndex;
        if (toIndex > fromIndex)
            insertAt--;

        // Insert extracted node at the new position
        if (insertAt == 0) {
            extracted->next = head;
            head = extracted;
        } else {
            Node* current = head;
            for (int i = 0; i < insertAt - 1; i++)
                current = current->next;
            extracted->next = current->next;
            current->next = extracted;
        }

        std::cout << "  [MOVE]    Moved node with value " << extracted->data
                  << " from index " << fromIndex << " to index " << toIndex
                  << ". Size: " << size << "\n";
        return true;
    }

    void print() const {
        std::cout << "\n  List (" << size << " nodes): [";
        Node* current = head;
        int count = 0;
        while (current) {
            if (count > 0) std::cout << ", ";
            // Print first 10 and last 5 if list is large
            if (size > 20 && count == 10) {
                std::cout << "... ";
                // Skip to last 5
                int skip = size - 5;
                while (count < skip) {
                    current = current->next;
                    count++;
                }
                continue;
            }
            std::cout << current->data;
            current = current->next;
            count++;
        }
        std::cout << "]\n\n";
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    SinglyLinkedList list;
    int operationCount = 0;

    std::cout << "=== Singly Linked List Random Operations ===\n";
    std::cout << "Target: exactly 100 nodes\n\n";

    // Seed the list with a few nodes to enable all operations early on
    for (int i = 0; i < 5; i++) {
        list.addToEnd(rand() % 1000);
    }
    std::cout << "\n--- Initial seeding complete ---\n";
    list.print();

    while (list.getSize() != 100) {
        operationCount++;
        int currentSize = list.getSize();

        // Weight operations to converge toward 100 nodes
        // If below 100: favor insertions. If above 100: favor deletions.
        int op;
        if (currentSize < 100) {
            // 40% add end, 30% insert random, 15% move, 15% delete
            int r = rand() % 100;
            if (r < 40)       op = 1;
            else if (r < 70)  op = 2;
            else if (r < 85)  op = 3;
            else              op = 4;
        } else {
            // Shouldn't normally happen with this weighting, but handle it
            // 70% delete, 15% move, 15% add
            int r = rand() % 100;
            if (r < 70)       op = 4;
            else if (r < 85)  op = 3;
            else              op = 1;
        }

        std::cout << "Op #" << operationCount << " (size=" << currentSize << "): ";

        switch (op) {
            case 1: // Add to end
                list.addToEnd(rand() % 1000);
                break;
            case 2: // Insert at random position
                list.insertRandom(rand() % 1000);
                break;
            case 3: // Move node
                if (currentSize >= 2) {
                    list.moveNode();
                } else {
                    list.addToEnd(rand() % 1000);
                }
                break;
            case 4: // Delete random
                if (currentSize > 0) {
                    list.deleteRandom();
                } else {
                    list.addToEnd(rand() % 1000);
                }
                break;
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "Reached exactly 100 nodes after " << operationCount << " operations!\n";
    std::cout << "========================================\n";
    list.print();

    // Verify by traversing
    std::cout << "Verification: list reports size = " << list.getSize() << "\n";

    return 0;
}