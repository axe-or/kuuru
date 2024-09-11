#pragma once

// Interface ///////////////////////////////////////////////////////////////////
#include "prelude.h"
#include "io.h"
#include "memory.h"

typedef struct {
	byte* data;
	isize cap;       // Total capacity
	isize last_read; // Offset of last read position
	isize len;       // Number of bytes after last_read
	Mem_Allocator allocator;
} Bytes_Buffer;

// Init a builder with a capacity, returns success status
bool buffer_init(Bytes_Buffer* bb, Mem_Allocator allocator, isize initial_cap);

// Get remaining free size given the current capacity
static inline
isize buffer_remaining(Bytes_Buffer* bb){
	return bb->cap - (bb->last_read + bb->len);
}

// Destroy a builder
void buffer_destroy(Bytes_Buffer* bb);

// Resize builder to have specified capacity, returns success status.
bool buffer_resize(Bytes_Buffer* bb, isize new_size);

// Resets builder's data, does not de-allocate
void buffer_reset(Bytes_Buffer* bb);

// Clear buffer's read bytes, this shifts the buffer's memory back to its base.
void buffer_clean_read_bytes(Bytes_Buffer* bb);

// Read bytes from the buffer, pushing its `read` pointer forward. Returns number of bytes read.
isize buffer_read(Bytes_Buffer* bb, byte* dest, isize size);

// Push bytes to the end of builder, returns success status
bool buffer_write(Bytes_Buffer* bb, byte const* b, isize len);

// Current unread bytes, this pointer becomes invalid as soon as the buffer is modified.
byte* buffer_bytes(Bytes_Buffer* bb);

IO_Stream buffer_stream(Bytes_Buffer* bb);

// Implementation //////////////////////////////////////////////////////////////
#ifdef BASE_C_IMPLEMENTATION

static inline
isize buffer_io_func(void* impl, IO_Operation op, byte* data, isize len){
	Bytes_Buffer* bb = (Bytes_Buffer*)(impl);
	switch(op){
	case IO_Op_Query: {
		return IO_Op_Read | IO_Op_Write;
	} break;

	case IO_Op_Read: {
		return buffer_read(bb, data, len);
	} break;

	case IO_Op_Write: {
		return buffer_write(bb, data, len);
	} break;
	}

	return 0;
}

bool buffer_init(Bytes_Buffer* bb, Mem_Allocator allocator, isize initial_cap){
	bb->allocator = allocator;
	bb->data = New(byte, initial_cap, allocator);

	if(bb->data != NULL){
		bb->cap = initial_cap;
		bb->len = 0;
		bb->last_read = 0;
		return true;
	} else {
		return false;
	}
}

void buffer_destroy(Bytes_Buffer* bb){
	mem_free(bb->allocator, bb->data);
	bb->data = 0;
}

// Clear buffer's read bytes, this shifts the buffer's memory back to its base.
void buffer_clean_read_bytes(Bytes_Buffer* bb){
	byte* data = buffer_bytes(bb);
	mem_copy(bb->data, data, bb->len);
	bb->last_read = 0;
}

byte* buffer_bytes(Bytes_Buffer* bb){
	return &bb->data[bb->last_read];
}

// Read bytes from the buffer, pushing its `read` pointer forward. Returns number of bytes read.
isize buffer_read(Bytes_Buffer* bb, byte* dest, isize size){
	if(bb->len == 0){ return 0; }
	isize n = Min(size, bb->len);
	mem_copy(dest, &bb->data[bb->last_read], n);
	bb->last_read += n;
	bb->len -= n;
	return n;
}

bool buffer_write(Bytes_Buffer* bb, byte const* bytes, isize len){
	if(bb->len + bb->last_read + len > bb->cap){
		bool status = buffer_resize(bb, bb->cap * 2);
		if(!status){ return false; }
	}
	mem_copy(&bb->data[bb->last_read + bb->len], bytes, len);
	bb->len += len;
	return true;
}

bool buffer_resize(Bytes_Buffer* bb, isize new_size){
	byte* new_data = New(byte, new_size, bb->allocator);
	if(new_data == NULL){ return false; }

	mem_copy(new_data, bb->data, Min(new_size, bb->len));
	mem_free(bb->allocator, bb->data);
	bb->data = new_data;
	bb->cap = new_size;

	return true;
}

void buffer_reset(Bytes_Buffer* bb){
	bb->len = 0;
	mem_set(bb->data, 0, bb->cap);
}

IO_Stream buffer_stream(Bytes_Buffer* bb){
	IO_Stream s = {
		.impl = bb,
		.func = buffer_io_func,
	};
	return s;
}

#endif
