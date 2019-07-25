smart_ptr:
	g++ -std=c++11 unique_ptr_demo.cpp -o unique_ptr_demo.out
	g++ -std=c++11 shared_ptr_demo.cpp -o shared_ptr_demo.out -lpthread
	g++ -std=c++11 weak_ptr_demo.cpp -o weak_ptr_demo.out
clean:
	rm -rf *.gch
	rm -rf *.out
