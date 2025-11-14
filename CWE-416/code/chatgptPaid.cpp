#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>

template <typename T>
class ObjectManager {
public:
    // Add a new dynamically allocated object
    void add(const T& obj) {
        objects.push_back(std::make_unique<T>(obj));
    }

    // Remove an object at a specific index
    void removeAt(size_t index) {
        if (index >= objects.size()) {
            throw std::out_of_range("Index out of range");
        }
        objects.erase(objects.begin() + index);
    }

    // Access object by index
    T& get(size_t index) {
        if (index >= objects.size()) {
            throw std::out_of_range("Index out of range");
        }
        return *objects[index];
    }

    const T& get(size_t index) const {
        if (index >= objects.size()) {
            throw std::out_of_range("Index out of range");
        }
        return *objects[index];
    }

    // Optional: get number of objects
    size_t size() const { return objects.size(); }

private:
    std::vector<std::unique_ptr<T>> objects;
};
