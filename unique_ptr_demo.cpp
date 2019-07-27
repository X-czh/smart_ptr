// demo of unique_ptr

/**
 *  Modified from cppreference.com's demo code of std::unique_ptr:
 *  https://en.cppreference.com/w/cpp/memory/unique_ptr.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <cstdio>
#include <cassert>

// #include <memory>
// using std::default_deleter;
// using std::unique_ptr;
// using std::make_unique; // requires C++14

#include "smart_ptr.hpp"
using smart_ptr::default_delete;
using smart_ptr::unique_ptr;
using smart_ptr::make_unique;

void close_file(std::FILE* fp) { std::fclose(fp); }

struct B {
    B() = default;
    virtual ~B() = default;
    virtual void bar() { std::cout << "B::bar" << std::endl; }
};

struct D : B
{
    D() { std::cout << "D::D" << std::endl; }
    ~D() { std::cout << "D::~D" << std::endl; }
    void bar() override { std::cout << "D::bar" << std::endl; }
};

int main()
{
    std::cout << "===============unique_ptr demo===============" << std::endl;
    std::cout << "\nExclusive ownership semantics demo\n";
    {
        auto up = make_unique<D>(); // up is a unique_ptr that owns a D
        auto p = up.release(); // up releases ownership to p
        assert(!up); // now up owns nothing and holds a null pointer
        p->bar(); // and p owns the D object
        up.reset(p); // up regains ownership from p
    } // ~D called here

    std::cout << "\nRuntime polymorphism demo\n";
    {
        unique_ptr<B> up = make_unique<D>(); // up is a unique_ptr that owns a D as a pointer to B
        up->bar(); // virtual dispatch, calls D::bar

        std::vector<unique_ptr<B>> v; // unique_ptr can be stored in a container
        v.push_back(make_unique<D>());
        v.push_back(std::move(up));
        v.emplace_back(new D{});
        for (auto& up: v) up->bar(); // virtual dispatch, calls D::bar
    } // ~D called 3 times
 
    std::cout << "\nCustom deleter demo\n";
    std::ofstream("demo.txt") << 'x'; // prepare the file to read
    {
        unique_ptr<std::FILE, decltype(&close_file)> fp(std::fopen("demo.txt", "r"),
                                                           &close_file);
        if(fp) { // fopen could have failed; in which case fp holds a null pointer
            std::cout << static_cast<char>(std::fgetc(fp.get())) << std::endl;
        }
    } // fclose() called here, but only if FILE* is not a null pointer, i.e., if fopen succeeded
 
    std::cout << "\nCustom lambda-expression deleter demo\n";
    {
        unique_ptr<D, std::function<void(D*)>> up(new D, [](D* ptr)
            {
                std::cout << "destroying from a custom deleter..." << std::endl;
                delete ptr;
            }); // up owns D
        up->bar();
    } // the lambda above is called and D is destroyed

    std::cout << "\nArray form of unique_ptr demo\n";
    {
        unique_ptr<D[]> up{new D[3]};
        up[2].bar();
    } // calls ~D 3 times

    std::cout << "\nEmpty Base Optimization (EBO) demo\n";
    {
        auto up1 = unique_ptr<int>{new int{3}};
        auto up2 = unique_ptr<int, default_delete<int>>{new int{3}};

        auto f = [](int* ptr){ delete ptr; };
        auto up3 = unique_ptr<int, decltype(f)>{new int{3}, f};
        auto up4 = unique_ptr<int, std::function<void(int*)>>{new int{3}, f};

        std::ofstream("demo.txt") << 'x'; // prepare the file to read
        auto up5 = unique_ptr<std::FILE, decltype(&close_file)>
            {std::fopen("demo.txt", "r"), &close_file};

        std::cout << sizeof(up1) << std::endl; // 8
        std::cout << sizeof(up2) << std::endl; // still 8 by EBO
        std::cout << sizeof(up3) << std::endl; // still 8 by EBO
        std::cout << sizeof(up4) << std::endl; // 40, std::function takes 32 bytes
        std::cout << sizeof(up5) << std::endl; // 16, additional function pointer
    }

    return 0;
}
