#include <iostream>
#include <stdexcept>
#include <algorithm> // For std::copy

// Define a simple object class for demonstration purposes
class MyObject {
private:
    int id;
    std::string name;
public:
    MyObject(int i, const std::string& n) : id(i), name(n) {
        std::cout << "  [INFO] MyObject " << id << " created." << std::endl;
    }
    ~MyObject() {
        std::cout << "  [INFO] MyObject " << id << " destroyed." << std::endl;
    }
    std::string toString() const {
        return "Object(ID: " + std::to_string(id) + ", Name: " + name + ")";
    }
    int getId() const { return id; }
};

/**
 * @brief Manages a dynamic collection of objects stored in heap memory.
 * * The collection stores pointers to objects of type T and is responsible for
 * dynamically allocating (using 'new') and deallocating (using 'delete')
 * the objects it holds.
 * * @tparam T The type of object to store.
 */
template <typename T>
class DynamicObjectCollection {
private:
    T** items;         // Array of pointers to T (the actual objects)
    size_t count;      // Current number of items
    size_t capacity;   // Current storage capacity

    /**
     * @brief Resizes the internal array to the new capacity.
     * @param newCapacity The desired new capacity.
     */
    void resize(size_t newCapacity) {
        if (newCapacity <= capacity) return;

        T** newItems = new T*[newCapacity];
        
        // Copy existing pointers to the new array
        if (items != nullptr) {
            // Note: std::copy is safe here as it copies the pointer values, not the objects.
            std::copy(items, items + count, newItems);
            delete[] items; // Delete the old array of pointers
        }

        items = newItems;
        capacity = newCapacity;
    }

public:
    /**
     * @brief Constructor: Initializes an empty collection.
     */
    DynamicObjectCollection() : items(nullptr), count(0), capacity(0) {}

    /**
     * @brief Destructor: Cleans up all dynamically allocated memory.
     * * IMPORTANT: This ensures two things:
     * 1. Deletes every object stored on the heap (items[i]).
     * 2. Deletes the dynamic array of pointers itself (items).
     */
    ~DynamicObjectCollection() {
        std::cout << "\n[Cleanup] Destroying collection and its " << count << " objects..." << std::endl;
        // 1. Delete the objects pointed to by the array
        for (size_t i = 0; i < count; ++i) {
            delete items[i];
        }
        // 2. Delete the array of pointers itself
        delete[] items;
        std::cout << "[Cleanup] Collection fully destroyed." << std::endl;
    }

    /**
     * @brief Adds a new object to the collection.
     * * Creates a deep copy of the provided object on the heap and stores its pointer.
     * @param obj The object to copy and add.
     */
    void add(const T& obj) {
        // If we reached capacity, resize the array (e.g., double the capacity)
        if (count == capacity) {
            size_t newCapacity = (capacity == 0) ? 4 : capacity * 2;
            resize(newCapacity);
        }
        
        // Create a copy of the object on the heap and store the pointer
        items[count] = new T(obj); 
        count++;
    }

    /**
     * @brief Accesses an object by index.
     * @param index The index of the object to retrieve.
     * @return A reference to the object at the specified index.
     */
    T& get(size_t index) const {
        if (index >= count) {
            throw std::out_of_range("Index out of bounds in get()");
        }
        return *items[index]; // Dereference the stored pointer to return the object reference
    }

    /**
     * @brief Removes the object at the specified index.
     * * The object is first deleted from the heap, and then the array is shifted.
     * @param index The index of the object to remove.
     */
    void remove(size_t index) {
        if (index >= count) {
            throw std::out_of_range("Index out of bounds in remove()");
        }

        // 1. Delete the object from the heap
        delete items[index];

        // 2. Shift all subsequent elements one position to the left
        for (size_t i = index; i < count - 1; ++i) {
            items[i] = items[i + 1];
        }
        
        // 3. Decrement the size
        count--;
    }

    /**
     * @brief Returns the current number of elements in the collection.
     */
    size_t getSize() const {
        return count;
    }
};

// --- Demonstration ---

void printCollectionStatus(const DynamicObjectCollection<MyObject>& collection) {
    std::cout << "--- Collection Status (Size: " << collection.getSize() << ") ---" << std::endl;
    for (size_t i = 0; i < collection.getSize(); ++i) {
        try {
            std::cout << "  [" << i << "]: " << collection.get(i).toString() << std::endl;
        } catch (const std::out_of_range& e) {
            std::cout << "  Error accessing index " << i << ": " << e.what() << std::endl;
        }
    }
    std::cout << "---------------------------------------" << std::endl;
}

int main() {
    // Note: The collection object itself is on the stack, but it manages the heap memory.
    DynamicObjectCollection<MyObject> collection;

    // 1. Add objects
    std::cout << "--- Adding Objects ---" << std::endl;
    collection.add(MyObject(101, "Alpha"));
    collection.add(MyObject(102, "Beta"));
    collection.add(MyObject(103, "Gamma"));
    collection.add(MyObject(104, "Delta"));
    collection.add(MyObject(105, "Epsilon"));
    printCollectionStatus(collection);

    // 2. Access an object by index
    std::cout << "\n--- Accessing Object at Index 2 ---" << std::endl;
    try {
        MyObject& obj = collection.get(2);
        std::cout << "Accessed: " << obj.toString() << std::endl;
        // Demonstrate modification via reference
        obj = MyObject(999, "MODIFIED_GAMMA");
        std::cout << "After Modification: " << obj.toString() << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    printCollectionStatus(collection);


    // 3. Remove an object
    std::cout << "\n--- Removing Object at Index 1 (Beta) ---" << std::endl;
    try {
        collection.remove(1);
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    printCollectionStatus(collection);

    // 4. Final check before automatic cleanup (Destructor call)
    std::cout << "\n--- End of Main Scope ---" << std::endl;
    // When main finishes, the collection's destructor is called, cleaning up all 
    // remaining objects (101, 999, 104, 105) and the array of pointers.

    return 0;
}