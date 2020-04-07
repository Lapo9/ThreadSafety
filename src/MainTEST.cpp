#include <iostream>
#include <string>
#include "ThreadSafe.h"

int main() {
    thread_safe::ThreadSafe<std::string> safe1("\nciao");
    std::cout << safe1->data();
    safe1->append("oooooooooo");
    std::cout << safe1->data();

}
