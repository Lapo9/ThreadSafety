#include <iostream>
#include <string>
#include <functional>
#include <sstream>
#include "ThreadSafe.h"
#include "Testt.h"

#define SCRATCH 0
#define BASIC 0
#define AUTOCAST 1



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

    int xwgt; std::cin >> xwgt;
}
