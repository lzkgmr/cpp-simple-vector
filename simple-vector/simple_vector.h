#pragma once

#include <cassert>
#include <initializer_list>
#include <iostream>
#include <algorithm>
#include <iterator>
#include "array_ptr.h"

using namespace std;

class ReserveProxyObj {
public:
    explicit ReserveProxyObj(size_t capacity_to_reserve)
        :capacity_(capacity_to_reserve)
    {
    }
 
    size_t ReserveCapasity() {
        return capacity_;
    }
 
private:
    size_t capacity_;
 
};
 
ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}


template <typename Type>
class SimpleVector
{

public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;


    explicit SimpleVector(size_t size)
            : SimpleVector(size, Type{}) 
    {
    }

    SimpleVector(ReserveProxyObj capacity_to_reserve) {
        Reserve(capacity_to_reserve.ReserveCapasity());
    }

    SimpleVector(size_t size, const Type& value) : capacity_(size), size_(size)
    {
        ArrayPtr<Type> new_array(size);
        items_.swap(new_array);
        fill(items_.Get(), items_.Get() + size, value);
    }

    SimpleVector(std::initializer_list<Type> init) : capacity_(init.size()), size_(init.size()) 
    {
        ArrayPtr<Type> new_array(init.size());
        items_.swap(new_array);
        copy(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(const SimpleVector& other) : capacity_(other.capacity_), size_(other.size_) 
    {
        ArrayPtr<Type> new_array(other.size_);
        copy(other.begin(), other.end(), new_array.Get());
        items_.swap(new_array);
    }

    SimpleVector(SimpleVector&& other) : items_(move(other.items_)), capacity_(other.capacity_), size_(other.size_) 
    {
        other.capacity_ = 0;
        other.size_ = 0;
    }

    //~SimpleVector();

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector<Type> new_vec(rhs);
            this->swap(new_vec);
        }
        return *this;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
 
        ArrayPtr<Type> tmp_items(new_capacity);
        for (std::size_t i{}; i < size_; ++i) {
            tmp_items[i] = items_[i];
        }
        items_.swap(tmp_items);
        capacity_ = new_capacity;
    }

    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            IncreaseCapacity(size_ + 1);
        }
        items_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        size_t new_capacity = size_ == 0 ? 1 : capacity_ * 2;
        ArrayPtr<Type> array(new_capacity);
        for (size_t i = 0; i < size_; ++i) {
            array.Get()[i] = move(items_.Get()[i]);
        }

        array.Get()[size_] = move(item);

        items_.swap(array);
        ++size_;
        capacity_ = max(capacity_, new_capacity);
    }

    
Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        
        size_t new_capacity = size_ == capacity_ ? (size_ == 0 ? 1 : capacity_ * 2) : capacity_;
        int ins_pos = 0;
        ArrayPtr<Type> array(new_capacity);
        if (size_ == 0) {
            array.Get()[0] = value;
        } else {
            int i = 0;
            for (auto it = begin(); it != end(); ++it) {
                if (it == pos) {
                    ins_pos = i;
                    array.Get()[i] = value;
                    ++i;
                }
                array.Get()[i] = *it;
                ++i;
            }
            if (pos == end()) {
                array.Get()[size_] = value;
                ins_pos = size_;
            }
        }

        items_.swap(array);
        ++size_;
        capacity_ = new_capacity;
        return (begin() + ins_pos);
    }


Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());

        size_t new_capacity = size_ == capacity_ ? (size_ == 0 ? 1 : capacity_ * 2) : capacity_;
        int ins_pos = 0;
        ArrayPtr<Type> array(new_capacity);
        if (size_ == 0) {
            array.Get()[0] = move(value);
        } else {
            int i = 0;
            for (auto it = begin(); it != end(); ++it) {
                if (it == pos) {
                    ins_pos = i;
                    array.Get()[i] = move(value);
                    ++i;
                }
                array.Get()[i] = move(*it);
                ++i;
            }
            if (pos == end()) {
                array.Get()[size_] = move(value);
                ins_pos = size_;
            }
        }
        items_.swap(array);
        ++size_;
        capacity_ = new_capacity;
        return (begin() + ins_pos);
    }
    
    void PopBack() noexcept {
        --size_;
    }

    Iterator Erase(Iterator&& pos) {
        assert(size_ > 0);
        assert(pos >= begin() && pos <= end());
        
        int dis = distance(begin(), pos);
        if (pos != end()) {
            for (; pos != end() - 1; ++pos) {
                *pos = move(*(pos + 1));
            }
        }
        --size_;
        return &items_.Get()[dis];
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index <= size_);
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index <= size_);
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw out_of_range("index more than size");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw out_of_range("index more than size");
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size < size_ || new_size <= capacity_) {
            size_ = new_size;
        } else if (new_size > capacity_) {
            ArrayPtr<Type> array(new_size);
            for (size_t i = 0; i < new_size; ++i) {
                array.Get()[i] = i < size_ ? move(items_.Get()[i]) : Type();
            }
            items_.swap(array);
            size_ = new_size;
            capacity_ = new_size;
        }
    }

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return begin();
    }

    ConstIterator cend() const noexcept {
        return end();
    }
    
private:
    ArrayPtr<Type> items_;    
    size_t capacity_ = 0;
    size_t size_ = 0;  
    
    void IncreaseCapacity(size_t new_size) {
        size_t new_array_capacity = max(new_size, 2 * capacity_);
        ArrayPtr<Type> new_array(new_array_capacity);
        copy(items_.Get(), items_.Get() + size_, new_array.Get());
        items_.swap(new_array);
        capacity_ = new_array_capacity;
    }
    
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() == rhs.GetSize()) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
    return false;
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
