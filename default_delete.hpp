#ifndef DEFAULT_DELETE_HPP
#define DEFAULT_DELETE_HPP

namespace smart_ptr {

class default_delete
{
public:
    /// Default constructor
    constexpr default_delete() noexcept = default;

    /// Function call operator
    template <typename T>
    void operator()(T* p) const
    { delete p; }
};

} // namespace smart_ptr

#endif
