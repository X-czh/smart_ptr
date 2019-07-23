// shared_ptr implementation

#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP 1

#include <cstddef>      /// nullptr_t
#include <utility>      /// move, forward, swap
#include <functional>   /// less, hash
#include <iostream>     /// basic_ostream
#include <memory>       /// bad_weak_ptr

#include "control_block.hpp"
#include "bad_weak_ptr.hpp"
#include "weak_ptr.hpp"
#include "unique_ptr.hpp"

namespace smart_ptr {

// Forward declarations

template<typename T, typename D> class unique_ptr;
template<typename T> class weak_ptr;

// 20.7.2.2 Class template shared_ptr
// shared_ptr for single object

/**
 * NOT implemented: custom allocator support.
 * 
 * The allocator is intended to be used to allocate and deallocate
 *  internal shared_ptr details, not the object.
 */

template<typename T>
class shared_ptr {
public:
    template<typename U>
    friend class shared_ptr;

    template<typename U>
    friend class weak_ptr;

    template<typename D, typename U>
    friend D* get_deleter(const shared_ptr<U>&) noexcept;

    using element_type = T;
    using weak_type = weak_ptr<T>; /* added in C++17 */

    // 20.7.2.2.1, constructors

    /// Default constructor, creates a shared_ptr with no managed object
    /// Postconditions: use_count() == 0 && get() == 0.
    constexpr shared_ptr() noexcept
    : _ptr{},
      _control_block{}
    { }

    /// Constructs a shared_ptr with no managed object
    /// Postconditions: use_count() == 0 && get() == 0.
    constexpr shared_ptr(std::nullptr_t) noexcept
    : _ptr{},
      _control_block{}
    { }

    /// Constructs a shared_ptr with p as the pointer to the managed object
    /// Postconditions: use_count() == 1 && get() == p. 
    template<typename U>
    explicit shared_ptr(U* p)
    : _ptr{p},
      _control_block{new detail::control_block<U>{p}}
    { }

    /// Constructs a shared_ptr with p as the pointer to the managed object,
    ///     supplied with custom deleter
    /// Postconditions: use_count() == 1 && get() == p.
    template<typename U, typename D>
    shared_ptr(U* p, D d)
    : _ptr{p},
      _control_block{new detail::control_block<U, D>{p, std::move(d)}}
    { }

    /// Constructs a shared_ptr with p as the pointer to the managed object,
    ///     supplied with custom deleter and allocator
    /// Postconditions: use_count() == 1 && get() == p.
    template<typename U, typename D, typename A>
    shared_ptr(U* p, D d, A a) = delete;

    /// Constructs a shared_ptr with no managed object,
    ///     supplied with custom deleter
    /// Postconditions: use_count() == 1 && get() == 0.
    template<typename D>
    shared_ptr(std::nullptr_t p, D d)
    : _ptr{nullptr},
      _control_block{new detail::control_block<T, D>{p, std::move(d)}}
    { }

    /// Constructs a shared_ptr with no managed object,
    ///     supplied with custom deleter and allocator
    /// Postconditions: use_count() == 1 && get() == 0.
    template<typename D, typename A>
    shared_ptr(std::nullptr_t p, D d, A a) = delete;

    /// Aliasing constructor: constructs a shared_ptr instance that
    ///     stores p and shares ownership with sp
    /// Postconditions: use_count() == sp.use_count() && get() == p.
    template<typename U>
    shared_ptr(const shared_ptr<U>& sp, T *p) noexcept
    : _ptr{p},
      _control_block{sp._control_block}
    { if (_control_block) _control_block->inc_ref(); }

    /// Copy constructor: shares ownership of the object managed by sp
    /// Postconditions: use_count() == sp.use_count() && get() == sp.get().
    shared_ptr(const shared_ptr& sp) noexcept
    : _ptr{sp._ptr},
      _control_block{sp._control_block}
    { if (_control_block) _control_block->inc_ref(); }

    /// Copy constructor: shares ownership of the object managed by sp
    /// Postconditions: use_count() == sp.use_count() && get() == sp.get().
    template<typename U>
    shared_ptr(const shared_ptr<U>& sp) noexcept
    : _ptr{sp._ptr},
      _control_block{sp._control_block}
    { if (_control_block) _control_block->inc_ref();}

    /// Move constructor: Move-constructs a shared_ptr from sp
    /// Postconditions: *this shall contain the old value of sp.
    ///     sp shall be empty. sp.get() == 0.
    shared_ptr(shared_ptr&& sp) noexcept
    : _ptr{std::move(sp._ptr)},
      _control_block{std::move(sp._control_block)}
    {
        sp._ptr = nullptr;
        sp._control_block = nullptr;
    }

    /// Move constructor: Move-constructs a shared_ptr from sp
    /// Postconditions: *this shall contain the old value of sp.
    ///     sp shall be empty. sp.get() == 0.
    template<typename U>
    shared_ptr(shared_ptr<U>&& sp) noexcept
    : _ptr{sp._ptr},
      _control_block{sp._control_block}
    {
        sp._ptr = nullptr;
        sp._control_block = nullptr;
    }

    /// Constructs a shared_ptr object that shares ownership with wp
    /// Postconditions: use_count() == wp.use_count().
    template<typename U>
    explicit shared_ptr(const weak_ptr<U>& wp)
    : _ptr{wp._ptr},
      _control_block{wp._control_block}
    {
        if (wp.expired()) {
            throw bad_weak_ptr{};
        } else {
            _control_block->inc_ref();
        }
    }

    /// Constructs a shared_ptr object that obtains ownership from up
    /// Postconditions: use_count() == 1. up shall be empty. up.get() = 0.
    template<typename U, typename D>
    shared_ptr(unique_ptr<U, D>&& up)
    : shared_ptr{up.release(), up.get_deleter()}
    { }

    // 20.7.2.2.2, destructor

    ~shared_ptr()
    { if (_control_block) _control_block->dec_ref(); }

    // 20.7.2.2.3, assignment

    /// Copy assignment
    shared_ptr&
    operator=(const shared_ptr& sp) noexcept
    {
        shared_ptr{sp}.swap(*this);
        return *this;
    }

    /// Copy assignment
    template<typename U>
    shared_ptr&
    operator=(const shared_ptr<U>& sp) noexcept
    {
        shared_ptr{sp}.swap(*this);
        return *this;
    }

    /// Move assignment
    shared_ptr&
    operator=(shared_ptr&& sp) noexcept
    {
        shared_ptr{std::move(sp)}.swap(*this);
        return *this;
    }

    /// Move assignment
    template<typename U>
    shared_ptr&
    operator=(shared_ptr<U>&& sp) noexcept
    {
        shared_ptr{std::move(sp)}.swap(*this);
        return *this;
    }

    /// Move assignment from a unique_ptr
    template<typename U, typename D>
    shared_ptr&
    operator=(unique_ptr<U, D>&& up) noexcept
    {
        shared_ptr{std::move(up)}.swap(*this);
        return *this;
    }

    // 20.7.2.2.4, modifiers

    /// Exchanges the contents of *this and sp
    void
    swap(shared_ptr& sp) noexcept
    {
        using std::swap;
        swap(_ptr, sp._ptr);
        swap(_control_block, sp._control_block);
    }

    /// Resets *this to empty
    void
    reset() noexcept
    { shared_ptr{}.swap(*this); }

    /// Resets *this with p as the pointer to the managed object
    template<typename U>
    void
    reset(U* p)
    { shared_ptr{p}.swap(*this); }

    /// Resets *this with p as the pointer to the managed object,
    ///     supplied with custom deleter
    template<typename U, typename D>
    void
    reset(U* p, D d)
    { shared_ptr{p, d}.swap(*this); }

    /// Resets *this with p as the pointer to the managed object,
    ///     supplied with custom deleter and allocator
    template<typename U, typename D, typename A>
    void
    reset(U* p, D d, A a) = delete;

    // 20.7.2.2.5, observers

    /// Gets the stored pointer
    element_type*
    get() const noexcept
    { return _ptr; }

    /// Dereferences pointer to the managed object
    element_type&
    operator*() const noexcept
    { return *_ptr; }

    /// Dereferences pointer to the managed object
    element_type* 
    operator->() const noexcept
    { return _ptr; }

    /// Gets use_count
    long
    use_count() const noexcept
    { return (_control_block) ? _control_block->use_count() : 0; }

    /* deprecated in C++17, removed in C++20 */
    /// Checks if use_count == 1
    bool
    unique() const noexcept
    { return (_control_block) ? _control_block->unique() : false; }

    /// Checks if there is a managed object
    explicit operator bool() const noexcept
    { return (_ptr) ? true : false; }

    /// Checks whether this shared_ptr precedes other in owner-based order
    /// Implemented by comparing the address of control_block
    template<typename U>
    bool owner_before(shared_ptr<U> const& sp) const
    { return std::less<>(_control_block, sp._control_block); }

    /// Checks whether this shared_ptr precedes other in owner-based order
    /// Implemented by comparing the address of control_block
    template<class U>
    bool owner_before(weak_ptr<U> const& wp) const
    { return std::less<>(_control_block, wp._control_block); }

private:
    element_type* _ptr;
    detail::control_block_base* _control_block;
};

// 20.7.2.2.6, shared_ptr creation

/// Creates a shared_ptr that manages a new object
template<typename T, typename... Args>
    inline shared_ptr<T>
    make_shared(Args&&... args)
    { return shared_ptr<T>{new T{std::forward<Args>(args)...}}; }

template<typename T, typename A, typename... Args>
    inline shared_ptr<T>
    allocate_shared(const A& a, Args&&... args) = delete;

// 20.7.2.2.7, shared_ptr comparisons

/// Operator == overloading
template<typename T, typename U>
    inline bool
    operator==(const shared_ptr<T>& sp1,
               const shared_ptr<U>& sp2)
    { return sp1.get() == sp2.get(); }

template<typename T>
    inline bool
    operator==(const shared_ptr<T>& sp, std::nullptr_t) noexcept
    { return !sp; }

template<typename T>
    inline bool
    operator==(std::nullptr_t, const shared_ptr<T>& sp) noexcept
    { return !sp; }

/// Operator != overloading
template<typename T, typename U>
    inline bool
    operator!=(const shared_ptr<T>& sp1,
               const shared_ptr<U>& sp2)
    { return sp1.get() != sp2.get(); }

template<typename T>
    inline bool
    operator!=(const shared_ptr<T>& sp, std::nullptr_t) noexcept
    { return bool{sp}; }

template<typename T>
    inline bool
    operator!=(std::nullptr_t, const shared_ptr<T>& sp) noexcept
    { return bool{sp}; }

/// Operator < overloading
template<typename T, typename U>
    inline bool
    operator<(const shared_ptr<T>& sp1,
               const shared_ptr<U>& sp2)
    { return std::less<>(sp1.get(), sp2.get()); }

template<typename T>
    inline bool
    operator<(const shared_ptr<T>& sp, std::nullptr_t)
    { return std::less<>(sp.get(), nullptr); }

template<typename T>
    inline bool
    operator<(std::nullptr_t, const shared_ptr<T>& sp)
    { return std::less<>(nullptr, sp.get()); }

/// Operator <= overloading
template<typename T, typename U>
    inline bool
    operator<=(const shared_ptr<T>& sp1,
               const shared_ptr<U>& sp2)
    { return !(sp2.get() < sp1.get()); }

template<typename T>
    inline bool
    operator<=(const shared_ptr<T>& sp, std::nullptr_t)
    { return !(nullptr < sp.get()); }

template<typename T>
    inline bool
    operator<=(std::nullptr_t, const shared_ptr<T>& sp)
    { return !(sp.get() < nullptr); }

/// Operator > overloading
template<typename T, typename U>
    inline bool
    operator>(const shared_ptr<T>& sp1,
               const shared_ptr<U>& sp2)
    { return sp2.get() < sp1.get(); }

template<typename T>
    inline bool
    operator>(const shared_ptr<T>& sp, std::nullptr_t)
    { return nullptr < sp.get(); }

template<typename T>
    inline bool
    operator>(std::nullptr_t, const shared_ptr<T>& sp)
    { return sp.get() < nullptr; }

/// Operator >= overloading
template<typename T, typename U>
    inline bool
    operator>=(const shared_ptr<T>& sp1,
               const shared_ptr<U>& sp2)
    { return !(sp1.get() < sp2.get()); }

template<typename T>
    inline bool
    operator>=(const shared_ptr<T>& sp, std::nullptr_t)
    { return !(sp.get() < nullptr); }

template<typename T>
    inline bool
    operator>=(std::nullptr_t, const shared_ptr<T>& sp)
    { return !(nullptr < sp.get()); }

// 20.7.2.2.8, shared_ptr specialized algorithms

/// Swaps with another shared_ptr
template<typename T>
    inline void
    swap(shared_ptr<T>& sp1, shared_ptr<T>& sp2)
    { sp1.swap(sp2); }

// 20.7.2.2.9, shared_ptr casts

template<typename T, typename U>
    inline shared_ptr<T>
    static_pointer_cast(const shared_ptr<U>& sp) noexcept
    {
        using _Sp = shared_ptr<T>;
        return _Sp(sp, static_cast<typename _Sp::element_type*>(sp.get()));
    }

template<typename T, typename U>
    inline shared_ptr<T>
    const_pointer_cast(const shared_ptr<U>& sp) noexcept
    {
        using _Sp = shared_ptr<T>;
        return _Sp(sp, const_cast<typename _Sp::element_type*>(sp.get()));
    }

template<typename T, typename U>
    inline shared_ptr<T>
    dynamic_pointer_cast(const shared_ptr<U>& sp) noexcept
    {
        using _Sp = shared_ptr<T>;
        if (auto* _p = dynamic_cast<typename _Sp::element_type*>(sp.get()))
            return _Sp(sp, _p);
        return _Sp();
    }

/* added in C++17 */
template<typename T, typename U>
    inline shared_ptr<T>
    reinterpret_pointer_cast(const shared_ptr<U>& sp) noexcept
    {
        using _Sp = shared_ptr<T>;
        return _Sp(sp, reinterpret_cast<typename _Sp::element_type*>(sp.get()));
    }

// 20.7.2.2.10, shared_ptr get_deleter

template<typename D, typename T>
    inline D*
    get_deleter(const shared_ptr<T>& sp) noexcept
    { return reinterpret_cast<D*>(sp._control_block->get_deleter()); }

// 20.7.2.2.11, shared_ptr I/O

template<class E, class T, class Y>
    inline std::basic_ostream<E, T>&
    operator<<(std::basic_ostream<E, T>& os, const shared_ptr<Y>& sp)
    {
        os << sp.get();
        return os;
    }

} // namespace smart_ptr

namespace std {

// 20.7.2.6 Smart pointer hash support

// Template specialization of std::hash for smart_ptr::shared_ptr<T>

/**
 * Allows users to obtain hashes of objects of type smart_ptr::shared_ptr<T>,
 *  which can be used to store those objects in an unordered container.
 * 
 * This specialization ensures that std::hash<smart_ptr::shared_ptr<T>>()(sp) ==
 *  hash<typename smart_ptr::shared_ptr<T>::element_type*>()(sp.get()).
 */

template<typename T>
struct hash<smart_ptr::shared_ptr<T>> {
    using result_type = size_t;
    using argument_type = smart_ptr::shared_ptr<T>;

    size_t
    operator()(const smart_ptr::shared_ptr<T>& sp) const {
        return hash<typename smart_ptr::shared_ptr<T>::element_type*>()(sp.get());
    }
};

} // namespace std

#endif
