#include <iostream>
#include <cstdlib>
#include <ctime>

class Node {
public:
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

    void add(int val) {
        Node* newNode = new Node(val);
        if (head == nullptr) {
            head = newNode;
        } else {
            Node* temp = head;
            while (temp->next) {
                temp = temp->next;
            }
            temp->next = newNode;
        }
        size++;
    }

    void remove(int val) {
        if (head == nullptr) return;
        if (head->data == val) {
            Node* temp = head;
            head = head->next;
            delete temp;
            size--;
            return;
        }
        Node* current = head;
        while (current->next && current->next->data != val) {
            current = current->next;
        }
        if (current->next) {
            Node* temp = current->next;
            current->next = current->next->next;
            delete temp;
            size--;
        }
    }

    void randomInsert() {
        if (size >= 100) return;
        int randomValue = rand() % 1000;
        int position = rand() % (size + 1);
        Node* newNode = new Node(randomValue);

        if (position == 0) {
            newNode->next = head;
            head = newNode;
        } else {
            Node* current = head;
            for (int i = 0; i < position - 1; i++) {
                current = current->next;
            }
            newNode->next = current->next;
            current->next = newNode;
        }
        size++;
    }

    void moveNode(int oldPosition, int newPosition) {
        if (oldPosition < 0 || oldPosition >= size || newPosition < 0 || newPosition >= size) return;
        Node* current = head;
        Node* nodeToMove = nullptr;
        Node* previous = nullptr;

        for (int i = 0; i <= oldPosition; i++) {
            if (i == oldPosition) {
                nodeToMove = current;
            }
            if (i > 0) {
                previous = current;
            }
            current = current->next;
        }

        if (previous) previous->next = nodeToMove->next;
        else head = nodeToMove->next;

        Node* temp = head;
        Node* newPrevious = nullptr;
        int i = 0;

        while (i < newPosition) {
            newPrevious = temp;
            temp = temp ? temp->next : nullptr;
            i++;
        }

        nodeToMove->next = temp;
        if (newPrevious) {
            newPrevious->next = nodeToMove;
        } else {
            head = nodeToMove;
        }
    }

    void printList() {
        Node* temp = head;
        while (temp) {
            std::cout << temp->data << " ";
            temp = temp->next;
        }
        std::cout << std::endl;
    }

    int getSize() {
        return size;
    }
};

int main() {
    srand(time(0));
    SinglyLinkedList list;

    while (list.getSize() < 100) {
        int operation = rand() % 4;
        int value = rand() % 1000;
        switch (operation) {
            case 0:
                list.add(value);
                break;
            case 1:
                list.remove(value);
                break;
            case 2:
                list.randomInsert();
                break;
            case 3:
                if (rand() % 2 == 0) {
                    list.moveNode(rand() % list.getSize(), rand() % list.getSize());
                }
                break;
        }
    }

    list.printList();
    return 0;
}