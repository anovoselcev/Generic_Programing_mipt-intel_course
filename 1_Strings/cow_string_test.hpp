#pragma once

#include <cow_string.hpp>
#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

void TestCreate(){
    const char* msg1 = "Message 1";
    const char  msg2[] = "Message 2";
    my::cow_string str1(msg1);
    my::cow_string str2(msg1, strlen(msg1));
    my::cow_string str3(msg2);
    assert(str1 == "Message 1");
    assert(str2 == "Message 1");
    assert(str3 == "Message 2");
}

void Test_pb(){
    const char* msg1 = "Message 1";
    my::cow_string str1(msg1);
    str1.push_back('+');
    assert('+' == *str1.back());
    auto sub = str1.substr(0, str1.size() - 1);
    assert(sub == my::cow_string(msg1));
}

void Test_plus_new(){
    const char* msg1 = "Message 1";
    const char  msg2[] = "Message 2";
    my::cow_string str1(msg1);
    my::cow_string str2(msg2);
    auto str3 = str1 + str2;
    assert(str3 == "Message 1Message 2");

}

void Test_plus_mod(){
    const char* msg1 = "Message 1";
    const char  msg2[] = "Message 2";
    my::cow_string str1(msg1);
    my::cow_string str2(msg2);
    auto str3 = str1 + str2;
    assert(str3 == "Message 1Message 2");
    str1 += str2;
    assert(str3 == str1);
}

void Test_erase(){
    const char* msg1 = "Message 1";
    my::cow_string str1(msg1);
    str1.push_back('+');
    assert('+' == *str1.back());
    auto it = str1.cfind('+');
    str1.erase(it);
    assert(str1 == "Message 1");
}

void Test_erase_range(){
    const char* msg1 = "Message 1";
    my::cow_string str1(msg1);
    str1.push_back('+');
    assert('+' == *str1.back());
    auto it = str1.find('s');
    str1.erase(it, std::prev(str1.end(), 1));
    assert(str1 == "Me+");
}

void Test_idx(){
    const char* msg1 = "Message 1";
    my::cow_string str1(msg1);
    my::cow_string str2(str1);
    assert(str1.references() == str2.references());
    assert(str1.references() == 2);
    str1[0] = '}';
    assert(str1 == "}essage 1");
    assert(str2 == "Message 1");
    assert(str1.references() == 1);
    assert(str2.references() == 1);
}

void Test_concur(){
    my::cow_string str = "Concurency d";
    std::condition_variable cv;
    std::mutex m;
    bool f = false;
    auto th = [&str, &cv, &m, &f](){
        std::unique_lock<std::mutex> ul(m);
        my::cow_string str2(str);
        cv.wait(ul, [&f]{return f;});
    };
    std::vector<std::thread> threads(100);
    for(auto& thr : threads){
        thr = std::thread(th);
        thr.detach();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(str.references() == 101);
    f = true;
    cv.notify_all();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(str.references() == 1);

}


void Test_cow_string(){
    TestCreate();
    Test_pb();
    Test_plus_mod();
    Test_plus_new();
    Test_erase();
    Test_erase_range();
    Test_idx();
    Test_concur();
    std::cout << "COW string tests passed\n";
}
