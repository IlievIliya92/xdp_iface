/*  =========================================================================
    xdpiface_classes - private header file

    MIT License

    Copyright (c) [2023] XDP Interface

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
    =========================================================================
*/

#ifndef XDPIFACE_CLASSES_H_INCLUDED
#define XDPIFACE_CLASSES_H_INCLUDED

//  Platform definitions, must come first
#include "platform.h"

//  Asserts check the invariants of methods. If they're not
//  fulfilled the program should fail fast. Therefore enforce them!
#ifdef NDEBUG
  #undef NDEBUG
  #include <assert.h>
  #define NDEBUG
#else
  #include <assert.h>
#endif

//  External API
#include "../include/xdpiface.h"

//  Private external dependencies

//  Opaque class structures to allow forward references

//  Extra headers

//  Internal API
// common definitions and idioms from czmq_prelude.h, which are used in generated code
#if ! defined(__CZMQ_PRELUDE_H_INCLUDED__)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define streq(s1,s2)    (!strcmp ((s1), (s2)))
#define strneq(s1,s2)   (strcmp ((s1), (s2)))
//  Replacement for malloc() which asserts if we run out of heap, and
//  which zeroes the allocated block.
static inline void *
safe_malloc (size_t size, const char *file, unsigned line)
{
//     printf ("%s:%u %08d\n", file, line, (int) size);
    void *mem = calloc (1, size);
    if (mem == NULL) {
        fprintf (stderr, "FATAL ERROR at %s:%u\n", file, line);
        fprintf (stderr, "OUT OF MEMORY (malloc returned NULL)\n");
        fflush (stderr);
        abort ();
    }
    return mem;
}

//  Define _ZMALLOC_DEBUG if you need to trace memory leaks using e.g. mtrace,
//  otherwise all allocations will claim to come from czmq_prelude.h. For best
//  results, compile all classes so you see dangling object allocations.
//  _ZMALLOC_PEDANTIC does the same thing, but its intention is to propagate
//  out of memory condition back up the call stack.
#if defined (_ZMALLOC_DEBUG) || defined (_ZMALLOC_PEDANTIC)
#   define zmalloc(size) calloc(1,(size))
#else
#   define zmalloc(size) safe_malloc((size), __FILE__, __LINE__)
#endif
#endif // __CZMQ_PRELUDE_H_INCLUDED__



//  *** To avoid double-definitions, only define if building without draft ***
#ifndef XDPIFACE_BUILD_DRAFT_API

//  Self test for private classes
XDPIFACE_PRIVATE void
    xdpiface_private_selftest (bool verbose, const char *subtest);

#endif // XDPIFACE_BUILD_DRAFT_API

#endif
