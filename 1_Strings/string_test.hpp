#pragma once
#include <string.hpp>
#include <cassert>
#include <vector>
#include <iostream>

void TestCreateStr(){
    const char* mess = "Message++";
    my::string str(mess);
}


void TestString(){
    TestCreateStr();
}
