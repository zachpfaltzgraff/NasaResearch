#include <iostream>
#include <stdexcept>
#include <utility>

template <typename T>
class DynamicCollection {
private:
    T* data;           // Pointer to dynamically allocated array
    size_t size;       // Current number of elements
    size_t capacity;   // Total allocated capacity

    // Helper function to resize the internal array
    void resize(size_t newCapacity) {
        T* newData = new T[newCapacity];
        
        // Copy existing elements to new array
        for (size_t i = 0; i < size; ++i) {
            newData[i] = std::move(data[i]);
        }
        
        // Delete old array and update pointer
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

public:
    // Constructor
    DynamicCollection() : data(nullptr), size(0), capacity(0) {
        capacity = 4; // Initial capacity
        data = new T[capacity];
    }

    // Destructor
    ~DynamicCollection() {
        delete[] data;
    }

    // Copy constructor
    DynamicCollection(const DynamicCollection& other) 
        : size(other.size), capacity(other.capacity) {
        data = new T[capacity];
        for (size_t i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }

    // Copy assignment operator
    DynamicCollection& operator=(const DynamicCollection& other) {
        if (this != &other) {
            delete[] data;
            size = other.size;
            capacity = other.capacity;
            data = new T[capacity];
            for (size_t i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    // Move constructor
    DynamicCollection(DynamicCollection&& other) noexcept
        : data(other.data), size(other.size), capacity(other.capacity) {
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }

    // Move assignment operator
    DynamicCollection& operator=(DynamicCollection&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            size = other.size;
            capacity = other.capacity;
            other.data = nullptr;
            other.size = 0;
            other.capacity = 0;
        }
        return *this;
    }

    // Add an object to the collection
    void add(const T& item) {
        if (size >= capacity) {
            resize(capacity * 2); // Double capacity when full
        }
        data[size++] = item;
    }

    // Add an object using move semantics
    void add(T&& item) {
        if (size >= capacity) {
            resize(capacity * 2);
        }
        data[size++] = std::move(item);
    }

    // Remove an object at a specific index
    void remove(size_t index) {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }

        // Shift elements left to fill the gap
        for (size_t i = index; i < size - 1; ++i) {
            data[i] = std::move(data[i + 1]);
        }
        --size;

        // Optional: shrink capacity if size is much smaller
        if (size > 0 && size <= capacity / 4) {
            resize(capacity / 2);
        }
    }

    // Access element by index (const version)
    const T& get(size_t index) const {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }

    // Access element by index (non-const version)
    T& get(size_t index) {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }

    // Operator[] for convenient access
    T& operator[](size_t index) {
        return get(index);
    }

    const T& operator[](size_t index) const {
        return get(index);
    }

    // Get current size
    size_t getSize() const {
        return size;
    }

    // Get current capacity
    size_t getCapacity() const {
        return capacity;
    }

    // Check if collection is empty
    bool isEmpty() const {
        return size == 0;
    }

    // Clear all elements
    void clear() {
        size = 0;
    }

    // Print all elements (for testing)
    void print() const {
        std::cout << "Collection [size=" << size << ", capacity=" << capacity << "]: ";
        for (size_t i = 0; i < size; ++i) {
            std::cout << data[i];
            if (i < size - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
};

// Example usage
int main() {
    std::cout << "=== Dynamic Collection Demo ===" << std::endl << std::endl;

    // Create a collection of integers
    DynamicCollection<int> numbers;

    std::cout << "Adding elements..." << std::endl;
    for (int i = 1; i <= 10; ++i) {
        numbers.add(i * 10);
    }
    numbers.print();

    std::cout << "\nAccessing elements:" << std::endl;
    std::cout << "Element at index 0: " << numbers.get(0) << std::endl;
    std::cout << "Element at index 5: " << numbers[5] << std::endl;
    std::cout << "Element at index 9: " << numbers[9] << std::endl;

    std::cout << "\nModifying element at index 3..." << std::endl;
    numbers[3] = 999;
    numbers.print();

    std::cout << "\nRemoving element at index 5..." << std::endl;
    numbers.remove(5);
    numbers.print();

    std::cout << "\nRemoving element at index 0..." << std::endl;
    numbers.remove(0);
    numbers.print();

    std::cout << "\nCollection info:" << std::endl;
    std::cout << "Size: " << numbers.getSize() << std::endl;
    std::cout << "Capacity: " << numbers.getCapacity() << std::endl;

    // Test with strings
    std::cout << "\n=== String Collection ===" << std::endl;
    DynamicCollection<std::string> words;
    words.add("Hello");
    words.add("World");
    words.add("C++");
    words.add("Dynamic");
    words.add("Memory");
    words.print();

    std::cout << "\nRemoving 'C++'..." << std::endl;
    words.remove(2);
    words.print();

    // Test error handling
    std::cout << "\n=== Error Handling ===" << std::endl;
    try {
        std::cout << "Trying to access index 100..." << std::endl;
        numbers.get(100);
    } catch (const std::out_of_range& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }

    try {
        std::cout << "Trying to remove index 100..." << std::endl;
        numbers.remove(100);
    } catch (const std::out_of_range& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }

    return 0;
}