#ifndef DEFAULT_DELETE_HPP
#define DEFAULT_DELETE_HPP 1

namespace smart_ptr {

// deafault_delete for single object

template <typename T>
class default_delete
{
public:
    /// Default constructor
    constexpr default_delete() noexcept = default;

    /// Converting constructor, convertibility is not checked
    template <typename U>
    default_delete(const default_delete<U>&) noexcept
    { }

    /// Call operator
    void operator()(T* p) const
    { delete p; }
};

// default_delete for array objects with a runtime length

template <typename T>
class default_delete<T[]>
{
public:
    /// Default constructor
    constexpr default_delete() noexcept = default;

    /// Converting constructor, convertibility is not checked
    template <typename U>
    default_delete(const default_delete<U[]>&) noexcept
    { }

    /// Call operator
    void operator()(T* p) const
    { delete[] p; }
};

} // namespace smart_ptr

#endif
