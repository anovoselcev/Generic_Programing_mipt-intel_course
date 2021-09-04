#pragma once
#include <vector>
#include <cow_string.hpp>
namespace my {


template<typename CharT,
         typename TraitsT,
         typename Allocator>
std::vector<my::cow_base_string<CharT, TraitsT, Allocator>> split(const my::cow_base_string<CharT, TraitsT, Allocator>& str,
                                                                  CharT sep){
    size_t size = str.count(sep) + 1;
    std::vector<my::cow_base_string<CharT, TraitsT, Allocator>> res(size);
    auto it = str.cbegin();
    size_t idx = 0;
    while(std::distance(it, str.cend()) > 0){
        const auto end = str.find(it, str.cend(), sep);
        my::cow_base_string<CharT, TraitsT, Allocator> sub;
        sub.assign(it, end);
        res[idx++] = std::move(sub);
        if(end == str.cend()) break;
        it = std::next(end);
    }
    return res;
}

}
