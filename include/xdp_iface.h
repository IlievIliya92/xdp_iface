/*  =========================================================================
    xdp_iface - class description

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

#ifndef XDP_IFACE_H_INCLUDED
#define XDP_IFACE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @warning THE FOLLOWING @INTERFACE BLOCK IS AUTO-GENERATED BY ZPROJECT
//  @warning Please edit the model at "api/xdp_iface.api" to make changes.
//  @interface
//  This API is a draft, and may change without notice.
#ifdef XDPIFACE_BUILD_DRAFT_API

// The default network interface
#define XDP_IFACE_DEFAULT "lo"

//  *** Draft method, for development use, may change without warning ***
//  Create a new xdp iface
XDPIFACE_EXPORT xdp_iface_t *
    xdp_iface_new (const char *interface);

//  *** Draft method, for development use, may change without warning ***
//  Destroy the xdp_iface.
XDPIFACE_EXPORT void
    xdp_iface_destroy (xdp_iface_t **self_p);

//  *** Draft method, for development use, may change without warning ***
//  Shout once!
XDPIFACE_EXPORT int
    xdp_iface_load_xdp (xdp_iface_t *self, const char *xdp_program_file);

//  *** Draft method, for development use, may change without warning ***
//  Self test of this class.
XDPIFACE_EXPORT void
    xdp_iface_test (bool verbose);

#endif // XDPIFACE_BUILD_DRAFT_API
//  @end

#ifdef __cplusplus
}
#endif

#endif
