/*  =========================================================================
    xdp_sock - The xdp_sock class!

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
    xdp_sock - The xdp_sock class!
@discuss
@end
*/

#include "xdpiface_classes.h"

//  Structure of our class

struct _xdp_sock_t {
    //  Declare class properties here
};


//  --------------------------------------------------------------------------
//  Create a new xdp_sock

xdp_sock_t *
xdp_sock_new (xdp_iface_t *xdp_interface)
{
    xdp_sock_t *self = (xdp_sock_t *) zmalloc (sizeof (xdp_sock_t));
    assert (self);
    //  Initialize class properties here
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the xdp_sock

void
xdp_sock_destroy (xdp_sock_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        xdp_sock_t *self = *self_p;
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
xdp_sock_test (bool verbose)
{
    int ret = 0;
    const char *xdp_prog_path = "xdp_sock_bpf.o";

    printf (" * xdp_sock: ");

    //  @selftest
    //  Simple create/destroy test
    xdp_iface_t *xdp_iface = xdp_iface_new (XDP_IFACE_DEFAULT);
    assert (xdp_iface);

    ret = xdp_iface_load_program(xdp_iface, xdp_prog_path);
    if (0 != ret) {
        fprintf(stderr, "Failed to load program (%s)!", xdp_prog_path);
        goto exit;
    }

    //  @selftest
    //  Simple create/destroy test
    xdp_sock_t *self = xdp_sock_new (xdp_iface);
    assert (self);
    xdp_sock_destroy (&self);


    xdp_iface_unload_program(xdp_iface);

exit:
    xdp_iface_destroy (&xdp_iface);

    //  @end
    printf ("OK\n");
}
