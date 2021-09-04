#pragma once

#include <atomic>
#include <stdexcept>
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
        if(info){
            info->ref.fetch_sub(1);
            if(info->ref.load() == 0){
                Allocator alloc;
                alloc.dealocate(info->data, info->size);
                delete info;
                info = nullptr;
                return true;
            }
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
        Allocator alloc;
        info = new ControlBlock();
        info->data = alloc.allocate(n);
        std::memcpy(info->data, data, n);
        info->ref.fetch_add(1);
        info->size = n;
        info->data[n] = '\0';
    }

    template<size_t N>
    cow_base_string(const CharT (&data)[N]){
        Allocator alloc;
        info = new ControlBlock();
        info->data = alloc.allocate(N);
        std::memcpy(info->data, data, N);
        info->ref.fetch_add(1);
        info->size = N ;
        info->data[N] = '\0';
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
    }

    cow_base_string& operator=(cow_base_string&& str){
        clean();
        info = str.info;
        str.info = nullptr;
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

    CharT& at(size_t idx) const{
        if(!info) throw std::runtime_error("Nullptr exeption");

        if(idx > info->size - 2) throw std::out_of_range("");

        return info->data[idx];
    }

    const CharT* c_str() const{
        if(!info) return nullptr;
        return info->data;
    }

    it begin(){
        if(info){
            restore();
            return it(info->data);
        }
        return it();
    }

    it end(){
        if(info && info->size >= 1){
            restore();
            return it(info->data + info->size - 1);
        }
        return it();
    }

    const_it cbegin() const{
        if(info) return const_it(info->data);
        return const_it();
    }

    const_it cend() const{
        if(info && info->size >= 1) return const_it(info->data + info->size - 1);
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
