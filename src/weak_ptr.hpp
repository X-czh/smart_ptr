// weak_ptr implementation

#ifndef WEAK_PTR_HPP
#define WEAK_PTR_HPP 1

#include "detail/control_block.hpp"
#include "shared_ptr.hpp"

namespace smart_ptr {

// Forward declaration of shared_ptr

template<typename T> class shared_ptr;

// weak_ptr

template <typename T>
class weak_ptr {
public:
    template<typename U>
    friend class shared_ptr;

    template<typename U>
    friend class weak_ptr;

    using element_type = T;

    // Constructors

    /// Default constructor, creates an empty weak_ptr
    /// Postconditions: use_count() == 0.
    constexpr weak_ptr() noexcept
    : _ptr{},
      _control_block{}
    { }

    /// Conversion constructor: shares ownership with sp
    /// Postconditions: use_count() == sp.use_count().
    template<class U>
    weak_ptr(shared_ptr<U> const& sp) noexcept
    : _ptr{sp._ptr},
      _control_block{sp._control_block}
    { if (_control_block) _control_block->inc_wref(); } 

    /// Copy constructor: shares ownership with wp
    /// Postconditions: use_count() == wp.use_count().
    weak_ptr(weak_ptr const& wp) noexcept
    : _ptr{wp._ptr},
      _control_block{wp._control_block}
    { if (_control_block) _control_block->inc_wref(); }

    /// Copy constructor: shares ownership with wp
    /// Postconditions: use_count() == wp.use_count().
    template<class U>
    weak_ptr(weak_ptr<U> const& wp) noexcept
    : _ptr{wp._ptr},
      _control_block{wp._control_block}
    { if (_control_block) _control_block->inc_wref(); }

    // Destructor

    ~weak_ptr()
    { if (_control_block) _control_block->dec_wref(); }

    // Assignment

    weak_ptr&
    operator=(const weak_ptr& wp) noexcept
    {
        weak_ptr{wp}.swap(*this);
        return *this;
    }

    template<typename U>
    weak_ptr&
    operator=(const weak_ptr<U>& wp) noexcept
    {
        weak_ptr{wp}.swap(*this);
        return *this;
    }

    template<typename U>
    weak_ptr&
    operator=(const shared_ptr<U>& sp) noexcept
    {
        weak_ptr{sp}.swap(*this);
        return *this;
    }

    // Observers

    /// Gets use_count
    long
    use_count() const noexcept
    { return (_control_block) ? _control_block->use_count() : 0; }

    /// Checks if use_count == 0
    bool
    expired() const noexcept
    { return (_control_block) ? _control_block->expired() : false; }

    /// Checks if there is a managed object
    shared_ptr<T>
    lock() const noexcept
    { return (expired()) ? shared_ptr<T>{} : shared_ptr<T>{*this}; }

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

    // Modifiers

    /// Exchanges the contents of *this and sp
    void
    swap(weak_ptr& wp) noexcept
    {
        using std::swap;
        swap(_ptr, wp._ptr);
        swap(_control_block, wp._control_block);
    }

    /// Resets *this to empty
    void
    reset() noexcept
    { weak_ptr{}.swap(*this); }

private:
    element_type* _ptr;
    detail::control_block_base* _control_block;
};

// weak_ptr specialized algorithms

/// Swaps with another weak_ptr
template<typename T>
inline void
swap(weak_ptr<T>& wp1, weak_ptr<T>& wp2)
{ wp1.swap(wp2); }

} // namespace smart_ptr

#endif
