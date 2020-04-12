#include <iostream>
#include <string>
#include <functional>
#include "ThreadSafe.h"


void scratch() {

    thread_safe::ThreadSafe<std::string> safe2{"safe2"};
    thread_safe::ThreadSafe<std::string> safe3{"safe3"};
    thread_safe::ThreadSafe<std::string> safe4{"safe4"};
    thread_safe::ThreadSafe<std::string> safe5{"safe5"};

    //safe2->* appendAndLenght(~safe2, 14.9); //ideal form, but how can I achieve this?
    //safe2->* appendAndLenght, ~safe2, 14.9; //ok, achievable form (the only problem is that the ->* can have only one argument)
    //safe2->*std::function([](int x, float y)->void {}); //" " "
    //safe2->*std::function(appendAndLenght); /" " " 

    int a = (safe2, safe3, safe4, safe5).printTEST();

    safe2->append(" free ");
    std::cout << std::string(*safe2) << a;

}

int appendAndLenght(std::string app, float x) {
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

    std::cout << "\n\n" << std::string(*safe1); //print the string safely
    std::cout << "\n\n" << ~safe1; //print the string NOT safely

    std::cout << "\n\n" << std::string(**pSafe1); //print the string safely
    std::cout << "\n\n" << ~*pSafe1; //print the string NOT safely

    std::cout << "\n\n\n\n\n=========================================================================================================\n\n\n\n\n";

    scratch();

    int x; std::cin >> x;
}
