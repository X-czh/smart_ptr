// unique_ptr implementation

#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP 1

#include <cstddef>              /// nullptr_t
#include <utility>              /// move, forward, swap
#include <functional>           /// less, hash
#include <type_traits>          /// remove_extent, conditional, is_reference
                                    /// common_type 

#include "default_delete.hpp"

namespace smart_ptr {

// 20.7.1.2 unique_ptr for single objects

template<typename T, typename D = default_delete<T>>
class unique_ptr {
public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = D;

    // 20.7.1.2.1, constructors

    /// Default constructor, creates a unique_ptr that owns nothing
    constexpr unique_ptr() noexcept = default;

    /// Constructs with nullptr, creates a unique_ptr that owns nothing
    constexpr unique_ptr(std::nullptr_t) noexcept
    { }

    /// Takes ownership from a pointer
    explicit unique_ptr(pointer p) noexcept
    : _ptr{p}
    { }

    /// Takes ownership from a pointer, supplied with a custom deleter
    /// d: a reference to a deleter.
    unique_ptr(pointer p, 
        typename std::conditional<std::is_reference<deleter_type>::value,
	        deleter_type, const deleter_type&>::type d) noexcept
    : _ptr{p}, _deleter{d}
    { }

    /// Takes ownership from a pointer, supplied with a custom deleter
    /// d: an rvalue reference to a deleter.
    /// Not permitted if deleter_type is an lvalue reference.
    unique_ptr(pointer p,
        typename std::remove_reference<deleter_type>::type&& d) noexcept
    : _ptr{p}, _deleter{std::move(d)}
    {
        static_assert(!std::is_reference<deleter_type>::value,
		    "rvalue deleter bound to reference");
    }

    /// Move constructor: takes ownership from a unique_ptr of the same type
    unique_ptr(unique_ptr&& up) noexcept
    : _ptr{up.release()}, _deleter{std::forward<deleter_type>(up.get_deleter())}
    { }

    /// Move constructor: takes ownership from a unique_ptr of a different type
    template <typename U, typename E>
    unique_ptr(unique_ptr<U, E>&& up) noexcept
    : _ptr{up.release()}, _deleter{std::forward<deleter_type>(up.get_deleter())}
    { }

    // 20.7.1.2.2, destructor

    /// Invokes the deleter if the stored pointer is not null
    ~unique_ptr() noexcept
    { if (_ptr) _deleter(_ptr); }

    // 20.7.1.2.3, assignment

    /// Move assignment: takes ownership from a unique_ptr of the same type
    unique_ptr& operator=(unique_ptr&& up) noexcept
    {
        reset(up.release());
	    _deleter = up.get_deleter();
        return *this;
    }

    /// Move assignment: takes ownership from a unique_ptr of a different type
    template <typename U, typename E>
    unique_ptr& operator=(unique_ptr<U, E>&& up) noexcept
    {
        reset(up.release());
	    _deleter = up.get_deleter();
        return *this;
    }

    /// Resets unique_ptr to empty if assigned to nullptr 
    unique_ptr&
    operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    // 20.7.1.2.4, observers

    /// Dereferences pointer to the managed object
    element_type&
    operator*() const noexcept
    { return *_ptr; }

    /// Dereferences pointer to the managed object
    pointer 
    operator->() const noexcept
    { return _ptr; }

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

    // 20.7.1.2.5 modifiers

    /// Releases ownership to the returned raw pointer
    pointer
    release() noexcept
    {
        pointer cp = _ptr;
        _ptr = nullptr;
        return cp;
    }

    /// Resets unique_ptr to empty and takes ownership from a pointer
    void
    reset(pointer p) noexcept
    {
        if (_ptr) _deleter(_ptr); 
        _ptr = p;
    }

    /// Resets unique_ptr to empty
    void
    reset() noexcept
    {
        if (_ptr) _deleter(_ptr);
        _ptr = pointer{}; /// probably nullptr
    }

    /// Swaps with another unique_ptr
    void
    swap(unique_ptr& up) noexcept
    {
        using std::swap;
        swap(_ptr, up._ptr);
        swap(_deleter, up._deleter);
    }
    
    // Disable copy from lvalue

    /// Disables copy constructor
    unique_ptr(const unique_ptr&) = delete;

    /// Disables copy assignment
    unique_ptr& operator=(const unique_ptr&) = delete;
private:
    pointer _ptr;
    deleter_type _deleter;
};


// 20.7.1.3 unique_ptr for array objects with a runtime length

template <typename T, typename D>
class unique_ptr<T[], D>
{
public:
    using pointer = T*;
    using element_type = T;
    using deleter_type = D;

    // 20.7.1.3.1, constructors

    /// Default constructor, creates a unique_ptr that owns nothing
    constexpr unique_ptr() noexcept = default;

    /// Constructs with nullptr, creates a unique_ptr that owns nothing
    constexpr unique_ptr(std::nullptr_t) noexcept
    { }

    /// Takes ownership from a pointer
    explicit unique_ptr(pointer p) noexcept
    : _ptr{p}
    { }

    /// Takes ownership from a pointer, supplied with a custom deleter
    /// d: a reference to a deleter.
    unique_ptr(pointer p, 
        typename std::conditional<std::is_reference<deleter_type>::value,
	        deleter_type, const deleter_type&>::type d) noexcept
    : _ptr{p}, _deleter{d}
    { }

    /// Takes ownership from a pointer, supplied with a custom deleter
    /// d: an rvalue reference to a deleter.
    /// Not permitted if deleter_type is an lvalue reference.
    unique_ptr(pointer p,
        typename std::remove_reference<deleter_type>::type&& d) noexcept
    : _ptr{p}, _deleter{std::move(d)}
    {
        static_assert(!std::is_reference<deleter_type>::value,
		    "rvalue deleter bound to reference");
    }

    /// Move constructor: takes ownership from a unique_ptr of the same type
    unique_ptr(unique_ptr&& up) noexcept
    : _ptr{up.release()}, _deleter{up.get_deleter()}
    { }

    // destructor
    
    /// Invokes the deleter if the stored pointer is not null
    ~unique_ptr() noexcept
    { if (_ptr) _deleter(_ptr); }

    // assignment

    /// Move assignment: takes ownership from a unique_ptr of the same type
    unique_ptr& operator=(unique_ptr&& up) noexcept
    {
        reset(up.release());
	    _deleter = up.get_deleter();
        return *this;
    }

    /// Resets unique_ptr to empty if assigned to nullptr 
    unique_ptr&
    operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    // 20.7.1.3.2, observers

    /// Index operator, dereferencing operators are not provided,
    /// bound range is not checked
    element_type&
    operator[](size_t i) const noexcept
    { return _ptr[i]; }

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

    // 20.7.1.3.3 modifiers

    /// Releases ownership to the returned raw pointer
    pointer
    release() noexcept
    {
        pointer cp = _ptr;
        _ptr = nullptr;
        return cp;
    }

    /// Resets unique_ptr to empty and takes ownership from a pointer
    void
    reset(pointer p) noexcept
    {
        if (_ptr) _deleter(_ptr); 
        _ptr = p;
    }

    /// Resets unique_ptr to empty
    void
    reset() noexcept
    {
        if (_ptr) _deleter(_ptr);
        _ptr = pointer{}; /// probably nullptr
    }

    /// Swaps with another unique_ptr
    void
    swap(unique_ptr& up) noexcept
    {
        using std::swap;
        swap(_ptr, up._ptr);
        swap(_deleter, up._deleter);
    }
    
    // Disable copy from lvalue
 
    /// Disables copy constructor
    unique_ptr(const unique_ptr&) = delete;

    /// Disables copy assignment
    unique_ptr& operator=(const unique_ptr&) = delete;

private:
    pointer _ptr;
    deleter_type _deleter;
};

/* added in C++14 */
// make_unique: creates a unique pointer that manages a new object

template<typename T>
    struct _Unique_if {
        using _Single_object = unique_ptr<T> ;
    };

template<typename T>
    struct _Unique_if<T[]> {
        using _Unknown_bound = unique_ptr<T[]> ;
    };

template<typename T, std::size_t N>
    struct _Unique_if<T[N]> {
        using _Known_bound = void;
    };

/// Only for non-array types
template<typename T, typename... Args>
    typename _Unique_if<T>::_Single_object
    make_unique(Args&&... args) {
        return unique_ptr<T>{new T{std::forward<Args>(args)...}};
    }

/// Only for array types with unknown bound
template<typename T>
    typename _Unique_if<T>::_Unknown_bound
    make_unique(std::size_t n) {
        using U = typename std::remove_extent<T>::type;
        return unique_ptr<T>{new U[n]{}};
    }

/// Only for array types with known bound: unspecified
template<typename T, typename... Args>
    typename _Unique_if<T>::_Known_bound
    make_unique(Args&&...) = delete;

// 20.7.1.4 unique_ptr specialized algorithms

/// Operator == overloading
template<typename T, typename D,
          typename U, typename E>
    inline bool
    operator==(const unique_ptr<T, D>& up1,
               const unique_ptr<U, E>& up2)
    { return up1.get() == up2.get(); }

template<typename T, typename D>
    inline bool
    operator==(const unique_ptr<T, D>& up, std::nullptr_t) noexcept
    { return !up; }

template<typename T, typename D>
    inline bool
    operator==(std::nullptr_t, const unique_ptr<T, D>& up) noexcept
    { return !up; }

/// Operator != overloading
template<typename T, typename D,
          typename U, typename E>
    inline bool
    operator!=(const unique_ptr<T, D>& up1,
               const unique_ptr<U, E>& up2)
    { return up1.get() != up2.get(); }

template<typename T, typename D>
    inline bool
    operator!=(const unique_ptr<T, D>& up, std::nullptr_t) noexcept
    { return bool{up}; }

template<typename T, typename D>
    inline bool
    operator!=(std::nullptr_t, const unique_ptr<T, D>& up) noexcept
    { return bool{up}; }

/// Operator < overloading
template<typename T, typename D,
          typename U, typename E>
    inline bool
    operator<(const unique_ptr<T, D>& up1,
               const unique_ptr<U, E>& up2)
    {
        using _CT = typename std::common_type<
            typename unique_ptr<T, D>::pointer,
            typename unique_ptr<U, E>::pointer>::type;
        return std::less<_CT>()(up1.get(), up2.get()); }

template<typename T, typename D>
    inline bool
    operator<(const unique_ptr<T, D>& up, std::nullptr_t)
    {
        return std::less<typename unique_ptr<T, D>::pointer>()
            (up.get(), nullptr);
    }

template<typename T, typename D>
    inline bool
    operator<(std::nullptr_t, const unique_ptr<T, D>& up)
    {
        return std::less<typename unique_ptr<T, D>::pointer>()
            (nullptr, up.get());
    }

/// Operator <= overloading
template<typename T, typename D,
          typename U, typename E>
    inline bool
    operator<=(const unique_ptr<T, D>& up1,
               const unique_ptr<U, E>& up2)
    { return !(up2.get() < up1.get()); }

template<typename T, typename D>
    inline bool
    operator<=(const unique_ptr<T, D>& up, std::nullptr_t)
    { return !(nullptr < up.get()); }

template<typename T, typename D>
    inline bool
    operator<=(std::nullptr_t, const unique_ptr<T, D>& up)
    { return !(up.get() < nullptr); }

/// Operator > overloading
template<typename T, typename D,
          typename U, typename E>
    inline bool
    operator>(const unique_ptr<T, D>& up1,
               const unique_ptr<U, E>& up2)
    { return up2.get() < up1.get(); }

template<typename T, typename D>
    inline bool
    operator>(const unique_ptr<T, D>& up, std::nullptr_t)
    { return nullptr < up.get(); }

template<typename T, typename D>
    inline bool
    operator>(std::nullptr_t, const unique_ptr<T, D>& up)
    { return up.get() < nullptr; }

/// Operator >= overloading
template<typename T, typename D,
          typename U, typename E>
    inline bool
    operator>=(const unique_ptr<T, D>& up1,
               const unique_ptr<U, E>& up2)
    { return !(up1.get() < up2.get()); }

template<typename T, typename D>
    inline bool
    operator>=(const unique_ptr<T, D>& up, std::nullptr_t)
    { return !(up.get() < nullptr); }

template<typename T, typename D>
    inline bool
    operator>=(std::nullptr_t, const unique_ptr<T, D>& up)
    { return !(nullptr < up.get()); }

/// Swaps with another unique_ptr
template<typename T, typename D>
    inline void
    swap(unique_ptr<T, D>& up1, unique_ptr<T, D>& up2)
    { up1.swap(up2); }

/* added in C++20 */
/// unique_ptr I/O

template<class E, class T, class Y>
    std::basic_ostream<E, T>&
    operator<<(std::basic_ostream<E, T>& os, const unique_ptr<Y>& up)
    {
        os << up.get();
        return os;
    }

} // namespace smart_ptr

namespace std {

// 20.7.2.6 Smart pointer hash support

// Template specialization of std::hash for smart_ptr::unique_ptr<T>

/**
 * Allows users to obtain hashes of objects of type smart_ptr::unique_ptr<T>,
 *  which can be used to store those objects in an unordered container.
 * 
 * This specialization ensures that std::hash<smart_ptr::unique_ptr<T>>()(up) ==
 *  hash<typename smart_ptr::unique_ptr<T>::element_type*>()(up.get()).
 */

template<typename T>
struct hash<smart_ptr::unique_ptr<T>> {
    using result_type = size_t;
    using argument_type = smart_ptr::unique_ptr<T>;

    size_t
    operator()(const smart_ptr::unique_ptr<T>& up) const {
        return hash<typename smart_ptr::unique_ptr<T>::element_type*>()(up.get());
    }
};

} // namespace std

#endif
