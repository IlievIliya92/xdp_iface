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

/******************************** INCLUDE FILES *******************************/
#include <net/if.h>
#include <xdp/libxdp.h>

#include "xdpiface_classes.h"

/******************************** LOCAL DEFINES *******************************/
#define XDP_IFACE_STRERR_BUFSIZE          1024

/********************************* TYPEDEFS ***********************************/

//  Structure of xdp_iface class
struct _xdp_iface_t {
    int ifindex;
    char *interface;

    enum xdp_attach_mode attach_mode;
    struct xdp_program *xdp_prog;
};

/********************************* LOCAL DATA *********************************/

/******************************* LOCAL FUNCTIONS ******************************/

/***************************** INTERFACE FUNCTIONS ****************************/
//  --------------------------------------------------------------------------
//  Create a new xdp_iface

xdp_iface_t *
xdp_iface_new (const char *interface)
{
    assert(interface);

    xdp_iface_t *self = (xdp_iface_t *) zmalloc (sizeof (xdp_iface_t));
    assert (self);

    //  Initialize class properties here
    self->ifindex = if_nametoindex(interface);
    if (!self->ifindex) {
        fprintf(stderr, "ERROR: interface \"%s\" does not exist\n", interface);
        free(self);
        return NULL;
    }
    self->interface = strdup(interface);

    /* Currently only skb mode is supported */
    self->attach_mode = XDP_MODE_SKB;

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
        free(self->interface);
        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

int
xdp_iface_load_program(xdp_iface_t *self, const char *xdp_prog_path)
{
    char errmsg[XDP_IFACE_STRERR_BUFSIZE];
    int err = 0;

    self->xdp_prog = xdp_program__open_file(xdp_prog_path, NULL, NULL);
    err = libxdp_get_error(self->xdp_prog);
    if (err) {
        libxdp_strerror(err, errmsg, sizeof(errmsg));
        fprintf(stderr, "ERROR: program loading failed: %s\n", errmsg);
    }
    else {
        err = xdp_program__attach(self->xdp_prog, self->ifindex, self->attach_mode, 0);
        if (err) {
            libxdp_strerror(err, errmsg, sizeof(errmsg));
            fprintf(stderr, "ERROR: attaching program failed: %s\n", errmsg);
        }
    }

    return err;
}

void
xdp_iface_unload_program(xdp_iface_t *self)
{
    xdp_program__detach(self->xdp_prog, self->ifindex, self->attach_mode, 0);
    xdp_program__close(self->xdp_prog);
}

const char *
xdp_iface_get_name (xdp_iface_t *self)
{
    return self->interface;
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
    int ret = 0;

    const char *xdp_prog_path = "xdp_sock_bpf.o";

    printf (" * xdp_iface: ");

    //  @selftest
    //  Simple create/destroy test
    xdp_iface_t *self = xdp_iface_new (XDP_IFACE_DEFAULT);
    assert (self);

    ret = xdp_iface_load_program(self, xdp_prog_path);
    if (0 != ret) {
        fprintf(stderr, "Failed to load program (%s)!", xdp_prog_path);
        goto exit;
    }

    xdp_iface_unload_program(self);

exit:
    xdp_iface_destroy (&self);

    //  @end
    printf ("OK\n");
}
