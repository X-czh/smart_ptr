// owner_less implementation

/**
 * This function object provides owner-based (as opposed to value-based)
 *  mixed-type ordering of both std::weak_ptr and std::shared_ptr.
 *  The order is such that two smart pointers compare equivalent only if
 *  they are both empty or if they share ownership, even if the values of
 *  the raw pointers obtained by get() are different (e.g. because they
 *  point at different subobjects within the same object).
 * 
 * This class template is the preferred comparison predicate when building
 *  associative containers with std::shared_ptr or std::weak_ptr as keys, aka,
 *  std::map<std::shared_ptr<T>, U, std::owner_less<std::shared_ptr<T>>> or
 *  std::map<std::weak_ptr<T>, U, std::owner_less<std::weak_ptr<T>>>.
 */

#ifndef OWNER_LESS_HPP
#define OWNER_LESS_HPP 1

#include "shared_ptr.hpp"
#include "weak_ptr.hpp"

namespace smart_ptr {

// Forward declarations

template<typename T> class shared_ptr;
template<typename T> class weak_ptr;

// 20.7.2.3.7, Class template owner_less

template<typename T> struct owner_less;

template<typename T>
struct owner_less<shared_ptr<T>> {
    using result_type = bool;
    using first_argument_type = shared_ptr<T>;
    using second_argument_type = shared_ptr<T>;

    bool
    operator()(const shared_ptr<T>& lhs, const shared_ptr<T>& rhs) const
    { return lhs.owner_before(rhs); }

    bool
    operator()(const shared_ptr<T>& lhs, const weak_ptr<T>& rhs) const
    { return lhs.owner_before(rhs); }

    bool
    operator()(const weak_ptr<T>& lhs, const shared_ptr<T>& rhs) const
    { return lhs.owner_before(rhs); }
};

template<typename T>
struct owner_less<weak_ptr<T>> {
    using result_type = bool;
    using first_argument_type = weak_ptr<T>;
    using second_argument_type = weak_ptr<T>;

    bool
    operator()(const weak_ptr<T>& lhs, const weak_ptr<T>& rhs) const
    { return lhs.owner_before(rhs); }

    bool
    operator()(const shared_ptr<T>& lhs, const weak_ptr<T>& rhs) const
    { return lhs.owner_before(rhs); }

    bool
    operator()(const weak_ptr<T>& lhs, const shared_ptr<T>& rhs) const
    { return lhs.owner_before(rhs); }
};

} // namespace smart_ptr

#endif
