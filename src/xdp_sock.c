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

/******************************** INCLUDE FILES *******************************/
#include <errno.h>
#include <sys/mman.h>
#include <linux/if_link.h>

#include <xdp/xsk.h>
//#include <xdp/libxdp.h>
//#include <bpf/libbpf.h>
//#include <bpf/bpf.h>

#include "xdpiface_classes.h"

/******************************** LOCAL DEFINES *******************************/
#define MAX_SOCKS 4

#define XDP_IFACE_XSK_FRAMES              (4 * 4096)
#define XDP_IFACE_XSK_FRAMESIZE           (XSK_UMEM__DEFAULT_FRAME_SIZE)


/********************************* TYPEDEFS ***********************************/

typedef struct _xsk_umem_info_t {
    struct xsk_ring_prod fq;
    struct xsk_ring_cons cq;
    struct xsk_umem *umem;
    void *buffer;
} xsk_umem_info_t;

//  Structure of xdp_sock class

struct _xdp_sock_t {
    struct xsk_ring_cons rx;
    struct xsk_ring_prod tx;
    struct xsk_socket *xsk;
    __u32 outstanding_tx;

    xsk_umem_info_t *umem;
    void *bufs;
};

/********************************* LOCAL DATA *********************************/

/******************************* LOCAL FUNCTIONS ******************************/

static xsk_umem_info_t *
xdp_iface_xsk_configure_umem(void *buffer, __u64 buff_size)
{
    xsk_umem_info_t *umem = NULL;
    struct xsk_umem_config cfg = {
        .fill_size = XSK_RING_PROD__DEFAULT_NUM_DESCS * 2,
        .comp_size = XSK_RING_CONS__DEFAULT_NUM_DESCS,
        .frame_size = XDP_IFACE_XSK_FRAMESIZE,
        .frame_headroom = XSK_UMEM__DEFAULT_FRAME_HEADROOM,
        .flags = 0
    };
    int ret = 0;

    umem = calloc(1, sizeof(*umem));
    if (umem == NULL) {
        fprintf(stderr, "Memmory allocation failed! err: \"%s\"\n", strerror(errno));
        return NULL;
    }

    ret = xsk_umem__create(&umem->umem, buffer, buff_size, &umem->fq, &umem->cq,
                   &cfg);
    if (0 != ret) {
        fprintf(stderr, "UMEM create failed! err: %d/\"%s\"\n", ret, strerror(errno));
        free(umem);
        return NULL;
    }
    umem->buffer = buffer;

    return umem;
}

static int
xdp_iface_xsk_populate_fill_ring(xsk_umem_info_t *umem)
{
    int ret = 0;
    int i = 0;
    __u32 idx = 0;

    ret = xsk_ring_prod__reserve(&umem->fq,
                     XSK_RING_PROD__DEFAULT_NUM_DESCS * 2, &idx);
    if (XSK_RING_PROD__DEFAULT_NUM_DESCS * 2 != ret) {
        fprintf(stderr, "Ring reserver failed! err: %d/\"%s\"\n", ret, strerror(errno));
    } else {
        for (i = 0; i < XSK_RING_PROD__DEFAULT_NUM_DESCS * 2; i++)
            *xsk_ring_prod__fill_addr(&umem->fq, idx++) = i * XDP_IFACE_XSK_FRAMESIZE;
        xsk_ring_prod__submit(&umem->fq, XSK_RING_PROD__DEFAULT_NUM_DESCS * 2);
    }

    return ret;
}

/***************************** INTERFACE FUNCTIONS ****************************/

//  --------------------------------------------------------------------------
//  Create a new xdp_sock

xdp_sock_t *
xdp_sock_new (xdp_iface_t *xdp_interface)
{
    assert(xdp_interface);
    struct xsk_socket_config cfg;
    int ret = 0;

    xdp_sock_t *self = (xdp_sock_t *) zmalloc (sizeof (xdp_sock_t));
    assert (self);

    //  Initialize class properties here
    self->bufs = mmap(NULL, XDP_IFACE_XSK_FRAMES * XDP_IFACE_XSK_FRAMESIZE,
            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (self->bufs == MAP_FAILED) {
        fprintf(stderr, "ERROR: mmap failed\n");
        free(self);
        return NULL;
    }

    /* Create socket */
    self->umem = xdp_iface_xsk_configure_umem(self->bufs,
        XDP_IFACE_XSK_FRAMES * XDP_IFACE_XSK_FRAMESIZE);
    xdp_iface_xsk_populate_fill_ring(self->umem);

    cfg.rx_size = XSK_RING_CONS__DEFAULT_NUM_DESCS;
    cfg.tx_size = XSK_RING_PROD__DEFAULT_NUM_DESCS;
    cfg.libxdp_flags = 0;
    cfg.xdp_flags = XDP_FLAGS_SKB_MODE;
    cfg.bind_flags = XDP_COPY;

    ret = xsk_socket__create(&self->xsk, xdp_iface_get_name(xdp_interface), 0, self->umem->umem,
                 &self->rx, &self->tx, &cfg);
    if (ret) {
        fprintf(stderr, "Failed to create socket! err: %d/\"%s\"\n", ret, strerror(errno));
        free(self);
        return NULL;
    }

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
        struct xsk_umem *umem = self->umem->umem;
        xsk_socket__delete(self->xsk);
        (void)xsk_umem__delete(umem);

        munmap(self->bufs, XDP_IFACE_XSK_FRAMES * XDP_IFACE_XSK_FRAMESIZE);
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
