#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <utility>

struct ReserveProxyObj
{
    size_t capasity;

    ReserveProxyObj(size_t capacity_to_reserve) :capasity(capacity_to_reserve) {}
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) :_items(size), _size(size), _capacity(_size) {
        std::fill(begin(), end(), Type());
    }

    SimpleVector(size_t size, const Type& value) :_items(size), _size(size), _capacity(_size) {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init) :_items(init.size()), _size(init.size()), _capacity(init.size()) {
        std::move(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type> tmp(other._size);
        std::copy(other.begin(), other.end(), tmp.Get());

        _items.swap(tmp);
        _size = other._size;
        _capacity = other._capacity;
    }

    SimpleVector(ReserveProxyObj object) :_items(object.capasity), _capacity(object.capasity)
    {
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (*this != rhs)
        {
            SimpleVector temp(rhs);
            swap(temp);
        }

        return *this;
    }

    SimpleVector(SimpleVector&& other)
    {
        _items = std::move(other._items);
        _size = other._size;
        _capacity = other._capacity;
        other._size = 0;
        other._capacity = 0;
    }

    SimpleVector operator=(SimpleVector&& rhs)
    {
        if (*this != rhs || !rhs.IsEmpty())
        {
            //SimpleVector tmp(rhs);
            SimpleVector tmp = std::move(rhs);
            swap(tmp);
        }
        return *this;
    }

    void swap(SimpleVector& other) noexcept {
        _items.swap(other._items);
        std::swap(_size, other._size);
        std::swap(_capacity, other._capacity);
    }
    
    size_t GetSize() const noexcept {

        return _size;
    }

    size_t GetCapacity() const noexcept {

        return _capacity;
    }

    bool IsEmpty() const noexcept {
        return !_size;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < _size);
        return _items[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < _size);
        return _items[index];
    }

    Type& At(size_t index) {
        if (index >= _size)
        {
            throw std::out_of_range("Недопустимый индекс");
        }
        return _items[index];
    }

    const Type& At(size_t index) const {
        if (index >= _size)
        {
            throw std::out_of_range("Недопустимый индекс");
        }
        return _items[index];
    }

    void Clear() noexcept {
        _size = 0;
    }

    void Resize(size_t new_size) {
        if (new_size < _size)
        {
            _size = new_size;
            return;
        }
        if (new_size < _capacity)
        {

            std::generate(end(), _items.Get() + _capacity, [] {return Type{}; });
            _size = new_size;
            return;
        }

        size_t new_cap = std::max(new_size, 2 * _capacity);
        ArrayPtr<Type> tmp(new_cap);
        std::generate(tmp.Get() + _size, tmp.Get() + new_cap, [] {return Type{}; });
        std::move(begin(), end(), tmp.Get());
        _items.swap(tmp);
        _size = new_size;
        _capacity = new_cap;
    }

    void PushBack(const Type& item) {
        if (_size < _capacity)
        {
            _items[_size] = item;
            ++_size;
            return;
        }
        size_t new_capacity = (_capacity == 0) ? 1 : _capacity * 2;
        ArrayPtr<Type> tmp(new_capacity);
        std::copy(begin(), end(), tmp.Get());
        tmp[_size] = item;
        _items.swap(tmp);
        ++_size;
        _capacity = new_capacity;
    }
    void PushBack(Type&& item) {
        if (_size < _capacity)
        {
            _items[_size] = std::move(item);
            ++_size;
            return;
        }
        size_t new_capacity = (_capacity == 0) ? 1 : _capacity * 2;
        ArrayPtr<Type> tmp(new_capacity);
        std::move(begin(), end(), tmp.Get());
        tmp[_size] = std::move(item);
        _items.swap(tmp);
        ++_size;
        _capacity = new_capacity;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        auto dist = std::distance(cbegin(), pos);

        if (_size < _capacity)
        {
            std::copy_backward(Iterator(pos), end(), end() + 1);
            _items[dist] = value;
            ++_size;
            return _items.Get() + dist;
        }

        size_t new_capacity = (_capacity == 0) ? 1 : _capacity * 2;
        ArrayPtr<Type> tmp(new_capacity);
        std::copy(begin(), Iterator(pos), tmp.Get());
        tmp[dist] = value;
        std::copy(begin() + dist, end(), tmp.Get() + dist + 1);
        _items.swap(tmp);
        ++_size;
        _capacity = new_capacity;
        return _items.Get() + dist;
    }

    Iterator Insert(ConstIterator pos,Type&& value) {
        assert(pos >= begin() && pos <= end());
        auto dist = std::distance(cbegin(), pos);

        if (_size < _capacity)
        {
            
            std::move_backward(Iterator(pos), end(), end() + 1);
            _items[dist] = std::move(value);
            ++_size;
            return _items.Get() + dist;
        }

        size_t new_capacity = (_capacity == 0) ? 1 : _capacity * 2;
        ArrayPtr<Type>tmp(new_capacity);
        std::move(begin(), begin() + dist, tmp.Get());
        tmp[dist] = std::move(value);

        std::move(begin() + dist, end(), tmp.Get() + dist + 1);
        _items.swap(tmp);
        ++_size;
        _capacity = new_capacity;
        return _items.Get() + dist;
    }

    void PopBack() noexcept {
        if (_size == 0) return;
        --_size;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        auto dist = std::distance(cbegin(), pos);
        std::move(begin()+dist+1,end(),begin()+dist);
        --_size;
        return _items.Get() + dist;
    }

    void Reserve(size_t new_capacity)
    {
        if (new_capacity > _capacity)
        {
            ArrayPtr<Type> tmp(new_capacity);
            std::move(begin(), end(), tmp.Get());
            _items.swap(tmp);
            _capacity = new_capacity;
        }
    }

    Iterator begin() noexcept {
        return Iterator{ _items.Get() };
    }

    Iterator end() noexcept {
        return Iterator{ _items.Get() + _size };
    }

    ConstIterator begin() const noexcept {
        return ConstIterator{ _items.Get() };

    }

    ConstIterator end() const noexcept {
        return ConstIterator{ _items.Get() + _size };
    }

    ConstIterator cbegin() const noexcept {
        return ConstIterator{ _items.Get() };
    }

    ConstIterator cend() const noexcept {
        return ConstIterator{ _items.Get() + _size };
    }

    ~SimpleVector()
    {

    }
private:
    ArrayPtr<Type> _items;
    size_t _size = 0;
    size_t _capacity = 0;


};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) return false;

    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
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
    return !(rhs > lhs);
}
