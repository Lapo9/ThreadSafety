#include <iostream>
#include <string>
#include "ThreadSafe.h"

int main() {
    thread_safe::ThreadSafe<std::string> safe1("Ciao");
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

    int x; std::cin>>x;
}
