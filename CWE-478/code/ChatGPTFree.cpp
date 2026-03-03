#include <iostream>
#include <cstdlib>
#include <ctime>

struct Node {
    int data;
    Node* next;
};

class SinglyLinkedList {
private:
    Node* head;
    int size;

public:
    SinglyLinkedList() : head(nullptr), size(0) {}

    ~SinglyLinkedList() {
        clear();
    }

    int getSize() const {
        return size;
    }

    void clear() {
        while (head != nullptr) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
        size = 0;
    }

    void pushBack(int value) {
        Node* newNode = new Node{value, nullptr};

        if (!head) {
            head = newNode;
        } else {
            Node* current = head;
            while (current->next)
                current = current->next;
            current->next = newNode;
        }
        size++;
    }

    void insertAt(int index, int value) {
        if (index < 0) index = 0;
        if (index > size) index = size;

        Node* newNode = new Node{value, nullptr};

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
    }

    void deleteAt(int index) {
        if (size == 0 || index < 0 || index >= size)
            return;

        Node* toDelete;

        if (index == 0) {
            toDelete = head;
            head = head->next;
        } else {
            Node* current = head;
            for (int i = 0; i < index - 1; i++)
                current = current->next;

            toDelete = current->next;
            current->next = toDelete->next;
        }

        delete toDelete;
        size--;
    }

    void moveNode(int fromIndex, int toIndex) {
        if (fromIndex < 0 || fromIndex >= size ||
            toIndex < 0 || toIndex >= size ||
            fromIndex == toIndex)
            return;

        Node* prevFrom = nullptr;
        Node* fromNode = head;

        for (int i = 0; i < fromIndex; i++) {
            prevFrom = fromNode;
            fromNode = fromNode->next;
        }

        // Detach node
        if (prevFrom)
            prevFrom->next = fromNode->next;
        else
            head = fromNode->next;

        // Adjust size temporarily
        size--;

        // Insert at new position
        if (toIndex == 0) {
            fromNode->next = head;
            head = fromNode;
        } else {
            Node* current = head;
            for (int i = 0; i < toIndex - 1; i++)
                current = current->next;

            fromNode->next = current->next;
            current->next = fromNode;
        }

        size++;
    }

    void printList() const {
        Node* current = head;
        while (current) {
            std::cout << current->data << " ";
            current = current->next;
        }
        std::cout << "\n";
    }
};

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    SinglyLinkedList list;

    while (list.getSize() != 100) {
        int operation = std::rand() % 4;

        if (list.getSize() == 0) {
            // If empty, must add
            list.pushBack(std::rand() % 1000);
            continue;
        }

        switch (operation) {
            case 0: // Add to end
                list.pushBack(std::rand() % 1000);
                break;

            case 1: // Delete random
                if (list.getSize() > 0) {
                    int index = std::rand() % list.getSize();
                    list.deleteAt(index);
                }
                break;

            case 2: // Insert at random
                {
                    int index = std::rand() % (list.getSize() + 1);
                    list.insertAt(index, std::rand() % 1000);
                }
                break;

            case 3: // Move random node
                if (list.getSize() > 1) {
                    int from = std::rand() % list.getSize();
                    int to = std::rand() % list.getSize();
                    list.moveNode(from, to);
                }
                break;
        }
    }

    std::cout << "Final list (100 nodes):\n";
    list.printList();

    return 0;
}