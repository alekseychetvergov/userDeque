#include <iostream>

#include <myDeque/my_deque.hpp>

int main() {
	Deque::MyDeque<int> values;
	values.push_back(20);
	values.push_front(10);
	values.push_back(30);

	for (const int value : values) {
		std::cout << value << ' ';
	}
	std::cout << '\n';
}
