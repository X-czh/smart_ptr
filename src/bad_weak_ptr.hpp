// bad_weak_ptr implementation

/**
 * bad_weak_ptr is the type of the object thrown as exceptions by
 *  the constructors of shared_ptr that take weak_ptr as the argument,
 *  when the weak_ptr refers to an already deleted object.
 */

#ifndef BAD_WEAK_PTR_HPP
#define BAD_WEAK_PTR_HPP 1

#include <exception>    // exception

namespace smart_ptr {

class bad_weak_ptr : public std::exception {
public:
    const char* what() noexcept
    { return "weak_ptr is expired!"; }
};

} // namespace smart_ptr

#endif
