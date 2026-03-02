#include <iostream>
#include <ctime>
#include <cstdlib>

struct Node {
    int data;
    Node* next;
};

class LinkedList {
private:
    Node* head;
    int size;

public:
    LinkedList() : head(nullptr), size(0) {}

    // 1) Add a new integer (to the end)
    void append(int val) {
        Node* newNode = new Node{val, nullptr};
        if (!head) {
            head = newNode;
        } else {
            Node* temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newNode;
        }
        size++;
    }

    // 2) Delete a random item
    void deleteRandom() {
        if (size == 0) return;
        int index = rand() % size;

        Node* toDelete;
        if (index == 0) {
            toDelete = head;
            head = head->next;
        } else {
            Node* prev = head;
            for (int i = 0; i < index - 1; ++i) prev = prev->next;
            toDelete = prev->next;
            prev->next = toDelete->next;
        }
        delete toDelete;
        size--;
    }

    // 3) Add a new integer at a random position
    void insertRandom(int val) {
        int index = (size == 0) ? 0 : rand() % (size + 1);
        Node* newNode = new Node{val, nullptr};

        if (index == 0) {
            newNode->next = head;
            head = newNode;
        } else {
            Node* temp = head;
            for (int i = 0; i < index - 1; ++i) temp = temp->next;
            newNode->next = temp->next;
            temp->next = newNode;
        }
        size++;
    }

    // 4) Move an integer from one area to another
    void moveNode() {
        if (size < 2) return;
        int fromIdx = rand() % size;
        int toIdx = rand() % size;
        if (fromIdx == toIdx) return;

        // Extract the node
        Node *prevFrom = nullptr, *currFrom = head;
        for (int i = 0; i < fromIdx; ++i) {
            prevFrom = currFrom;
            currFrom = currFrom->next;
        }

        if (prevFrom) prevFrom->next = currFrom->next;
        else head = currFrom->next;

        // Insert it at the new position
        if (toIdx == 0) {
            currFrom->next = head;
            head = currFrom;
        } else {
            Node* temp = head;
            // Adjust toIdx because the list size technically shrunk during extraction
            for (int i = 0; i < toIdx - 1; ++i) temp = temp->next;
            currFrom->next = temp->next;
            temp->next = currFrom;
        }
    }

    int getSize() { return size; }

    ~LinkedList() {
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

int main() {
    srand(time(0));
    LinkedList list;

    std::cout << "Running random operations until size is 100...\n";

    while (list.getSize() < 100) {
        int op = rand() % 4;
        int val = rand() % 1000;

        switch (op) {
            case 0: list.append(val); break;
            case 1: list.deleteRandom(); break;
            case 2: list.insertRandom(val); break;
            case 3: list.moveNode(); break;
        }
    }

    std::cout << "Success! Final list size: " << list.getSize() << std::endl;
    return 0;
}