// demo of unique_ptr

/**
 *  Modified from cppreference.com's demo code of std::unique_ptr:
 *  https://en.cppreference.com/w/cpp/memory/unique_ptr.
 */

#include <iostream>
#include <vector>
#include <cassert>

// #include <memory>
// using std::unique_ptr;

#include "unique_ptr.hpp"

struct D
{
    D() { std::cout << "D::D" << std::endl; }
    ~D() { std::cout << "D::~D" << std::endl; }
    void bar() { std::cout << "D::bar" << std::endl; }
};

int main()
{
    std::cout << "\nExclusive ownership semantics demo\n";
    {
        unique_ptr<D> up(new D()); // up is a unique_ptr that owns a D
        auto p = up.release(); // up releases ownership to p
        assert(!up); // now up owns nothing and holds a null pointer
        p->bar(); // and p owns the D object
        up.reset(p); // up regains ownership from p
    } // ~D called here
}
