// control block implementation

#ifndef CONTROL_BLOCK_HPP
#define CONTROL_BLOCK_HPP 1

#include <memory>       // addressof
#include <atomic>       // atomic

#include "default_delete.hpp"

namespace detail {

// control block interface
// Type erasure for storing deleter and allocators

class control_block_base {
public:
    virtual ~control_block_base() { };

    virtual void inc_ref() noexcept = 0;
    virtual void inc_wref() noexcept = 0;
    virtual void dec_ref() noexcept = 0;
    virtual void dec_wref() noexcept = 0;

    virtual long use_count() const noexcept = 0;
    virtual bool unique() const noexcept = 0;
    virtual long weak_use_count() const noexcept = 0;
    virtual bool expired() const noexcept = 0;

    virtual void* get_deleter() noexcept = 0;
};

// control block for reference counting of shared_ptr and weak_ptr

template<typename T,
         typename D = default_delete<T>, 
         typename A = std::allocator<T>>
class control_block : public control_block_base {
public:
    using element_type = T;
    using deleter_type = D;
    using allocator_type = A;

    // Constructors

    control_block(T* p)
    : _ptr{p}, _deleter{D{}}, _allocator{A{}}
    { }
    
    control_block(T* p, D d)
    : _ptr{p}, _deleter{d}, _allocator{A{}}
    { }

    control_block(T* p, D d, A a)
    : _ptr{p}, _deleter{d}, _allocator{a}
    { }

    // Destructor

    ~control_block()
    { }

    // Modifiers

    void
    inc_ref() noexcept override
    { ++_use_count; }

    void
    inc_wref() noexcept override
    { ++_weak_use_count; }

    void
    dec_ref() noexcept override
    {
        if (--_use_count == 0) {
            if (_ptr) _deleter(_ptr); // destroy the object _ptr points to
            dec_wref();
        }
    }

    void
    dec_wref() noexcept override
    {
        if (--_weak_use_count == 0) {
            delete this; // destroy control_block itself
        }
    }

    // Observers

    long
    use_count() const noexcept override // Returns #shared_ptr
    { return _use_count; }

    bool
    unique() const noexcept override
    { return _use_count == 1; }

    long
    weak_use_count() const noexcept override // Returns #weak_ptr + 1, see Note below
    { return _weak_use_count; }

    bool
    expired() const noexcept override
    { return _use_count == 0; }

    void*
    get_deleter() noexcept override // Type erasure for storing deleter
    { return reinterpret_cast<void*>(std::addressof(_deleter)); }

private:
    std::atomic<long> _use_count{1};
    std::atomic<long> _weak_use_count{1}; // Note: _weak_use_count = #weak_ptrs + (#shared_ptr > 0) ? 1 : 0
    element_type* _ptr;         // stored pointer
    deleter_type _deleter;      // stored deleter
    allocator_type _allocator;  // stored allocator
};

} // namespace detail

#endif
