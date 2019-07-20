// minimal unique_ptr implementation

/**
 * Supports the core interface.
 * 
 * No move semantics or custom deleter.
 * Does not support array objects with a runtime length.
 * Partial support of the interface.
 */

#include <cstdlib>      /// nullptr_t
#include <utility>      /// swap

template<typename T>
class unique_ptr {
public:

    // Constructors

    /// Default constructor, creates a unique_ptr that owns nothing
    constexpr unique_ptr() noexcept = default;

    /// Constructs with nullptr, creates a unique_ptr that owns nothing
    constexpr unique_ptr(std::nullptr_t) noexcept
    { }

    /// Takes ownership from a pointer
    explicit unique_ptr(T* p) noexcept
    : _ptr{p}
    { }

    // Destructor

    ~unique_ptr()
    { if (_ptr) delete(_ptr); }

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

    /// Checks if there is an associated managed object
    explicit operator bool() const noexcept
    { return (_ptr) ? true : false; }

    /// Releases ownership to the returned raw pointer
    T* release()
    {
        T* cp = _ptr;
        _ptr = nullptr;
        return cp;
    }

    /// Resets unique_ptr to empty
    void reset()
    {
        if (_ptr) delete(_ptr);
        _ptr = nullptr;
    }

    /// Resets unique_ptr to empty and takes ownership from a pointer
    void reset(T* p) noexcept
    {
        if (_ptr) delete(_ptr);
        _ptr = p;
    }

    /// Swaps with another unique_ptr
    void swap(unique_ptr& up) noexcept
    {
        using std::swap;
        swap(_ptr, up._ptr);
    }

    // Disables copy from lvalue

    /// Disables copy constructor
    unique_ptr(const unique_ptr&) = delete;

    /// Disables copy assignment
    unique_ptr& operator=(const unique_ptr&) = delete;

private:
    T* _ptr; // contained pointer
};

// Operator == overloading

template <typename T, typename U>
    inline bool
    operator==(const unique_ptr<T>& sp1,
               const unique_ptr<U>& sp2)
    { return sp1.get() == sp2.get(); }

template<typename T>
    inline bool
    operator==(const unique_ptr<T>& sp, std::nullptr_t) noexcept
    { return !sp; }

template<typename T>
    inline bool
    operator==(std::nullptr_t, const unique_ptr<T>& sp) noexcept
    { return !sp; }

// Operator != overloading

template <typename T, typename U>
    inline bool
    operator!=(const unique_ptr<T>& sp1,
               const unique_ptr<U>& sp2)
    { return sp1.get() != sp2.get(); }

template<typename T>
    inline bool
    operator!=(const unique_ptr<T>& sp, std::nullptr_t) noexcept
    { return bool{sp}; }

template<typename T>
    inline bool
    operator!=(std::nullptr_t, const unique_ptr<T>& sp) noexcept
    { return bool{sp}; }
