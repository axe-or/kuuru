/* Utility file if you want to compile the library without potentially polluting a file with static functions. */
#define BASE_C_IMPLEMENTATION 1
#include "arena_allocator.h"
#include "assert.h"
#include "bytes_buffer.h"
#include "heap_allocator.h"
#include "io.h"
#include "memory.h"
#include "prelude.h"
#include "string.h"
#include "testing.h"
#include "utf8.h"

