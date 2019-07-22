// demo of shared_ptr

/**
 *  Modified from cppreference.com's demo code of std::shared_ptr:
 *      https://en.cppreference.com/w/cpp/memory/shared_ptr,
 *  and cppreference.com's demo code of std::unique_ptr:
 *      https://en.cppreference.com/w/cpp/memory/unique_ptr,
 *  and cplusplus.com's demo code of std::shared_ptr constructors:
 *      http://www.cplusplus.com/reference/memory/shared_ptr/shared_ptr/
 */

#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>

// using std::unique_ptr;
// using std::shared_ptr;
// using std::weak_ptr;
// using std::make_shared;
// using std::default_delete;

#include "smart_ptr.hpp"
using smart_ptr::unique_ptr;
using smart_ptr::shared_ptr;
using smart_ptr::weak_ptr;
using smart_ptr::default_delete;
using smart_ptr::make_shared;
using smart_ptr::get_deleter;


void close_file(std::FILE* fp) { std::fclose(fp); }

struct B {
    B() = default;
    ~B() = default;
    virtual void bar() { std::cout << "B::bar" << std::endl; }
};

struct D : B
{
    D() { std::cout << "D::D" << std::endl; }
    ~D() { std::cout << "D::~D" << std::endl; }
    void bar() override { std::cout << "D::bar" << std::endl; }
};

struct C {
    int* data;
};

struct Base
{
    Base() { std::cout << "  Base::Base()\n"; }
    // Note: non-virtual destructor is OK here
    // See https://stackoverflow.com/questions/3899790/shared-ptr-magic
    ~Base() { std::cout << "  Base::~Base()\n"; }
};
 
struct Derived: public Base
{
    Derived() { std::cout << "  Derived::Derived()\n"; }
    ~Derived() { std::cout << "  Derived::~Derived()\n"; }
};

void thr(shared_ptr<Base> p)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    shared_ptr<Base> lp = p; // thread-safe, even though the
                             // shared use_count is incremented
    {
        static std::mutex io_mutex;
        std::lock_guard<std::mutex> lk(io_mutex);
        std::cout << "local pointer in a thread:\n"
                  << "  lp.get() = " << lp.get()
                  << ", lp.use_count() = " << lp.use_count() << '\n';
    }
}
void deletees(D*) {

}

int main()
{
    std::cout << "===============shared_ptr demo===============" << std::endl;

    std::cout << "\nShared ownership semantics demo\n";
    {
        auto sp = make_shared<D>(); // sp is a shared_ptr that manages a D
        sp->bar(); // and p manages the D object
        std::cout << sp.use_count() << std::endl;
        auto sp2(sp); // copy constructs a new shared_ptr sp2
        std::cout << sp.use_count() << std::endl;
    } // ~D called here

    std::cout << "\nRuntime polymorphism demo\n";
    {
        shared_ptr<B> sp(make_shared<D>()); // sp is a shared_ptr that manages a D as a pointer to B
        sp->bar(); // virtual dispatch, calls D::bar

        std::vector<shared_ptr<B>> v; // shared_ptr can be stored in a container
        v.push_back(make_shared<D>());
        v.push_back(std::move(sp));
        v.emplace_back(new D{});
        for (auto& sp: v) sp->bar(); // virtual dispatch, calls D::bar
    } // ~D called 3 times

    std::cout << "\nCustom deleter demo\n";
    std::ofstream("demo.txt") << 'x'; // prepare the file to read
    {
        shared_ptr<std::FILE> fp(std::fopen("demo.txt", "r"), &close_file);
        if(fp) { // fopen could have failed; in which case fp holds a null pointer
            std::cout << static_cast<char>(std::fgetc(fp.get())) << std::endl;
        }
    } // fclose() called here, but only if FILE* is not a null pointer, i.e., if fopen succeeded

    std::cout << "\nCustom lambda-expression deleter demo\n";
    {
        shared_ptr<D> sp(new D, [](D* ptr)
            {
                std::cout << "destroying from a custom deleter..." << std::endl;
                delete ptr;
            }); // up owns D
        sp->bar();
    } // the lambda above is called and D is destroyed

    std::cout << "\nConstructor demo\n";
    {
        shared_ptr<int> p0;
        shared_ptr<int> p1 (nullptr); // use_count = 0
        shared_ptr<int> p2 (nullptr, default_delete<int>()); // use_count = 1 with custom deleter
        shared_ptr<int> p3 (new int);
        shared_ptr<int> p4 (new int, default_delete<int>());
        shared_ptr<int> p5 (new int, [](int* p){delete p;}, std::allocator<int>());
        shared_ptr<int> p6 (p5);
        shared_ptr<int> p7 (std::move(p6));
        shared_ptr<int> p8 (unique_ptr<int>(new int));
        shared_ptr<C> obj (new C);
        shared_ptr<int> p9 (obj, obj->data); // aliasing constructor, different pointers, shared lifetime

        std::cout << "use_count:\n";
        std::cout << "p0: " << p1.use_count() << '\n';
        std::cout << "p1: " << p1.use_count() << '\n';
        std::cout << "p2: " << p2.use_count() << '\n';
        std::cout << "p3: " << p3.use_count() << '\n';
        std::cout << "p4: " << p4.use_count() << '\n';
        std::cout << "p5: " << p5.use_count() << '\n';
        std::cout << "p6: " << p6.use_count() << '\n';
        std::cout << "p7: " << p7.use_count() << '\n';
        std::cout << "p8: " << p8.use_count() << '\n';
        std::cout << "p9: " << p9.use_count() << '\n';
    }

    std::cout << "\nMulti-threading demo\n";
    { 
        shared_ptr<Base> p(new Derived());
    
        std::cout << "Created a shared Derived (as a pointer to Base)\n"
                << "  p.get() = " << p.get()
                << ", p.use_count() = " << p.use_count() << '\n';
        std::thread t1(thr, p), t2(thr, p), t3(thr, p);
        p.reset(); // release ownership from main
        std::cout << "Shared ownership between 3 threads and released\n"
                << "ownership from main:\n"
                << "  p.get() = " << p.get()
                << ", p.use_count() = " << p.use_count() << '\n';
        t1.join(); t2.join(); t3.join();
        std::cout << "All threads completed, the last one deleted Derived\n";
    }

    std::cout << "\nGet deleter demo\n";
    {
        auto sp = make_shared<D>();
        D* p = new D;
        auto del_p = get_deleter<default_delete<D>>(sp);
        (*del_p)(p);
    }

    return 0;
}
