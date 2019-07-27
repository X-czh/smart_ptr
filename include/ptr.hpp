// Ptr implementation

#ifndef PTR_HPP
#define PTR_HPP 1

#include <tuple> // tuple, get(tuple)

namespace smart_ptr {

namespace detail {

// Ptr class that wraps the deleter, use tuple for Empty Base Optimization

template<typename T, typename D>
class Ptr {
public:
    using pointer = T*;
    using deleter_type = D;

    constexpr Ptr() noexcept = default;

    Ptr(pointer p)
    : _impl_t{}
    { _impl_ptr() = p; }

    template<typename Del>
    Ptr(pointer p, Del&& d)
	: _impl_t{p, std::forward<Del>(d)}
    { }

    ~Ptr() noexcept = default;

    pointer& _impl_ptr() { return std::get<0>(_impl_t); }
    pointer _impl_ptr() const { return std::get<0>(_impl_t); }
    deleter_type& _impl_deleter() { return std::get<1>(_impl_t); }
    const deleter_type& _impl_deleter() const { return std::get<1>(_impl_t); }

private:
    std::tuple<pointer, deleter_type> _impl_t;
};

} // namespace detail

} // namespace smart_ptr

#endif
