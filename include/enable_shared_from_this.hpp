// enable_shared_from_this implementation

/**
 * enable_shared_from_this allows an object t that is currently managed by
 *  a shared_ptr named sp to safely generate additional shared_ptr instances
 *  sp1, sp2, ... that all share ownership of t with sp.
 * 
 * Publicly inheriting from enable_shared_from_this<T> provides the type T
 *  with a member function shared_from_this. If an object t of type T is
 *  managed by a shared_ptr<T> named sp, then calling T::shared_from_this
 *  will return a new shared_ptr<T> that shares ownership of t with sp.
 */

#ifndef ENABLE_SHARED_FROM_THIS_HPP
#define ENABLE_SHARED_FROM_THIS_HPP 1

#include "shared_ptr.hpp"
#include "weak_ptr.hpp"

namespace smart_ptr {

// Forward declarations

template<typename T> class shared_ptr;
template<typename T> class weak_ptr;

// 20.7.2.4 Class template enable_shared_from_this

template<typename T>
class enable_shared_from_this {
private:
    weak_ptr<T> weak_this;
protected:
    constexpr enable_shared_from_this() noexcept
    : weak_this{}
    { }
    
    enable_shared_from_this(const enable_shared_from_this& r) noexcept
    { }
    
    enable_shared_from_this&
    operator=(const enable_shared_from_this&)
    { return *this; }
    
    ~enable_shared_from_this()
    { }
public:
    shared_ptr<T>
    shared_from_this()
    { return shared_ptr<T>(weak_this); }

    shared_ptr<const T>
    shared_from_this() const
    { return shared_ptr<const T>(weak_this); }
};

} // namespace smart_ptr

#endif
