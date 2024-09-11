#pragma once

// Interface ///////////////////////////////////////////////////////////////////
#include "prelude.h"

typedef enum {
	IO_Op_Query = 0,
	IO_Op_Read  = 1 << 0,
	IO_Op_Write = 1 << 1,
} IO_Operation;

typedef isize (*IO_Func)(void* impl, IO_Operation op, byte* data, isize len);

typedef struct {
	void* impl;
	IO_Func func;
} IO_Stream;

typedef struct { IO_Stream _stream; } IO_Writer;

typedef struct { IO_Stream _stream; } IO_Reader;

typedef enum {
	IO_Err_None = 0,
	IO_Err_End = -1,
	IO_Err_Closed = -2,
	IO_Err_Out_Of_Memory = -3,

	IO_Err_Unknown = -127,
} IO_Error;


// Read into buffer, returns number of bytes read into buffer. On error returns
// the negative valued IO_Error code.
isize io_read(IO_Reader r, byte* buf, isize buflen);

// Write into buffer, returns number of bytes written to buffer. On error returns
// the negative valued IO_Error code.
isize io_write(IO_Writer r, byte const* buf, isize buflen);

// Query the capabilites of the IO stream as per IO_Operation
i8 io_query_stream(IO_Stream s);

// Cast a stream to a IO reader, in debug mode this uses a query to assert that
// the stream supports reading
IO_Reader io_to_reader(IO_Stream s);

// Cast a stream to a IO writer, in debug mode this uses a query to assert that
// the stream supports writing
IO_Writer io_to_writer(IO_Stream s);

// Implementation //////////////////////////////////////////////////////////////
#ifdef BASE_C_IMPLEMENTATION
#include "assert.h"

i8 io_query_stream(IO_Stream s){
	return s.func(s.impl, IO_Op_Query, NULL, 0);
}

isize io_read(IO_Reader r, byte* buf, isize buflen){
	IO_Stream s = r._stream;
	return s.func(s.impl, IO_Op_Read, buf, buflen);
}

isize io_write(IO_Writer w, byte const* buf, isize buflen){
	IO_Stream s = w._stream;
	return s.func(s.impl, IO_Op_Write, (byte*)(buf), buflen);
}

IO_Reader io_to_reader(IO_Stream s){
	i8 cap = io_query_stream(s);
	debug_assert(cap & IO_Op_Read, "Stream does not support reading.");
	IO_Reader r = { ._stream = s };
	return r;
}

IO_Writer io_to_writer(IO_Stream s){
	i8 cap = io_query_stream(s);
	debug_assert(cap & IO_Op_Write, "Stream does not support writing.");
	IO_Writer w = { ._stream = s };
	return w;
}

#endif
