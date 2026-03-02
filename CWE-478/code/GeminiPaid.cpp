#include <iostream>
#include <ctime>
#include <cstdlib>

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

    // 1. Add a new integer to the end
    void pushBack(int val) {
        Node* newNode = new Node(val);
        if (!head) {
            head = newNode;
        } else {
            Node* temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newNode;
        }
        size++;
    }

    // 2. Delete an item at a specific index
    void deleteAt(int index) {
        if (index < 0 || index >= size || !head) return;

        Node* temp = head;
        if (index == 0) {
            head = head->next;
            delete temp;
        } else {
            Node* prev = nullptr;
            for (int i = 0; i < index; i++) {
                prev = temp;
                temp = temp->next;
            }
            prev->next = temp->next;
            delete temp;
        }
        size--;
    }

    // 3. Add a new integer at a random position
    void insertAt(int index, int val) {
        if (index < 0 || index > size) return;
        if (index == 0) {
            Node* newNode = new Node(val);
            newNode->next = head;
            head = newNode;
            size++;
            return;
        }
        
        Node* newNode = new Node(val);
        Node* temp = head;
        for (int i = 0; i < index - 1; i++) {
            temp = temp->next;
        }
        newNode->next = temp->next;
        temp->next = newNode;
        size++;
    }

    // 4. Move an integer from one index to another
    void moveNode(int fromIdx, int toIdx) {
        if (fromIdx == toIdx || fromIdx >= size || toIdx >= size) return;

        // Extract the node
        Node* prevFrom = nullptr;
        Node* currFrom = head;
        for (int i = 0; i < fromIdx; i++) {
            prevFrom = currFrom;
            currFrom = currFrom->next;
        }

        if (prevFrom) prevFrom->next = currFrom->next;
        else head = currFrom->next;

        // Insert it back at toIdx
        if (toIdx == 0) {
            currFrom->next = head;
            head = currFrom;
        } else {
            Node* tempTo = head;
            // Adjust index if we removed a node from before the target
            int adjustIdx = (fromIdx < toIdx) ? toIdx - 1 : toIdx;
            for (int i = 0; i < adjustIdx; i++) {
                tempTo = tempTo->next;
            }
            currFrom->next = tempTo->next;
            tempTo->next = currFrom;
        }
    }

    int getSize() const { return size; }

    ~SinglyLinkedList() {
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

int main() {
    srand(time(0));
    SinglyLinkedList list;

    std::cout << "Starting operations..." << std::endl;

    while (list.getSize() < 100) {
        int op = rand() % 4;
        int val = rand() % 1000;

        switch (op) {
            case 0: // Add to end
                list.pushBack(val);
                break;
            case 1: // Delete random
                if (list.getSize() > 0)
                    list.deleteAt(rand() % list.getSize());
                break;
            case 2: // Add random position
                list.insertAt(rand() % (list.getSize() + 1), val);
                break;
            case 3: // Move random
                if (list.getSize() > 1) {
                    list.moveNode(rand() % list.getSize(), rand() % list.getSize());
                }
                break;
        }
    }

    std::cout << "Success! The list now contains " << list.getSize() << " nodes." << std::endl;
    return 0;
}