// demo of shared_ptr

/**
 *  Modified from cppreference.com's demo code of std::weak_ptr:
 *      https://en.cppreference.com/w/cpp/memory/weak_ptr
 */

#include <iostream>

#include <memory>
// using std::shared_ptr;
// using std::weak_ptr;
// using std::make_shared;

#include "smart_ptr.hpp"
using smart_ptr::shared_ptr;
using smart_ptr::weak_ptr;
using smart_ptr::make_shared;

weak_ptr<int> gw;
 
void observe()
{
    std::cout << "use_count == " << gw.use_count() << ": ";
    if (auto spt = gw.lock()) { // Has to be copied into a shared_ptr before usage
	    std::cout << *spt << "\n";
    }
    else {
        std::cout << "gw is expired\n";
    }
}
 
int main()
{
    {
        auto sp = make_shared<int>(42);
	    gw = sp;
 
	    observe();
    }
 
    observe();
}
