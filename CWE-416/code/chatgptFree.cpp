#include <iostream>
#include <stdexcept>

class Object {
public:
    int value;

    Object(int v = 0) : value(v) {}
    void display() const { std::cout << value << std::endl; }
};

class ObjectManager {
private:
    Object** objects;  // array of pointers to Object
    size_t capacity;
    size_t size;

    // Resize the internal array when needed
    void resize(size_t newCapacity) {
        Object** newArray = new Object*[newCapacity];
        for (size_t i = 0; i < size; ++i) {
            newArray[i] = objects[i];
        }
        delete[] objects;
        objects = newArray;
        capacity = newCapacity;
    }

public:
    ObjectManager(size_t initialCapacity = 2)
        : capacity(initialCapacity), size(0) {
        objects = new Object*[capacity];
    }

    ~ObjectManager() {
        for (size_t i = 0; i < size; ++i) {
            delete objects[i];  // delete each object
        }
        delete[] objects;  // delete array of pointers
    }

    // Add a new object
    void add(Object* obj) {
        if (size >= capacity) {
            resize(capacity * 2);  // double the capacity
        }
        objects[size++] = obj;
    }

    // Remove object at index
    void remove(size_t index) {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        delete objects[index];  // delete the object

        // Shift remaining elements
        for (size_t i = index; i < size - 1; ++i) {
            objects[i] = objects[i + 1];
        }
        --size;
    }

    // Access object by index
    Object* get(size_t index) const {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        return objects[index];
    }

    size_t getSize() const { return size; }
};

int main() {
    ObjectManager manager;

    manager.add(new Object(10));
    manager.add(new Object(20));
    manager.add(new Object(30));

    std::cout << "Objects in manager:" << std::endl;
    for (size_t i = 0; i < manager.getSize(); ++i) {
        manager.get(i)->display();
    }

    std::cout << "Removing object at index 1..." << std::endl;
    manager.remove(1);

    std::cout << "Objects after removal:" << std::endl;
    for (size_t i = 0; i < manager.getSize(); ++i) {
        manager.get(i)->display();
    }

    return 0;
}
