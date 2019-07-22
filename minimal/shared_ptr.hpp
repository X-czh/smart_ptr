// minimal shared_ptr implementation

/**
 * Supports the core interface.
 * Reference counting is thread-safe.
 * 
 * No move semantics, custom deleter, or custom allocator.
 * Does not support array objects with a runtime length.
 * Partial support of the interface.
 * Weak type checking.
 */

#include <cstdlib>      /// nullptr_t
#include <atomic>       /// atomic
#include <utility>      /// swap

namespace detail {

// Type erasure for run-time polymorphic behavior of deleter 

class deleter_base {
public:
    virtual ~deleter_base() = default;
    virtual void operator()(void*) = 0;
};

template <typename T>
class deleter : public deleter_base {
public:
    void operator()(void* p) override {
        delete static_cast<T*>(p);
    }
};

} // namespace detail

template<typename T>
class shared_ptr {
public:
    template<typename U>
    friend class shared_ptr;

    // Constructors

    /// Default constructor, constructs an empty shared_ptr
    constexpr shared_ptr() noexcept = default;

    /// Constructs an empty shared_ptr
    constexpr shared_ptr(std::nullptr_t) noexcept
    { }

    /// Constructor to wrap raw pointer
    shared_ptr(T* p)
    : _ptr{p},
      _ref_count{new std::atomic<long>{1}},
      _deleter{new detail::deleter<T>()}
    { }

    /// Constructor to wrap raw pointer of convertible type
    template<typename U>
    shared_ptr(U* p)
    : _ptr{p},
      _ref_count{new std::atomic<long>{1}},
      _deleter{new detail::deleter<U>()}
    { }

    /// Copy constructor
    shared_ptr(const shared_ptr& sp) noexcept
    : _ptr{sp._ptr},
      _ref_count{sp._ref_count},
      _deleter{sp._deleter}
    { if (_ptr) ++(*_ref_count); }

    /// Conversion constructor
    template<typename U>
    shared_ptr(const shared_ptr<U>& sp) noexcept
    : _ptr{sp._ptr},
      _ref_count{sp._ref_count},
      _deleter{sp._deleter}
    { if (_ptr) ++(*_ref_count); }

    // Destructor

    /// No side-effect if shared_ptr is empty or use_count() > 1,
    /// otherwise release the resources
    ~shared_ptr()
    {
        if (_ptr) {
            if (--(*_ref_count) == 0) {
                delete _ref_count;
                (*_deleter)(_ptr);
                delete _deleter;
            }
        }
    }

    // Assignment

    shared_ptr& operator=(const shared_ptr& sp) noexcept
    {
        /// copy and swap idiom
        shared_ptr tmp{sp};
        tmp.swap(*this);
        return *this;
    }

    // Observers

    /// Dereferences pointer to the managed object
    T& operator*() const noexcept
    { return *_ptr; }

    /// Dereferences pointer to the managed object
    T* operator->() const noexcept
    { return _ptr; }

    /// Returns the contained pointer
    T* get() const noexcept
    { return _ptr; }

    /// Returns use_count (use_count == 0 if shared_ptr is empty)
    long use_count() const noexcept
    {
        if (_ptr) {
            return *_ref_count;
        } else {
            return 0;
        }
    }

    /// Checks if solely owns the managed object
    bool unique() const noexcept
    { return (use_count() == 1) ? true: false; }

    /// Checks if there is an associated managed object
    explicit operator bool() const noexcept
    { return (_ptr) ? true : false; }

    // Modifiers

    /// Resets shared_ptr to empty
    void reset() noexcept
    {
        shared_ptr tmp{};
        tmp.swap(*this);
    }

    /// Resets shared_ptr to wrap raw pointer p
    template<typename U>
    void reset(U* p)
    {
        shared_ptr tmp{p};
        tmp.swap(*this);
    }

    /// Swap with another shared_ptr
    void swap(shared_ptr& sp) noexcept {
        using std::swap;
        swap(_ptr, sp._ptr);
        swap(_ref_count, sp._ref_count);
        swap(_deleter, sp._deleter);
    }

private:
    T* _ptr;                        /// contained pointer
    std::atomic<long>* _ref_count;  /// reference counter
    detail::deleter_base* _deleter; /// deleter
};

// Operator == overloading

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

// Operator != overloading

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
