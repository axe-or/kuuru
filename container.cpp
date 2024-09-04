#pragma once

#include "assert.cpp"
#include "allocators.cpp"

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

	T at_or(isize idx, T const& val) const {
		if(idx < 0 && idx >= length){
			return val;
		}
		return data[idx];
	}

	Slice<T> sub(isize start, isize end){
		assert(start <= end && start >= 0 && end >= 0, "Invalid range for slice");
		return Slice<T>::from(&data[start], end - start);
	}
	
	Slice<T> sub(){
		return *this;
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
        assert(n >= 0, "Invalid size");
		Slice<T> s;
		s.data = ptr;
		s.length = n;
		return s;
	}
};

template<typename T> [[nodiscard]]
static Slice<T> make_slice(isize n, Allocator allocator){
	T* elems = (T*)(allocator.alloc(n * sizeof(T), alignof(T)));
	if(elems != nullptr){
		mem_zero(elems, n * sizeof(T));
	}
	return Slice<T>::from(elems, n);
}

template<typename T>
void destroy(Slice<T> s, Allocator allocator){
	for(isize i = 0; i < s.size(); i += 1){
		s[i].~T();
	}
	allocator.free(s.raw_data(), alignof(T));
}

template<typename T>
void reverse(Slice<T> s){
    T* buf = s.raw_data();
    isize n = s.size();

    for(isize i = 0; i < (n / 2); i += 1){
        swap(&buf[i], &buf[n - (i + 1)]);
    }
}

template<typename T>
struct Option {
	union {
		T data;
	};
	bool has_value = false;

	T get(){
		if(!has_value){
			panic("Cannot get() from empty option");
		}
		return data;
	}

	T get_or(T const& v){
		if(!has_value){
			return v;
		}
		return data;
	}

	bool ok() const {
		return has_value;
	}

	void operator=(T const& e){
		reset(e);
	}

	void operator=(Option<T> const& opt){
		if(opt.has_value){
			reset(opt.data);
		}
	}

	static Option from(T const& e){
		Option<T> opt;
		opt.reset(e);
		return opt;
	}

	static Option none(){
		return Option<T>{};
	}

	void reset(T const& e){
		if(has_value){
			data = e;
		} else {
			new (&data) T(e);
		}
		has_value = true;
	}

	void reset(){
		if(has_value){
			data.~T();
		}
		has_value = false;
	}

	Option(){}

	Option(Option<T> const& opt) : data(opt.data), has_value(opt.has_value) {}

	~Option(){
		reset();
	}
};

template<typename T, isize N>
struct Array {
    T data[N];

    T& operator[](isize idx){
   		assert(idx >= 0 && idx < N, "Out of bounds access to array");
        return data[idx];
    }

    T const& operator[](isize idx) const {
   		assert(idx >= 0 && idx < N, "Out of bounds access to array");
        return data[idx];
    }

    isize size() const {
        return N;
    }

	Slice<T> sub(){
		return Slice<T>::from(data, N);
	}

    Slice<T> sub(isize start, isize end){
        return Slice<T>::from(&data[start], end - start);
    }
};

/* Auto generated array operations */ namespace {
	template<typename T, isize N> constexpr auto operator+(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] + b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator-(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] - b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator*(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] * b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator/(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] / b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator%(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] % b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator&(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] & b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator|(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] | b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator^(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] ^ b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator<<(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] << b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator>>(Array<T, N> const& a, Array<T, N> const& b){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] >> b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator&&(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] && b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator||(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] || b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator==(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] == b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator!=(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] != b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator>(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] > b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator>=(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] >= b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator<(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] < b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator<=(Array<T, N> const& a, Array<T, N> const& b){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = a.data[i] <= b.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator+(Array<T, N> const& a){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = + a.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator-(Array<T, N> const& a){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = - a.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator~(Array<T, N> const& a){ Array<T, N> c; for(isize i = 0; i < N; i++){ c[i] = ~ a.data[i]; } return c; }
	template<typename T, isize N> constexpr auto operator!(Array<T, N> const& a){ Array<bool, N> c; for(isize i = 0; i < N; i++){ c[i] = ! a.data[i]; } return c; }
}

template<typename T>
struct DynamicArray {
	T* data = nullptr;
	isize capacity = 0;
	isize length = 0;
	Allocator allocator = {0};

	void resize(isize new_cap){
		T* new_data = (T*)(allocator.alloc(sizeof(T) * new_cap, alignof(T)));
		assert(new_data != nullptr, "Failed allocation");

		if(data != nullptr){
			mem_copy(new_data, data, sizeof(T) * length);
		}

		if(new_cap > length){
			mem_zero(&new_data[length], (new_cap - length) * sizeof(T));
		}
		allocator.free(data, alignof(T));

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

	void append_slice(Slice<T> elems){
		if(length >= capacity){
			resize(align_forward((length * 2) + elems.size(), alignof(T)));
		}

		mem_copy(&data[length], elems.raw_data(), elems.size() * sizeof(T));
		length += elems.size();
	}

	void pop(){
		if(length == 0){ return; }
		length -= 1;
		data[length].~T();
	}


	void insert(isize idx, T const& e){
		assert(idx >= 0 && idx <= length, "Out of bounds insertion to dynamic array");
		if(idx == length){
			return append(e);
		}

		if(length >= capacity){
			resize(align_forward((length * 2) + 1, alignof(T)));
		}

		mem_copy(&data[idx + 1], &data[idx], sizeof(T) * (length - idx));
		length += 1;
		data[idx] = e;
	}


	void remove(isize idx){
		assert(idx >= 0 && idx < length, "Out of bounds insertion to dynamic array");
		data[idx].~T();

		mem_copy(&data[idx], &data[idx + 1], sizeof(T) * (length - idx));
		length -= 1;
	}

	void clear(){
		for(isize i = 0; i < length; i += 1){
			isize pos = length - (i + 1);
			data[pos].~T();
		}
		length = 0;
	}

	T& operator[](isize idx){
		assert(idx >= 0 && idx < length, "Out of bounds access to dynamic array");
		return data[idx];
	}

	T const& operator[](isize idx) const {
		assert(idx >= 0 && idx < length, "Out of bounds access to dynamic array");
		return data[idx];
	}

	isize size() const {
		return length;
	}

	isize cap() const {
		return capacity;
	}

	// Create a clone of the array's current items using allocator.
	Slice<T> build_slice(Allocator allocator){
		auto s = make_slice<T>(allocator);
		mem_copy_no_overlap(s.raw_data(), data, length * sizeof(T));
		return s;
	}

	// Returns elements at moment of extraction, this clears the array.
	Slice<T> extract_slice(){
		auto s = Slice<T>::from(data, length);
		data = nullptr;
		length = 0;
		capacity = 0;
		return s;
	}

	Slice<T> sub(isize start, isize end){
		auto s = Slice<T>::from(data, length).sub(start, end);
		return s;
	}

	static DynamicArray create(Allocator allocator){
		DynamicArray arr;
		arr.allocator = allocator;
		return arr;
	}

	void dealloc(){
		clear();
		allocator.free(data, alignof(T));
		data = nullptr;
		capacity = 0;
	}
};

template<typename T>
void destroy(DynamicArray<T> arr){
	arr.dealloc();
}
