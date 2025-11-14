#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <memory>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <cstddef>

template <typename T>
class DynamicArray {
public:
    // --- Constructors / Destructor / Assignment ---
    DynamicArray() noexcept
        : data_(nullptr), size_(0), capacity_(0) {}

    DynamicArray(const DynamicArray& other)
        : data_(nullptr), size_(0), capacity_(0)
    {
        if (other.size_ > 0) {
            data_ = alloc_.allocate(other.capacity_);
            std::uninitialized_copy(other.data_, other.data_ + other.size_, data_);
            size_ = other.size_;
            capacity_ = other.capacity_;
        }
    }

    DynamicArray(DynamicArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    DynamicArray& operator=(DynamicArray other) noexcept {
        swap(*this, other);
        return *this;
    }

    ~DynamicArray() {
        clear();
        if (data_) alloc_.deallocate(data_, capacity_);
    }

    // --- Modifiers ---
    // Append by copy
    void push_back(const T& value) {
        ensure_capacity_for_one_more();
        alloc_.construct(data_ + size_, value);
        ++size_;
    }

    // Append by move
    void push_back(T&& value) {
        ensure_capacity_for_one_more();
        alloc_.construct(data_ + size_, std::move(value));
        ++size_;
    }

    // Emplace construct in place
    template <typename... Args>
    T& emplace_back(Args&&... args) {
        ensure_capacity_for_one_more();
        alloc_.construct(data_ + size_, std::forward<Args>(args)...);
        ++size_;
        return data_[size_ - 1];
    }

    // Remove element at index. Throws out_of_range if invalid.
    // Requires T to be MoveAssignable (or CopyAssignable) so we can shift elements down.
    void remove_at(std::size_t index) {
        if (index >= size_) throw std::out_of_range("DynamicArray::remove_at: index out of range");
        // Shift elements left using move assignment
        for (std::size_t i = index; i + 1 < size_; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }
        // Destroy last element
        alloc_.destroy(data_ + size_ - 1);
        --size_;
    }

    // Remove last element (pop)
    void pop_back() {
        if (size_ == 0) throw std::out_of_range("DynamicArray::pop_back: empty");
        alloc_.destroy(data_ + size_ - 1);
        --size_;
    }

    // Clear all elements, keeps allocated capacity
    void clear() noexcept {
        if (data_) std::destroy(data_, data_ + size_);
        size_ = 0;
    }

    // Reserve capacity
    void reserve(std::size_t new_cap) {
        if (new_cap <= capacity_) return;
        reallocate(new_cap);
    }

    // Shrink to fit current size
    void shrink_to_fit() {
        if (capacity_ == size_) return;
        if (size_ == 0) {
            if (data_) {
                alloc_.deallocate(data_, capacity_);
                data_ = nullptr;
                capacity_ = 0;
            }
            return;
        }
        reallocate(size_);
    }

    // --- Element access ---
    T& operator[](std::size_t index) {
        return data_[index];
    }
    const T& operator[](std::size_t index) const {
        return data_[index];
    }

    T& at(std::size_t index) {
        if (index >= size_) throw std::out_of_range("DynamicArray::at: index out of range");
        return data_[index];
    }
    const T& at(std::size_t index) const {
        if (index >= size_) throw std::out_of_range("DynamicArray::at: index out of range");
        return data_[index];
    }

    // --- Capacity queries ---
    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }

    // Swap helper
    friend void swap(DynamicArray& a, DynamicArray& b) noexcept {
        using std::swap;
        swap(a.data_, b.data_);
        swap(a.size_, b.size_);
        swap(a.capacity_, b.capacity_);
    }

private:
    std::allocator<T> alloc_;
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

    void ensure_capacity_for_one_more() {
        if (size_ + 1 > capacity_) {
            std::size_t newCap = capacity_ == 0 ? 1 : capacity_ * 2;
            reallocate(newCap);
        }
    }

    void reallocate(std::size_t newCap) {
        T* newData = alloc_.allocate(newCap);
        if (data_) {
            // Move-construct existing elements into new storage
            std::uninitialized_move(data_, data_ + size_, newData);
            // Destroy old elements and deallocate
            std::destroy(data_, data_ + size_);
            alloc_.deallocate(data_, capacity_);
        }
        data_ = newData;
        capacity_ = newCap;
    }
};

#endif // DYNAMIC_ARRAY_H



#include <iostream>
#include <string>
#include "DynamicArray.h"

int main() {
    DynamicArray<std::string> arr;

    arr.push_back("apple");
    arr.push_back("banana");
    arr.emplace_back("cherry");
    arr.push_back(std::string("date"));

    std::cout << "Size after adds: " << arr.size() << "\n";
    for (std::size_t i = 0; i < arr.size(); ++i) {
        std::cout << i << ": " << arr[i] << "\n";
    }

    // Remove the second element ("banana")
    arr.remove_at(1);

    std::cout << "\nAfter remove_at(1):\n";
    for (std::size_t i = 0; i < arr.size(); ++i) {
        std::cout << i << ": " << arr.at(i) << "\n";
    }

    // Pop last
    arr.pop_back();
    std::cout << "\nAfter pop_back, size: " << arr.size() << "\n";

    return 0;
}