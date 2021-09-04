#pragma once
#include <iterator>

namespace my {

template  <typename T>
class StringIterator : public std::iterator<std::random_access_iterator_tag, T>{

    T* data = nullptr;

    StringIterator(T* p) : data(p){}

    template<typename CharT,
             typename TraitsT,
             typename Allocator>
    friend class cow_base_string;

public:

    StringIterator(const StringIterator& it) : data(it.data) {}

     StringIterator() {}

    bool operator!=(const StringIterator& it) const{
        return data != it.data;
    }

    bool operator==(const StringIterator& it) const{
        return data == it.data;
    }

    typename StringIterator::reference operator*() const{
        return *data;
    }

    StringIterator& operator++(){
        ++data;
        return *this;
    }

    StringIterator& operator+=(size_t d){
        data += d;
        return *this;
    }

    StringIterator operator+(size_t d) const{
        StringIterator it(*this);
        it.data += d;
        return it;
    }

    StringIterator& operator--(){
        --data;
        return *this;
    }

    StringIterator& operator-=(size_t d){
        data -= d;
        return *this;
    }

    StringIterator operator-(size_t d) const{
        StringIterator it(*this);
        it.data -= d;
        return it;
    }
};

}
