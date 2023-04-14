/*  =========================================================================
    xdpiface - generated layer of public API

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
    =========================================================================
*/

#ifndef XDPIFACE_LIBRARY_H_INCLUDED
#define XDPIFACE_LIBRARY_H_INCLUDED

//  Set up environment for the application

//  External dependencies

//  XDPIFACE version macros for compile-time API detection
#define XDPIFACE_VERSION_MAJOR 1
#define XDPIFACE_VERSION_MINOR 1
#define XDPIFACE_VERSION_PATCH 0

#define XDPIFACE_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define XDPIFACE_VERSION \
    XDPIFACE_MAKE_VERSION(XDPIFACE_VERSION_MAJOR, XDPIFACE_VERSION_MINOR, XDPIFACE_VERSION_PATCH)

#include <stdbool.h>

#if (defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER
#define XDPIFACE_PRIVATE __attribute__ ((visibility ("hidden")))
#define XDPIFACE_EXPORT __attribute__ ((visibility ("default")))
#else
#define XDPIFACE_PRIVATE
#define XDPIFACE_EXPORT
#endif

//  Project has no stable classes, so we build the draft API
#undef  XDPIFACE_BUILD_DRAFT_API
#define XDPIFACE_BUILD_DRAFT_API

//  Opaque class structures to allow forward references
//  These classes are stable or legacy and built in all releases
//  Draft classes are by default not built in stable releases
#ifdef XDPIFACE_BUILD_DRAFT_API
typedef struct _xdp_iface_t xdp_iface_t;
#define XDP_IFACE_T_DEFINED
#endif // XDPIFACE_BUILD_DRAFT_API


//  Public classes, each with its own header file
#ifdef XDPIFACE_BUILD_DRAFT_API
#include "xdp_iface.h"
#endif // XDPIFACE_BUILD_DRAFT_API

#ifdef XDPIFACE_BUILD_DRAFT_API

#ifdef __cplusplus
extern "C" {
#endif

//  Self test for private classes
XDPIFACE_EXPORT void
    xdpiface_private_selftest (bool verbose, const char *subtest);

#ifdef __cplusplus
}
#endif
#endif // XDPIFACE_BUILD_DRAFT_API

#endif
