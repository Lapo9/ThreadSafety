#include <iostream>
#include <string>
#include <functional>
#include <sstream>
#include "ThreadSafe.h"
#include "Testt.h"


#define LESSON1 0
#define EXP1 1
#define SCRATCH 0
#define BASIC 0
#define AUTOCAST 0


#if LESSON1

class bar
{
public:
    bar(const bar &b)
    {
    }
};


int main() {
    Testt t1;
    
}

struct int_vector
{
    int *ptr_;
    int size_;
    int cap_;
public:
    int_vector(int_vector &v)
    {
        ptr_ = v.ptr_;
        size_ = v.size_;
        cap_ = v.cap_;

        v.ptr_=0;
        // ...
        // I need to allocate the same size, and copy all values over
    }
};


// Imagine we had a function add

big_number add(big_number a, big_number b)


// big_number add(big_number *a, big_number *b);
// big_number add(big_number &a, big_number &b);
// big_number add(big_number &&a, big_number &&b);

big_number three(3bn);
big_number  four(4bn);

add(three,four);

// this is our stack
//bottom [sizeof(void*), sizeof(void*)] top ?

// call add

// add pops 3 & 4, pushes 7


int foo(std::string s);
int foo_r(std::string&& s);



int main()
{
    std::string s = "foo";
    foo(s); // make a copy ctor const std::string &
    foo("bar"); // make a copy ctor const char*
    foo(std::move(s)); // makes a copy, ctor std::string &&
}




void foo(int v);
void foo(const int& v);
void foo(int& v);
void foo(int&& v);

#endif

#if SCRATCH
int appendAndLenght(std::string& app, float x) {
    app.append(" ");
    app.append(std::to_string(x));
    int y = app.size();
    return y;
}

void scratch() {
    std::string notSafe2{"ns2"};
    thread_safe::ThreadSafe<std::string> safe2{"safe2"};
    thread_safe::ThreadSafe<std::vector<int>> safe3{1,4};
    thread_safe::ThreadSafe<std::string> safe4{"safe4"};
    thread_safe::ThreadSafe<int> safe5{2};
    thread_safe::ThreadSafe<int> safe6{87};

    std::string test = "gg";
    std::string ns = test + std::string(*safe4); //why this works but if I omit std::string it doesn't? Implicit conversion should be performed

    int xyz;
    xyz << *safe5;

    //Left to right evaluation of ->* is forced (Rule 18 in https://en.cppreference.com/w/cpp/language/eval_order)
    *safe2 ->* appendAndLenght(notSafe2, 7126);
    int lenght2 = (safe2, safe3, safe4) ->* appendAndLenght(~safe2, 14.9); //ideal form, but how can I achieve this?
    int lenght4 = (safe2, safe3, safe4) ->* [&safe4]()->int{(~safe4).append(" lambda"); return (~safe4).size();}();
    //(safe2, safe3, safe4) ->* (int(*safe5) = appendAndLenght(~safe2, 3)); //NOTWORKING (but int(*safef) should be an lvalue since cast operator returns a ref)

    std::cout << std::string(*safe2) << "\t\x1B[32mlenght2: \033[0m" << lenght2 << "\n";
    std::cout << std::string(*safe4) << "\t\x1B[32mlenght4: \033[0m" << lenght4 << "\n";
}
#endif

#if BASIC
void basic() {
    thread_safe::ThreadSafe<std::string> safe1{"Ciao"};
    thread_safe::ThreadSafe<std::string>* pSafe1 = &thread_safe::ThreadSafe<std::string>("Pointer");

    safe1->append("oooooooooo"); //append something to the protected string
    (~safe1).append(" how are you?"); //append something to the string NOT safely

    (*pSafe1)->append(" /|/|/|/"); //append something to the pointer string safely
    (~*pSafe1).append(" -_-_-_-_-_-_"); //append something to the pointer string NOT safely

    std::string notSafe1 = *safe1;
    //std::cout << "\n\n" << (*safe1) << "\t" << notSafe1; //print the string safely
    std::cout << (*safe1); //print the string safely

    std::cout << "\n\n" << ~safe1; //print the string NOT safely

    std::cout << "\n\n" << std::string(**pSafe1); //print the string safely
    std::cout << "\n\n" << ~*pSafe1; //print the string NOT safely

    std::cout << "\n\n\n\n\n=========================================================================================================\n\n\n\n\n";
}
#endif


#if AUTOCAST

//void foo(const Testt& t) {
//    std::cout << "\nconst T&\n";
//}
//
//void foo(Testt&& t) {
//    std::cout << "\nT&&\n";
//}


void foo(Testt t) {
    std::cout << "\nfoo\n";
}


Testt bar(Testt& t) {
    std::cout << "\nbar\n";
    return t;
}


void autoCast() {

    Testt tt1;

    foo(bar(tt1));
    foo(std::move(bar(tt1)));


    //thread_safe::ThreadSafe<Testt> t1;

    //int xxx = 9;
    //xxx << *t1; //always rvalue, bad
    //xxx << std::move(*t1); //same as before, not good
}
#endif

#if EXP1

void exp1() {
    Testt t1;

    int a = 9;
    static_cast<float>(a);
    std::cout << std::is_same<int, decltype(a)>::value;

}


#endif


int main() {
    #if BASIC
        basic();
    #endif
    
    #if SCRATCH
        scratch();
    #endif

    #if AUTOCAST
        autoCast();
    #endif

    #if EXP1
        exp1();
    #endif

    int xwgt; std::cin >> xwgt;
}
