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

/*
@header
    xdp_iface -
@discuss
@end
*/

#include <net/if.h>

#include <xdp/xsk.h>
#include <xdp/libxdp.h>

#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "xdpiface_classes.h"

//  Structure of xdp_iface class

struct _xdp_iface_t {
    struct xdp_program *xdp_prog;
};


//  --------------------------------------------------------------------------
//  Create a new xdp_iface

xdp_iface_t *
xdp_iface_new (const char *interface)
{
    xdp_iface_t *self = (xdp_iface_t *) zmalloc (sizeof (xdp_iface_t));
    assert (self);
    //  Initialize class properties here
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the xdp_iface

void
xdp_iface_destroy (xdp_iface_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        xdp_iface_t *self = *self_p;
        //  Free class properties here
        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

//  --------------------------------------------------------------------------
//  Self test of this class

// If your selftest reads SCMed fixture data, please keep it in
// src/selftest-ro; if your test creates filesystem objects, please
// do so under src/selftest-rw.
// The following pattern is suggested for C selftest code:
//    char *filename = NULL;
//    filename = zsys_sprintf ("%s/%s", SELFTEST_DIR_RO, "mytemplate.file");
//    assert (filename);
//    ... use the "filename" for I/O ...
//    zstr_free (&filename);
// This way the same "filename" variable can be reused for many subtests.
#define SELFTEST_DIR_RO "src/selftest-ro"
#define SELFTEST_DIR_RW "src/selftest-rw"

void
xdp_iface_test (bool verbose)
{
    printf (" * xdp_iface: ");

    //  @selftest
    //  Simple create/destroy test
    xdp_iface_t *self = xdp_iface_new (XDP_IFACE_DEFAULT);
    assert (self);
    xdp_iface_destroy (&self);
    //  @end
    printf ("OK\n");
}
