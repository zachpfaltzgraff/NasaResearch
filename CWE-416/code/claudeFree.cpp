#include <iostream>
#include <stdexcept>
#include <utility>

template <typename T>
class DynamicCollection {
private:
    T** items;           // Array of pointers to objects
    size_t capacity;     // Current capacity
    size_t count;        // Number of items
    
    // Resize internal array when needed
    void resize(size_t newCapacity) {
        T** newItems = new T*[newCapacity];
        
        // Copy existing pointers
        for (size_t i = 0; i < count; ++i) {
            newItems[i] = items[i];
        }
        
        delete[] items;
        items = newItems;
        capacity = newCapacity;
    }
    
public:
    // Constructor
    DynamicCollection(size_t initialCapacity = 4) 
        : capacity(initialCapacity), count(0) {
        items = new T*[capacity];
    }
    
    // Destructor - clean up all dynamically allocated memory
    ~DynamicCollection() {
        clear();
        delete[] items;
    }
    
    // Copy constructor
    DynamicCollection(const DynamicCollection& other) 
        : capacity(other.capacity), count(other.count) {
        items = new T*[capacity];
        for (size_t i = 0; i < count; ++i) {
            items[i] = new T(*other.items[i]);
        }
    }
    
    // Copy assignment operator
    DynamicCollection& operator=(const DynamicCollection& other) {
        if (this != &other) {
            clear();
            delete[] items;
            
            capacity = other.capacity;
            count = other.count;
            items = new T*[capacity];
            
            for (size_t i = 0; i < count; ++i) {
                items[i] = new T(*other.items[i]);
            }
        }
        return *this;
    }
    
    // Move constructor
    DynamicCollection(DynamicCollection&& other) noexcept
        : items(other.items), capacity(other.capacity), count(other.count) {
        other.items = nullptr;
        other.capacity = 0;
        other.count = 0;
    }
    
    // Move assignment operator
    DynamicCollection& operator=(DynamicCollection&& other) noexcept {
        if (this != &other) {
            clear();
            delete[] items;
            
            items = other.items;
            capacity = other.capacity;
            count = other.count;
            
            other.items = nullptr;
            other.capacity = 0;
            other.count = 0;
        }
        return *this;
    }
    
    // Add a new object (takes ownership)
    void add(const T& obj) {
        if (count >= capacity) {
            resize(capacity * 2);
        }
        items[count++] = new T(obj);
    }
    
    // Remove object at index
    void remove(size_t index) {
        if (index >= count) {
            throw std::out_of_range("Index out of range");
        }
        
        delete items[index];
        
        // Shift remaining items
        for (size_t i = index; i < count - 1; ++i) {
            items[i] = items[i + 1];
        }
        
        --count;
        
        // Shrink if using less than 1/4 of capacity
        if (count > 0 && count < capacity / 4 && capacity > 4) {
            resize(capacity / 2);
        }
    }
    
    // Access object by index (const version)
    const T& get(size_t index) const {
        if (index >= count) {
            throw std::out_of_range("Index out of range");
        }
        return *items[index];
    }
    
    // Access object by index (non-const version)
    T& get(size_t index) {
        if (index >= count) {
            throw std::out_of_range("Index out of range");
        }
        return *items[index];
    }
    
    // Operator[] overload for convenient access
    T& operator[](size_t index) {
        return get(index);
    }
    
    const T& operator[](size_t index) const {
        return get(index);
    }
    
    // Get current size
    size_t size() const {
        return count;
    }
    
    // Check if empty
    bool empty() const {
        return count == 0;
    }
    
    // Clear all objects
    void clear() {
        for (size_t i = 0; i < count; ++i) {
            delete items[i];
        }
        count = 0;
    }
};

// Example usage
int main() {
    DynamicCollection<std::string> collection;
    
    // Add items
    collection.add("Hello");
    collection.add("World");
    collection.add("C++");
    collection.add("Programming");
    
    std::cout << "Collection size: " << collection.size() << "\n\n";
    
    // Access items
    std::cout << "Items in collection:\n";
    for (size_t i = 0; i < collection.size(); ++i) {
        std::cout << i << ": " << collection[i] << "\n";
    }
    
    // Modify an item
    collection[1] = "Beautiful World";
    std::cout << "\nAfter modification:\n";
    std::cout << "Item 1: " << collection[1] << "\n";
    
    // Remove an item
    collection.remove(2);
    std::cout << "\nAfter removing index 2:\n";
    for (size_t i = 0; i < collection.size(); ++i) {
        std::cout << i << ": " << collection[i] << "\n";
    }
    
    std::cout << "\nFinal size: " << collection.size() << "\n";
    
    return 0;
}