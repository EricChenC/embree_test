#include "EmbreeTest.h"

#include <iostream>

int main() {

    EmbreeTest test;

    test.Init();
    test.Render();


    std::cout << "embree test \n";

    return 0;
}