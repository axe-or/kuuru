#include <cstdio>
#include <cstddef>
#include <cstdint>

#pragma region "Prelude"
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using isize = ptrdiff_t;
using usize = size_t;

using byte = uint8_t;
using rune = int32_t;

using uintptr = uintptr_t;

using cstring = char const *;

static_assert(sizeof(isize) == sizeof(usize), "Mismatched size types");

template<typename T>
T min(T a, T b){
    return (a < b) ? a : b;
}

template<typename T>
T max(T a, T b){
    return (a > b) ? a : b;
}

template<typename T, typename... Args>
T min(T a, T b, Args&& ...rest){
    if(a < b){
        return min(a, rest...);
    }
    else {
        return min(b, rest...);
    }
}

template<typename T, typename... Args>
T max(T a, T b, Args&& ...rest){
    if(a > b){
        return max(a, rest...);
    }
    else {
        return max(b, rest...);
    }
}

template<typename T>
T clamp(T lo, T x, T hi){
    return min(max(lo, x), hi);
}

#pragma endregion

#pragma region "Debug"
#include <iostream> /* DEBUG ONLY. DON'T USE iostream, IT SUCKS. */

void assert(bool cond, cstring msg){
    if(!cond){
        std::fprintf(stderr, "Assert failed: %s\n", msg);
        throw "FAILED ASSERT";
    }
}

template<typename T>
void print(T x){
    std::cout << x << '\n';
}

template<typename T, typename ...Args>
void print(T x, Args&& ...args){
    std::cout << x << ' ';
    print(args...);
}
#pragma endregion

#pragma region "Memory"
void mem_copy(void* dest, void const* src, isize nbytes){
    __builtin_memmove(dest, src, nbytes);
}

void mem_copy_no_overlap(void* dest, void const* src, isize nbytes){
    __builtin_memcpy(dest, src, nbytes);
}

void mem_set(void* dest, byte val, isize nbytes){
    __builtin_memset(dest, val, nbytes);
}

bool valid_alignment(usize align){
    return (align != 0) && ((align & (align - 1)) == 0);
}

template<typename Integer>
Integer align_forward(Integer n, usize align){
    assert(valid_alignment(align), "Invalid memory alignment");
    Integer aligned = 0;
    Integer mod = n & (Integer(align) - 1);

    if(mod != 0){
        aligned = n + (align - mod);
    }

    return aligned;
}
#pragma endregion

#pragma region "Containers"
template<typename T>
struct Slice {
private:
    T* data = nullptr;
    isize length = 0;

public:
    T& operator[](isize idx){
        assert(idx >= 0 && idx < length, "Out of bounds access to slice");
        return data[idx];
    }

    T const& operator[](isize idx) const {
        assert(idx >= 0 && idx < length, "Out of bounds access to slice");
        return data[idx];
    }

    Slice<T> sub(isize start, isize end){
        assert(start <= end && start >= 0 && end >= 0, "Invalid range for slice");
        return Slice<T>::from(&data[start], end - start);
    }

    void reset() {
        data = nullptr;
        length = 0;
    }

    bool empty() const {
        return (length == 0) || (data == nullptr);
    }

    isize size() const { return length; }

    T* raw_data() const { return data; }

    static Slice<T> from(T* ptr, isize n){
        Slice<T> s;
        s.data = ptr;
        s.length = n;
        return s;
    }
};

template<typename T>
static Slice<T> make_slice(isize n){
    T* ptr = new T[n];
    return Slice<T>::from(ptr, n);
}

template<typename T>
void destroy(Slice<T> s){
    delete[] s.raw_data();
}

template<typename T>
struct DynamicArray {
private:
    T* data = nullptr;
    isize capacity = 0;
    isize length = 0;
public:
    void resize(isize new_cap){
        T* new_data = new T[new_cap];
        isize nbytes = sizeof(T) * new_cap;

        mem_copy(new_data, data, nbytes);
        delete [] data;

        length = min(new_cap, length);
        capacity = new_cap;
        data = new_data;
    }

    void append(T const& e){
        if(length >= capacity){
            resize(align_forward((length * 2) + 1, alignof(T)));
        }

        data[length] = e;
        length += 1;
    }

    void pop(){
        if(length == 0){ return; }
        length -= 1;
    }
};
#pragma endregion

struct Lexer {
};


int main(void) {
    auto values = make_slice<int>(100);
    for(isize i = 0; i < values.size(); i ++){
        print(values[i]);
    }

    destroy(values);
    return 0;
}
