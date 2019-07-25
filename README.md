# smart_ptr

__*smart_ptr*__ is my own implementation of C++ smart pointers. It implements the smart pointers part (§20.7) of ISO C++ 2011 with some useful new features added (like make_unique) and the deprecated features removed (like auto_ptr).

## Motivation

Smart pointers are a distinctive feature of modern C++. They enable automatic and exception-safe object lifetime management. Implementing smart pointers is a great opportunity to deepen my understanding of smart pointers themselves and dynamic memory management in gerenal. The implementation also involves an understanding of templates, copy control and operator overloading, all important concepts of C++.

## Features

__*smart_ptr*__ fully implements the smart pointers part (§20.7) of ISO C++ 2011 with a few exceptions. Custom deleter/allocator, all kinds of non-member helper funcitons, as well as the std::hash class template specialization are supported. However, I do few checkings for template argument requirements as they are too tedious for educational purposes. For example, I do not explicitly check whether two pointer types are convertible, or whether a custom deleter type is copy-constructible. Conforming to these implicit requirements is left to the users.

It includes the following smart pointers and helper classes:

| Smart Pointer | Description |
| ------------- | ----------- |
| unique_ptr | smart pointer with exclusive object ownership semantics |
| shared_ptr | smart pointer with shared object ownership semantics |
| weak_ptr | weak reference to an object managed by shared_ptr |

| Helper class | Description |
| ------------ | ----------- |
| bad_weak_ptr | exception thrown when accessing an expired weak_ptr |
| default_delete | default deleter used by smart pointers |
| enable_shared_from_this | allows an object to create a shared_ptr referring to itself |
| owner_less | provides mixed-type owner-based ordering of shared and weak pointers |

A list of the extra features and the removed ones are given below. Notes regarding the status of those features in more recent C++ versions are given in brackets.

### Extra features

* make_unique (added in C++14)
* array type support for shared_ptr (added in C++17)
* reinterpret_pointer_cast for shared_ptr (added in C++17)
* operator<< for unique_ptr (added in C++20)

### Removed features

* auto_ptr (deprecated in C++11, removed in C++17)
* specialized atomic operations for shared_ptr (deprecated in C++20)

## Requirement

To include, simply include "smart_ptr.hpp", C++11 required. All names are defined in the smart_ptr namespace except for _control_block_base and _control_block, which are defined in the smart_ptr::detail namespace.

To run the demo, run Makefile, pthread support required.

## Implementation

![impl](https://github.com/X-czh/smart_ptr/blob/master/img/impl.jpg)

### Indirect management through control block

shared_ptr and weak_ptr do not directly manage the object. They do it indirectly through the control block. The control block will stay alive until no associated shared_ptr/weak_ptr lives. In this way, when we try to use a weak_ptr which points to an already destroyed object, we can learn from the control block that this weak_ptr has been expired and the object no longer exists.

### Type erasure of deleter/allocator in shared_ptr/weak_ptr

The template class of shared_ptr and weak_ptr has only one type parameter, which is the element type, and have no direct information about the type of the deleter and allocator. This is to ease the swap and assignment between different shared_ptrs/weak_ptrs (They may share the same element type, but have different deleter/allocator types. In that case, type conversion is difficult to handle.). However, this type information is required for deleter/allocator to function properly. To overcome this difficulty, I need to erase the type of the deleter/allocator (since they are stored in the control block, this is equivalent to erasing the type of the control block) in shared_ptr/weak_ptr. I accomplish it by relying on the runtime polymorphic behavior of a type with virtual functions. I first define a base class _control_block_base that defines the public interface. I then define a derived class control_block that contains all the neccessay type information and does the real stuff. The template class of shared_ptr/weak_ptr only stores a pointer to _control_block_base. When shared_ptr/weak_ptr is constructed, the constructors are supplied with the correct type information of the deleter/allocator, which can be used to initilize a control_block object. The pointer itself is of type _control_block_base\*, but it points to an object of type control_block. Relying on the runtime polymorphic behavior of C++, all control block operations are called through the _control_block_base interface, but actually done in the control_block object.

```c++
class _control_block_base {
    ... // defines the interface
};

template<typename T,
         typename D = default_delete<T>,
         typename A = allocator<T>>
class control_block : public _control_block_base {
    ... // actual implementation
};
```

## Note

* Since the access to ISO/IEC documents are not public, I refered to [N3337](https://github.com/cplusplus/draft/blob/master/papers/n3337.pdf), which is the same as the C++11 standard but with a few typographical corrections.
