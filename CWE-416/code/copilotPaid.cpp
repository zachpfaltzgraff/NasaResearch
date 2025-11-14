// Simple header-only dynamic array using manual heap allocation and placement-new.
// Usage: include this header and instantiate DynamicArray<T> for any T.
//
// Notes:
// - Implements basic rule-of-five (dtor, copy/move ctors, copy/move assignments).
// - Uses placement new and manual destruction; no requirement that T be default-constructible.
// - push_back, emplace_back, remove_at, operator[], at, size, capacity provided.
// - Compiles with C++11 and later (uses variadic templates for emplace_back).

#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <cstddef>
#include <new>        // operator new
#include <utility>    // std::move, std::forward
#include <stdexcept>  // std::out_of_range
#include <initializer_list>
#include <algorithm>  // std::swap
#include <type_traits>

template<typename T>
class DynamicArray {
public:
    // Constructors / destructor
    DynamicArray()
        : data_(nullptr), size_(0), capacity_(0) {}

    DynamicArray(std::initializer_list<T> init)
        : DynamicArray() {
        reserve(init.size());
        for (const T& v : init) emplace_back(v);
    }

    ~DynamicArray() {
        clear();
        ::operator delete(data_);
        data_ = nullptr;
        capacity_ = 0;
    }

    // Copy constructor
    DynamicArray(const DynamicArray& other)
        : data_(nullptr), size_(0), capacity_(0) {
        if (other.size_ > 0) {
            reserve(other.size_);
            try {
                for (std::size_t i = 0; i < other.size_; ++i)
                    new (ptr_at(i)) T(other[i]);
                size_ = other.size_;
            } catch (...) {
                // destroy any constructed ones and free
                for (std::size_t j = 0; j < size_; ++j)
                    ptr_at(j)->~T();
                ::operator delete(data_);
                data_ = nullptr;
                size_ = capacity_ = 0;
                throw;
            }
        }
    }

    // Move constructor
    DynamicArray(DynamicArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = other.capacity_ = 0;
    }

    // Copy assignment
    DynamicArray& operator=(const DynamicArray& other) {
        if (this == &other) return *this;
        // Strong exception guarantee: make a copy then swap
        DynamicArray temp(other);
        swap(temp);
        return *this;
    }

    // Move assignment
    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this == &other) return *this;
        clear();
        ::operator delete(data_);
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = other.capacity_ = 0;
        return *this;
    }

    // Capacity / size
    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }

    void reserve(std::size_t new_cap) {
        if (new_cap <= capacity_) return;
        reallocate(new_cap);
    }

    void shrink_to_fit() {
        if (size_ == capacity_) return;
        if (size_ == 0) {
            clear();
            ::operator delete(data_);
            data_ = nullptr;
            capacity_ = 0;
            return;
        }
        reallocate(size_);
    }

    void clear() noexcept {
        for (std::size_t i = 0; i < size_; ++i) {
            ptr_at(i)->~T();
        }
        size_ = 0;
    }

    // Add elements
    void push_back(const T& value) {
        if (size_ == capacity_) grow();
        new (ptr_at(size_)) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) grow();
        new (ptr_at(size_)) T(std::move(value));
        ++size_;
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (size_ == capacity_) grow();
        new (ptr_at(size_)) T(std::forward<Args>(args)...);
        ++size_;
    }

    // Remove element at index (order-preserving: shifts subsequent elements left)
    void remove_at(std::size_t index) {
        if (index >= size_) throw std::out_of_range("remove_at: index out of range");
        // destroy element at index, then move-construct shift for following elements
        ptr_at(index)->~T();
        for (std::size_t i = index; i + 1 < size_; ++i) {
            // Move element i+1 into slot i
            T* src = ptr_at(i + 1);
            new (ptr_at(i)) T(std::move(*src));
            src->~T();
        }
        --size_;
    }

    // Access
    T& operator[](std::size_t index) noexcept { return *ptr_at(index); }
    const T& operator[](std::size_t index) const noexcept { return *ptr_at(index); }

    T& at(std::size_t index) {
        if (index >= size_) throw std::out_of_range("at: index out of range");
        return *ptr_at(index);
    }

    const T& at(std::size_t index) const {
        if (index >= size_) throw std::out_of_range("at: index out of range");
        return *ptr_at(index);
    }

    // Swap
    void swap(DynamicArray& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    // raw storage pointer (allocated via operator new). Type T* used for pointer arithmetic,
    // but elements are constructed in-place.
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

    // helper to get pointer to element slot index
    T* ptr_at(std::size_t index) noexcept {
        return reinterpret_cast<T*>(reinterpret_cast<char*>(data_) + index * sizeof(T));
    }
    const T* ptr_at(std::size_t index) const noexcept {
        return reinterpret_cast<const T*>(reinterpret_cast<const char*>(data_) + index * sizeof(T));
    }

    void grow() {
        std::size_t newcap = capacity_ == 0 ? 1 : capacity_ * 2;
        reallocate(newcap);
    }

    void reallocate(std::size_t new_cap) {
        // allocate raw memory
        T* new_mem = static_cast<T*>(::operator new(new_cap * sizeof(T)));
        std::size_t i = 0;
        try {
            // move-construct existing elements into new memory
            for (; i < size_; ++i) {
                new (reinterpret_cast<char*>(new_mem) + i * sizeof(T)) T(std::move(*ptr_at(i)));
            }
        } catch (...) {
            // if construction fails, destroy any constructed in new_mem and free
            for (std::size_t j = 0; j < i; ++j) {
                reinterpret_cast<T*>(reinterpret_cast<char*>(new_mem) + j * sizeof(T))->~T();
            }
            ::operator delete(new_mem);
            throw;
        }

        // destroy old elements and free old memory
        for (std::size_t j = 0; j < size_; ++j) {
            ptr_at(j)->~T();
        }
        ::operator delete(data_);

        data_ = new_mem;
        capacity_ = new_cap;
        // size_ unchanged
    }
};

#endif // DYNAMIC_ARRAY_H


#include <iostream>
#include "DynamicArray.h"

struct MyObject {
    int id;
    MyObject(int id_) : id(id_) {
        std::cout << "MyObject(" << id << ") constructed\n";
    }
    MyObject(const MyObject& other) : id(other.id) {
        std::cout << "MyObject(" << id << ") copy-constructed\n";
    }
    MyObject(MyObject&& other) noexcept : id(other.id) {
        std::cout << "MyObject(" << id << ") move-constructed\n";
        other.id = -1;
    }
    MyObject& operator=(const MyObject& other) {
        id = other.id;
        std::cout << "MyObject(" << id << ") copy-assigned\n";
        return *this;
    }
    MyObject& operator=(MyObject&& other) noexcept {
        id = other.id;
        other.id = -1;
        std::cout << "MyObject(" << id << ") move-assigned\n";
        return *this;
    }
    ~MyObject() {
        std::cout << "MyObject(" << id << ") destroyed\n";
    }
};

int main() {
    DynamicArray<MyObject> arr;
    arr.emplace_back(1);
    arr.emplace_back(2);
    arr.push_back(MyObject(3));
    std::cout << "Array size: " << arr.size() << ", capacity: " << arr.capacity() << "\n";

    std::cout << "Access by index:\n";
    for (std::size_t i = 0; i < arr.size(); ++i) {
        std::cout << "arr[" << i << "].id = " << arr[i].id << "\n";
    }

    std::cout << "Removing index 1\n";
    arr.remove_at(1);

    std::cout << "After remove:\n";
    for (std::size_t i = 0; i < arr.size(); ++i) {
        std::cout << "arr[" << i << "].id = " << arr.at(i).id << "\n";
    }

    std::cout << "Done. Exiting main, destructor will run.\n";
    return 0;
}