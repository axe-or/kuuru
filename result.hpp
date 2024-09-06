#include <stdio.h>
#include <stdint.h>

namespace result_impl {
    static inline void mem_set(void * ptr, uint8_t value, size_t nbytes){
        __builtin_memset(ptr, value, nbytes);
    }
}

template<typename T, typename E>
struct Result {
private:
    union {
        T value;
        E error;
    } data;
    bool is_value = false;

public:
    bool is_ok() const { return is_value; }
    
    bool is_error() const { return is_error; }

    T get(){
        if(!is_value){
            throw "Bad result";
        }
        return data.value;
    }

    T get_or(T const& alt){
        if(!is_value){
            return alt;
        } else {
            return data.value;
        }
    }

    void reset(T const& new_val){
        if(is_value){
            data.value = new_val;
        } else {
            new (&data.value) T(new_val);
        }
        is_value = true;
    }

    void reset(E const& new_err){
        if(!is_value){
            data.error = new_err;
        } else {
            data.value.~T();
            new (&data.error) T(new_err);
        }
        is_value = true;
    }

    Result(){
        mem_set(&this->data, 0, sizeof(this->data));
    }

    ~Result(){
        if(is_value){
            data.value.~T();
        } else {
            data.error.~T();
        }
    }
};

