// demo of shared_ptr
/**
 *  Modified from cppreference.com's demo code of std::shared_ptr:
 *  https://en.cppreference.com/w/cpp/memory/shared_ptr.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
 
// #include <memory>
// using std::shared_ptr;

#include "shared_ptr.hpp"

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
 
int main()
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
