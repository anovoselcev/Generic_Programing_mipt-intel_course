#pragma once

#include <atomic>
#include <stdexcept>
#include <cstring>
#include <iterator.hpp>

namespace my {

template<typename CharT,
         typename TraitsT = std::char_traits<CharT>,
         typename Allocator = std::allocator<CharT>>

class cow_base_string{

    using it       = StringIterator<CharT>;
    using const_it = StringIterator<const CharT>;


    struct ControlBlock{
        CharT* data = nullptr;
        size_t size = 0;
        std::atomic<size_t> ref;
    };

    ControlBlock* info = nullptr;

    bool clean(){
        if(info && info->data){
            info->ref.fetch_sub(1);
            if(info->ref.load() == 0){
                Allocator alloc;
                alloc.deallocate(info->data, info->size);
                delete info;
            }
            info = nullptr;
            return true;
        }
        return false;
    }

    void restore(){
        if(info->ref.load() > 1){
            CharT* data = info->data;
            size_t size = info->size;
            info->ref.fetch_sub(1);
            info = new ControlBlock();
            Allocator alloc;
            info->data = alloc.allocate(size);
            std::memcpy(info->data, data, size);
            info->size = size;
            info->ref.fetch_add(1);
        }
    }

    void create(const CharT* data, size_t n){
        Allocator alloc;
        info = new ControlBlock();
        info->data = alloc.allocate(n);
        std::memcpy(info->data, data, n);
        info->ref.fetch_add(1);
        info->size = n;
        info->data[n] = '\0';
    }

public:

    ~cow_base_string(){
        if(info){
            if(info->ref.load() == 1){
                Allocator alloc;
                alloc.deallocate(info->data, info->size);
                delete info;
            }
            else
                info->ref.fetch_sub(1);
        }
    }

    cow_base_string() = default;

    cow_base_string(const CharT* data, size_t n){
        create(data, n);
    }

    cow_base_string(const CharT* data){
        size_t n = std::strlen(data) + 1;
        create(data, n);
    }

    template<size_t N>
    cow_base_string(const CharT (&data)[N]){
        create(&data[0], N);
    }

    cow_base_string(const cow_base_string& str){
        info = str.info;
        info->ref.fetch_add(1);
    }

    cow_base_string(cow_base_string&& str){
        info = str.info;
        str.info = nullptr;
    }

    cow_base_string& operator=(const cow_base_string& str){
        clean();
        info = str.info;
        info->ref.fetch_add(1);
        return *this;
    }

    cow_base_string& operator=(cow_base_string&& str){
        clean();
        info = str.info;
        str.info = nullptr;
        return *this;
    }

    template<typename CharU,
             typename TraitsU,
             typename AllocatorU,
             std::enable_if_t<std::is_convertible_v<CharU, CharT>>>
    bool operator==(const cow_base_string<CharU, TraitsU, AllocatorU>& str) const{
        if(str.info == info) return true;

        if(!str->info || !info || str.info->size != info->size) return false;

        for(size_t i = 0; i < info->size; ++i)
            if(!TraitsT::eq(info->data[i], str.info->data[i])) return false;

        return true;

    }

    template<typename CharU,
             typename TraitsU,
             typename AllocatorU,
             std::enable_if_t<std::is_constructible_v<CharU, CharT>>>
    bool operator<(const cow_base_string<CharU, TraitsU, AllocatorU>& str) const{
        if(str.info == nullptr && info == nullptr) return false;
        if(str.info == nullptr)                    return false;
        if(info == nullptr)                        return true;
        if(info == str.info)                       return false;

        if(info->size < str.info->size) return true;
        if(info->size > str.info->size) return false;


        for(size_t i = 0; i < info->size; ++i)
            if(!TraitsT::lt(info->data[i], str.info->data[i])) return false;

        return true;

    }

    CharT& operator[](size_t idx){

        if(!info) throw std::runtime_error("Nullptr exeption");

        restore();
        return info->data[idx];
    }

    CharT operator[](size_t idx) const{

        if(!info) throw std::runtime_error("Nullptr exeption");

        return info->data[idx];
    }

    CharT& at(size_t idx){
        if(!info) throw std::runtime_error("Nullptr exeption");

        if(idx > info->size - 2) throw std::out_of_range("");
        restore();
        return info->data[idx];
    }

    CharT at(size_t idx) const{
        if(!info) throw std::runtime_error("Nullptr exeption");

        if(idx > info->size - 2) throw std::out_of_range("");

        return info->data[idx];
    }

    const CharT* c_str() const{
        if(!info) return nullptr;
        return info->data;
    }

    it begin(){
        if(info && info->data){
            restore();
            return it(info->data);
        }
        return it();
    }

    it end(){
        if(info && info->data && info->size >= 1){
            restore();
            return it(&(info->data[info->size - 1]));
        }
        return it();
    }

    const_it cbegin() const{
        if(info) return const_it(info->data);
        return const_it();
    }

    const_it cend() const{
        if(info && info->data && info->size >= 1) return const_it(&(info->data[info->size]));
        return const_it();
    }

    size_t size() const{
        if(!info) return 0;
        if(!info->data) return 0;
        return info->size - 1;
    }

    size_t references() const{
        if(!info) return 0;
        return info->ref.load();
    }

    it front(){
        if(info && info->data){
            restore();
            return it(info->data);
        }
        return it();
    }

    const_it front() const{
        if(info && info->data) return const_it(info->data);
        return const_it();
    }

    it back(){
        if(info && info->data){
            restore();
            return it(&(info->data[info->size - 2]));
        }
        return it();
    }

    const_it back() const{
        if(info && info->data) return const_it(&(info->data[info->size - 2]));
        return const_it();
    }

    cow_base_string copy() const{
        if(!info || !info->data) return cow_base_string();
        return cow_base_string(info->data, info->size);
    }

    void assign(const_it beg, const_it end){
        Allocator alloc;
        size_t size = end - beg + 1;
        CharT* data = alloc.allocate(size);
        std::memcpy(data, &(*beg), size - 1);
        clean();
        if(!info) info = new ControlBlock();
        info->data = data;
        info->size = size;
        info->ref.fetch_add(1);
        data[size - 1] = '\0';
    }

    cow_base_string substr(size_t beg, size_t end) const{
        if(!info || !info->data) return cow_base_string();
        if(end - beg == info->size - 1) return copy();
        cow_base_string str;
        str.assign(beg, end);
        return str;
    }


    it find(CharT v){
        if(!info || !info->data) return it();
        if(info && info->data) restore();
        for(size_t idx = 0; idx < size(); ++idx)
            if(TraitsT::eq(v, info->data[idx])) return it(&(info->data[idx]));
        return end();
    }

    const_it find(CharT v) const{
        if(!info || !info->data) return const_it();
        for(size_t idx = 0; idx < size(); ++idx)
            if(TraitsT::eq(v, info->data[idx])) return const_it(&(info->data[idx]));
        return cend();
    }


    it find(it beg, it end, CharT v){
        if(!info || !info->data) return it();
        if(info && info->data) restore();
        for(auto it = beg; it != end; ++it)
            if(TraitsT::eq(v, *it)) return it;
        return end();
    }

    const_it find(const_it beg, const_it end, CharT v) const{
        if(!info || !info->data) return const_it();
        for(auto it = beg; it != end; it = std::next(it))
            if(TraitsT::eq(v, *it)) return it;
        return cend();
    }

    template<typename F>
    it find_if(F pred){
        if(!info || !info->data) return it();
        if(info && info->data) restore();
        for(size_t idx = 0; idx < size(); ++idx)
            if(pred(info->data[idx])) return it(&(info->data[idx]));
        return end();
    }

    template<typename F>
    const_it find_if(F pred) const{
        if(!info || !info->data) return const_it();
        for(size_t idx = 0; idx < size(); ++idx)
            if(pred(info->data[idx])) return const_it(&(info->data[idx]));
        return cend();
    }

    size_t count(CharT v) const{
        size_t res = 0;
        for(size_t idx = 0; idx < size(); ++idx) if(TraitsT::eq(v, info->data[idx])) res++;
        return res;
    }

};
}

template<typename CharU,
         typename TraitsU,
         typename AllocatorU>
std::basic_ostream<CharU, TraitsU>& operator<<(std::basic_ostream<CharU, TraitsU>& os, const my::cow_base_string<CharU, TraitsU, AllocatorU>& str){
    os << str.c_str();
    return os;
}

using cow_string = my::cow_base_string<char>;
