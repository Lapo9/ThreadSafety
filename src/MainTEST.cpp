#include <iostream>
#include <string>
#include <functional>
#include "ThreadSafe.h"


int appendAndLenght(std::string& app, float x);

void scratch() {
    std::string notSafe2{"ns2"};
    thread_safe::ThreadSafe<std::string> safe2{"safe2"};
    thread_safe::ThreadSafe<std::vector<int>> safe3{1,4};
    thread_safe::ThreadSafe<std::string> safe4{"safe4"};
    thread_safe::ThreadSafe<int> safe5{7};

    int out = 9;

    //is left to right evaluation of ->* forced? (standard says so: https://en.cppreference.com/w/cpp/language/eval_order)
    *safe2 ->* appendAndLenght(notSafe2, 7126);
    int lenght2 = (safe2, safe3, safe4) ->* appendAndLenght(~safe2, 14.9); //ideal form, but how can I achieve this?
    int lenght4 = (safe2, safe3, safe4) ->* [&safe4]()->int{safe4--.append(" lambda"); return (~safe4).size();}();
    (safe2, safe3, safe4) ->* (int(*safe5) = appendAndLenght(~safe2, 3)); //NOTWORKING (but int(*safef) should be an lvalue since cast operator returns a ref)

    //TEST int lenght4 = thread_safe::chainTEST( (safe2, safe3, safe4), [&safe4](){safe4--.append(" lambda"); return (~safe4).size();});     //it works, but it is verbose (moreover how can I avoid the cast to std::function)?

    std::cout << std::string(*safe2) << "\t\x1B[32mlenght2: \033[0m" << out << "\n";
    std::cout << std::string(*safe4) << "\t\x1B[32mlenght4: \033[0m" << lenght4 << "\n";
}

int appendAndLenght(std::string& app, float x) {
    app.append(" ");
    app.append(std::to_string(x));
    int y = app.size();
    return y;
}



int main() {
    thread_safe::ThreadSafe<std::string> safe1{"Ciao"};
    thread_safe::ThreadSafe<std::string>* pSafe1 = &thread_safe::ThreadSafe<std::string>("Pointer");

    safe1->append("oooooooooo"); //append something to the protected string
    (~safe1).append(" how are you"); //append something to the string NOT safely (method 1)
    safe1--.append("?????????"); //append something to the string NOT safely (method 2)

    (*pSafe1)->append(" /|/|/|/"); //append something to the pointer string safely
    (~*pSafe1).append(" -_-_-_-_-_-_"); //append something to the pointer string NOT safely (method 1)
    (*pSafe1)--.append(" mhhh pretty ugly!"); //append something to the pointer string NOT safely (method 2)

    std::string notSafe1 = *safe1;

    std::cout << "\n\n" << std::string(*safe1) << "\t" << notSafe1; //print the string safely
    operator<<(std::cout, *safe1);
    
    std::cout << "\n\n" << ~safe1; //print the string NOT safely

    std::cout << "\n\n" << std::string(**pSafe1); //print the string safely
    std::cout << "\n\n" << ~*pSafe1; //print the string NOT safely

    std::cout << "\n\n\n\n\n=========================================================================================================\n\n\n\n\n";

    scratch();

    int x; std::cin >> x;
}
