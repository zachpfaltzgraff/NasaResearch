#include <iostream>
#include <stdexcept>
#include <algorithm> // Required for std::copy

// Define the initial capacity for the dynamic array
constexpr size_t INITIAL_CAPACITY = 4;

/**
 * @brief A generic class to manage a collection of objects stored in
 * dynamically allocated memory.
 * * Implements the core functionality of a vector: adding, removing, and 
 * indexed access, while handling memory management (allocation and resizing).
 * * @tparam T The type of object to store in the array.
 */
template <typename T>
class DynamicArray {
private:
    T* data;           // Pointer to the dynamically allocated array
    size_t currentSize;    // Current number of elements stored
    size_t maxCapacity;    // Total memory allocated

    /**
     * @brief Doubles the array's capacity and copies elements to the new memory block.
     */
    void resize() {
        // Calculate new capacity (double the current capacity)
        size_t newCapacity = (maxCapacity == 0) ? INITIAL_CAPACITY : maxCapacity * 2;
        
        // 1. Allocate new memory
        T* newData = new T[newCapacity];
        
        // 2. Copy existing elements to the new memory
        // std::copy is safer and often faster than a manual loop
        std::copy(data, data + currentSize, newData);
        
        // 3. Deallocate old memory
        delete[] data;
        
        // 4. Update members
        data = newData;
        maxCapacity = newCapacity;

        std::cout << "[INFO] Resized array to capacity: " << maxCapacity << std::endl;
    }

public:
    // --- Rule of Five (Memory Management) ---

    /**
     * @brief Constructor: Initializes the array with zero capacity.
     */
    DynamicArray() : data(nullptr), currentSize(0), maxCapacity(0) {
        std::cout << "[INFO] DynamicArray created." << std::endl;
    }

    /**
     * @brief Destructor: Deallocates the dynamically allocated memory.
     */
    ~DynamicArray() {
        delete[] data;
        std::cout << "[INFO] DynamicArray destroyed and memory freed." << std::endl;
    }

    // Prevent copies (for simplicity, we'll only allow moves)
    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    /**
     * @brief Move Constructor: Transfers ownership of resources from another array.
     */
    DynamicArray(DynamicArray&& other) noexcept
        : data(other.data), 
          currentSize(other.currentSize), 
          maxCapacity(other.maxCapacity) {
        
        // Nullify 'other' so its destructor doesn't free the memory we now own
        other.data = nullptr;
        other.currentSize = 0;
        other.maxCapacity = 0;
    }

    /**
     * @brief Move Assignment Operator: Transfers ownership of resources.
     */
    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            // 1. Free existing resources
            delete[] data;

            // 2. Transfer resources
            data = other.data;
            currentSize = other.currentSize;
            maxCapacity = other.maxCapacity;

            // 3. Nullify 'other'
            other.data = nullptr;
            other.currentSize = 0;
            other.maxCapacity = 0;
        }
        return *this;
    }

    // --- Core Functionality ---

    /**
     * @brief Adds a new object to the end of the collection.
     * @param element The object to add.
     */
    void pushBack(const T& element) {
        // Check if resizing is necessary
        if (currentSize == maxCapacity) {
            resize();
        }
        // Add the new element
        data[currentSize] = element;
        currentSize++;
    }

    /**
     * @brief Removes an object at the specified index, shifting remaining elements.
     * @param index The zero-based index of the object to remove.
     * @throws std::out_of_range If the index is invalid.
     */
    void removeAt(size_t index) {
        if (index >= currentSize) {
            throw std::out_of_range("Index out of range for removal.");
        }

        // Shift elements left to overwrite the removed element
        for (size_t i = index; i < currentSize - 1; ++i) {
            data[i] = data[i + 1];
        }

        // Decrease the size
        currentSize--;
    }

    /**
     * @brief Accesses an element at the specified index.
     * @param index The zero-based index of the object to access.
     * @return A reference to the object, allowing modification.
     * @throws std::out_of_range If the index is invalid.
     */
    T& get(size_t index) {
        if (index >= currentSize) {
            throw std::out_of_range("Index out of range for access.");
        }
        return data[index];
    }
    
    /**
     * @brief Returns the current number of elements in the collection.
     * @return The size of the collection.
     */
    size_t getSize() const {
        return currentSize;
    }
};

// --- Example Usage ---

void printArray(const DynamicArray<int>& arr) {
    std::cout << "  [Content] (Size: " << arr.getSize() << "): ";
    for (size_t i = 0; i < arr.getSize(); ++i) {
        // We use 'get' for safe access
        std::cout << arr.get(i) << (i < arr.getSize() - 1 ? ", " : "");
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "--- Dynamic Array Management Example ---\n" << std::endl;
    
    // Create an instance of the dynamic array for integers
    DynamicArray<int> myCollection;

    // 1. Add objects
    std::cout << "1. Adding elements (5, 10, 15, 20, 25, 30):\n";
    myCollection.pushBack(5);
    myCollection.pushBack(10);
    myCollection.pushBack(15);
    myCollection.pushBack(20); // This should trigger a resize
    myCollection.pushBack(25);
    myCollection.pushBack(30);

    printArray(myCollection);
    std::cout << std::endl;

    // 2. Access and Modify objects by index
    std::cout << "2. Accessing element at index 2 (15) and modifying it to 99:\n";
    try {
        int& elementToModify = myCollection.get(2);
        std::cout << "  Original value at index 2: " << elementToModify << std::endl;
        elementToModify = 99; // Modification via the returned reference
        std::cout << "  New value at index 2: " << myCollection.get(2) << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error accessing element: " << e.what() << std::endl;
    }
    
    printArray(myCollection);
    std::cout << std::endl;


    // 3. Remove an object by index
    std::cout << "3. Removing element at index 1 (10):\n";
    try {
        myCollection.removeAt(1);
        std::cout << "  Element removed successfully.\n";
    } catch (const std::out_of_range& e) {
        std::cerr << "Error removing element: " << e.what() << std::endl;
    }
    
    printArray(myCollection);
    std::cout << std::endl;
    
    std::cout << "4. Removing element at the end (index " << myCollection.getSize() - 1 << "): " << myCollection.get(myCollection.getSize() - 1) << "\n";
    try {
        myCollection.removeAt(myCollection.getSize() - 1);
        std::cout << "  Element removed successfully.\n";
    } catch (const std::out_of_range& e) {
        std::cerr << "Error removing element: " << e.what() << std::endl;
    }

    printArray(myCollection);
    
    // Note: The destructor is automatically called when 'myCollection' goes out of scope.
    std::cout << "\n--- End of Program ---\n";
    return 0;
}