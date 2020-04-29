#ifndef TESTTTTTTTTTTTTTTTTT
#define TESTTTTTTTTTTTTTTTTT

#include <iostream>


class Testt {

	public:
	Testt() {
		std::cout << "\x1B[42mTestt ctor\033[0m\n";
	}

	Testt(Testt&)
	{
		std::cout << "\x1B[42mTestt copy ctor\033[0m\n";
	}

	Testt(Testt&&) {
		std::cout << "\x1B[42mTestt move ctor\033[0m\n";
	}

	Testt& operator=(Testt&)
	{
		std::cout << "\x1B[42mTestt copy =\033[0m\n";
	}

	Testt& operator=(Testt&&) {
		std::cout << "\x1B[42mTestt move =\033[0m\n";
	}

	~Testt() {
		std::cout << "\x1B[42mTestt dtor\033[0m\n";
	}


	int example(float x) {
		std::cout << "\x1B[32mTestt example\033[0m\n";
		return x;
	}


	friend int operator<<(Testt& t, int x) {
		std::cout << "\x1B[32mTestt <<lhs\033[0m\n";
		return ++x;
	}

	friend int operator<<(int x, Testt& t) {
		std::cout << "\x1B[32mTestt <<rhs\033[0m\n";
		t.a = 3;
		return x+=10;
	}


	int a=0;

};



#endif
