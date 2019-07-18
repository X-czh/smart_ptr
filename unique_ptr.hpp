#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP

#include <cstddef>
#include <utility>
#include <functional>

#include "default_delete.hpp"

using std::nullptr_t;
using std::swap;
using std::less; // requires C++14 for TODO 

namespace smart_ptr {

// For single object only, not for array
template <typename T, typename D = default_delete>
class unique_ptr {
public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = D;

    // Constructors

    /// Default constructor, creates a unique_ptr that owns nothing
    constexpr unique_ptr() noexcept = default;

    /// Constructs with nullptr, creates a unique_ptr that owns nothing
    constexpr unique_ptr(nullptr_t) noexcept { }

    /// Takes ownership from a pointer
    explicit unique_ptr(pointer p) noexcept
    : _ptr{p} { }

    /// Takes ownership from a pointer, supplied with a deleter
    explicit unique_ptr(pointer p, deleter_type d) noexcept
    : _ptr{p}, _deleter{d} { }

    // Copy

    /// Disables copy constructor
    unique_ptr(const unique_ptr&) = delete;

    /// Disables copy assignment
    unique_ptr& operator=(const unique_ptr&) = delete;

    // Move

    /// Move constructor: takes ownership from a unique_ptr of the same type
    unique_ptr(unique_ptr&& up) noexcept
    : _ptr{up.release()}, _deleter{up.get_deleter()} { }

    /// Move constructor: takes ownership from a unique_ptr of a different type
    template <typename U, typename E>
    unique_ptr(unique_ptr<U, E>&& up) noexcept
    : _ptr{up.release()}, _deleter{up.get_deleter()} { }

    /// Move assignment: takes ownership from a unique_ptr of the same type
    unique_ptr& operator=(unique_ptr&& up) noexcept
    {
        _deleter = up.get_deleter();
        _ptr = up.release();
        return *this;
    }

    /// Move assignment: takes ownership from a unique_ptr of a different type
    template <typename U, typename E>
    unique_ptr& operator=(unique_ptr<U, E>&& up) noexcept
    {
        _deleter = up.get_deleter();
        _ptr = up.release();
        return *this;
    }

    /// Destructor, invokes the deleter if the stored pointer is not null
    ~unique_ptr() noexcept
    {
        if (_ptr != nullptr) {
            _deleter(_ptr);
        }
    }

    /// Resets unique_ptr to empty if assigned to nullptr 
    unique_ptr&
    operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    // Observers

    /// Dereferences pointer to the managed object
    element_type&
    operator*() const { return *_ptr; }

    /// Dereferences pointer to the managed object
    pointer 
    operator->() const { return &(this->operator*()); }

    /// Gets the stored pointer
    pointer
    get() const noexcept
    { return _ptr; }

    /// Gets a reference to the stored deleter
    deleter_type&
    get_deleter() noexcept
    { return _deleter; }

    /// Gets a const reference to the stored deleter
    const deleter_type&
    get_deleter() const noexcept
    { return _deleter; }

    /// Checks if there is an associated managed object
    explicit operator bool() const noexcept
    { return (_ptr) ? true : false; }

    // Modifiers

    /// Releases ownership to the returned raw pointer
    pointer
    release() noexcept
    {
        pointer cp = _ptr;
        _ptr = nullptr;
        return cp;
    }

    /// Resets unique_ptr to empty and take ownership from a pointer
    void
    reset(pointer p) noexcept
    {
        if (_ptr != nullptr) {
            _deleter(_ptr); 
        }
        _ptr = p;
    }

    /// Resets unique_ptr to empty
    void
    reset() noexcept
    {
        if (_ptr != nullptr) {
            _deleter(_ptr);
        }
        _ptr = nullptr;
    }

    /// Swaps with another unique_ptr
    void
    swap(unique_ptr& up) noexcept
    {
        swap(_ptr, up.get());
        swap(_deleter, up.get_deleter());
    }
private:
    pointer _ptr;
    deleter_type _deleter;
};

/// Creates a unique_ptr that manages a new object 
template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
    return unique_ptr<T>{new T{args...}};
}

/// Swaps with another unique_ptr
template<typename T, typename D>
inline void
swap(unique_ptr<T, D>& up1, unique_ptr<T, D>& up2)
{ up1.swap(up2); }

// Operator == overloading

template <typename T, typename D,
          typename U, typename E>
    inline bool
    operator==(const unique_ptr<T, D>& up1,
               const unique_ptr<U, E>& up2)
    { return up1.get() == up2.get(); }

template<typename T, typename D>
    inline bool
    operator==(const unique_ptr<T, D>& up, nullptr_t) noexcept
    { return !up; }

template<typename T, typename D>
    inline bool
    operator==(nullptr_t, const unique_ptr<T, D>& up) noexcept
    { return !up; }

// Operator != overloading

template <typename T, typename D,
          typename U, typename E>
    inline bool
    operator!=(const unique_ptr<T, D>& up1,
               const unique_ptr<U, E>& up2)
    { return up1.get() != up2.get(); }

template<typename T, typename D>
    inline bool
    operator!=(const unique_ptr<T, D>& up, nullptr_t) noexcept
    { return bool{up}; }

template<typename T, typename D>
    inline bool
    operator!=(nullptr_t, const unique_ptr<T, D>& up) noexcept
    { return bool{up}; }

// Operator < overloading

template <typename T, typename D,
          typename U, typename E>
    inline bool
    operator<(const unique_ptr<T, D>& up1,
               const unique_ptr<U, E>& up2)
    { return less<>(up1.get(), up2.get()); }

template <typename T, typename D>
    inline bool
    operator<(const unique_ptr<T, D>& up, nullptr_t)
    { return less<>(up.get(), nullptr); }

template <typename T, typename D>
    inline bool
    operator<(nullptr_t, const unique_ptr<T, D>& up)
    { return less<>(nullptr, up.get()); }

// Operator <= overloading

template <typename T, typename D,
          typename U, typename E>
    inline bool
    operator<=(const unique_ptr<T, D>& up1,
               const unique_ptr<U, E>& up2)
    { return !(up2.get() < up1.get()); }

template <typename T, typename D>
    inline bool
    operator<=(const unique_ptr<T, D>& up, nullptr_t)
    { return !(nullptr < up.get()); }

template <typename T, typename D>
    inline bool
    operator<=(nullptr_t, const unique_ptr<T, D>& up)
    { return !(up.get() < nullptr); }

// Operator > overloading

template <typename T, typename D,
          typename U, typename E>
    inline bool
    operator>(const unique_ptr<T, D>& up1,
               const unique_ptr<U, E>& up2)
    { return up2.get() < up1.get(); }

template <typename T, typename D>
    inline bool
    operator>(const unique_ptr<T, D>& up, nullptr_t)
    { return nullptr < up.get(); }

template <typename T, typename D>
    inline bool
    operator>(nullptr_t, const unique_ptr<T, D>& up)
    { return up.get() < nullptr; }

// Operator >= overloading

template <typename T, typename D,
          typename U, typename E>
    inline bool
    operator>=(const unique_ptr<T, D>& up1,
               const unique_ptr<T, D>& up2)
    { return !(up1.get() < up2.get()); }

template <typename T, typename D>
    inline bool
    operator>=(const unique_ptr<T, D>& up, nullptr_t)
    { return !(up.get() < nullptr); }

template <typename T, typename D>
    inline bool
    operator>=(nullptr_t, const unique_ptr<T, D>& up)
    { return !(nullptr < up.get()); }

} // namespace smart_ptr

#endif
