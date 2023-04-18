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
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <linux/if_link.h>

#include <xdp/xsk.h>
#include <xdp/libxdp.h>
//#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "xdp_log.h"
#include "xdpiface_classes.h"

/******************************** LOCAL DEFINES *******************************/
#define XDP_MODULE_NAME "xdp_sock"
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
    __u32 idx_rx;
    __u32 idx_fq;
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
xdp_sock_xsk_configure_umem(void *buffer, __u64 buff_size)
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
        XDP_LOG_MSG(XDP_LOG_ERROR, "Memmory allocation failed! err: \"%s\"\n", strerror(errno));
        return NULL;
    }

    ret = xsk_umem__create(&umem->umem, buffer, buff_size, &umem->fq, &umem->cq,
                   &cfg);
    if (0 != ret) {
        XDP_LOG_MSG(XDP_LOG_ERROR, "UMEM create failed! err: %d/\"%s\"\n", ret, strerror(errno));
        free(umem);
        return NULL;
    }
    umem->buffer = buffer;

    return umem;
}

static int
xdp_sock_xsk_populate_fill_ring(xsk_umem_info_t *umem)
{
    int ret = 0;
    int i = 0;
    __u32 idx = 0;

    ret = xsk_ring_prod__reserve(&umem->fq,
                     XSK_RING_PROD__DEFAULT_NUM_DESCS * 2, &idx);
    if (XSK_RING_PROD__DEFAULT_NUM_DESCS * 2 != ret) {
        XDP_LOG_MSG(XDP_LOG_ERROR, "Ring reserver failed! err: %d/\"%s\"\n", ret, strerror(errno));
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
        XDP_LOG_MSG(XDP_LOG_ERROR, "mmap failed\n");
        free(self);
        return NULL;
    }

    /* Create socket */
    self->umem = xdp_sock_xsk_configure_umem(self->bufs,
        XDP_IFACE_XSK_FRAMES * XDP_IFACE_XSK_FRAMESIZE);
    xdp_sock_xsk_populate_fill_ring(self->umem);

    cfg.rx_size = XSK_RING_CONS__DEFAULT_NUM_DESCS;
    cfg.tx_size = XSK_RING_PROD__DEFAULT_NUM_DESCS;
    cfg.libxdp_flags = 0;
    cfg.xdp_flags = XDP_FLAGS_SKB_MODE;
    cfg.bind_flags = XDP_COPY | XDP_USE_NEED_WAKEUP;

    ret = xsk_socket__create(&self->xsk, xdp_iface_get_name(xdp_interface), 0, self->umem->umem,
                 &self->rx, &self->tx, &cfg);
    if (ret) {
        XDP_LOG_MSG(XDP_LOG_ERROR, "Failed to create socket! err: %d/\"%s\"\n", ret, strerror(errno));
        /** TODO: free res */
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

int
xdp_sock_lookup_bpf_map(xdp_sock_t *self, xdp_iface_t *xdp_interface, const char *map_name,
    uint32_t key_size, uint32_t value_size)
{
    struct xdp_program *xdp_prog = (struct xdp_program *)xdp_iface_get_prog(xdp_interface);
    __u32 i = 0;
    __u32 *map_ids = NULL;
    __u32 num_maps = 0;
    __u32 prog_len = sizeof(struct bpf_prog_info);
    __u32 map_len = sizeof(struct bpf_map_info);
    struct bpf_prog_info prog_info = {};
    int fd, err, xsks_map_fd = -ENOENT;
    struct bpf_map_info map_info;
    int sock_fd = 0;
    int key = 0;
    int xdp_prog_fd = -1;

    xdp_prog_fd = xdp_program__fd(xdp_prog);
    err = bpf_obj_get_info_by_fd(xdp_prog_fd, &prog_info, &prog_len);
    if (err)
        return err;

    num_maps = prog_info.nr_map_ids;

    map_ids = calloc(prog_info.nr_map_ids, sizeof(*map_ids));
    if (!map_ids)
        return -ENOMEM;

    memset(&prog_info, 0, prog_len);
    prog_info.nr_map_ids = num_maps;
    prog_info.map_ids = (__u64)(unsigned long)map_ids;

    err = bpf_obj_get_info_by_fd(xdp_prog_fd, &prog_info, &prog_len);
    if (err) {
        free(map_ids);
        return err;
    }

    for (i = 0; i < prog_info.nr_map_ids; i++) {
        fd = bpf_map_get_fd_by_id(map_ids[i]);
        if (fd < 0)
            continue;

        memset(&map_info, 0, map_len);
        err = bpf_obj_get_info_by_fd(fd, &map_info, &map_len);
        if (err) {
            close(fd);
            continue;
        }

        if (!strncmp(map_info.name, map_name, sizeof(map_info.name)) &&
            map_info.key_size == key_size && map_info.value_size == value_size) {
            xsks_map_fd = fd;
            break;
        }

        close(fd);
    }
    free(map_ids);

    if (xsks_map_fd < 0) {
        XDP_LOG_MSG(XDP_LOG_ERROR, "No xsks map found: %s\n", strerror(xsks_map_fd));
        return -1;
    }

    sock_fd = xsk_socket__fd(self->xsk);
    bpf_map_update_elem(xsks_map_fd, &key, &fd, 0);

    return 0;
}

int
xdp_sock_set_sockopt (xdp_sock_t *self, int opt_type, int opt_value)
{
    int ret = 0;

    ret = setsockopt(xsk_socket__fd(self->xsk), SOL_SOCKET, opt_type,
               (void *)&opt_value, sizeof(opt_value));

    return ret;
}

int
xdp_sock_get_fd (xdp_sock_t *self)
{
    return xsk_socket__fd(self->xsk);
}

int
xdp_sock_get_batch (xdp_sock_t *self, uint32_t *pkts_recvd, uint32_t nb)
{
    int ret = 0;
    self->idx_rx = 0;
    self->idx_fq = 0;

    *pkts_recvd = xsk_ring_cons__peek(&self->rx, nb, &self->idx_rx);
    if (0 == *pkts_recvd) {
        if (xsk_ring_prod__needs_wakeup(&self->umem->fq)) {
            recvfrom(xsk_socket__fd(self->xsk), NULL, 0, MSG_DONTWAIT, NULL, NULL);
        }

        return -1;
    }

    ret = xsk_ring_prod__reserve(&self->umem->fq, *pkts_recvd, &self->idx_fq);
    while (ret != *pkts_recvd) {
        if (ret < 0)
        {
            XDP_LOG_MSG(XDP_LOG_ERROR, "Sock get batch failed....");
            return -1;
        }

        if (xsk_ring_prod__needs_wakeup(&self->umem->fq)) {
            recvfrom(xsk_socket__fd(self->xsk), NULL, 0, MSG_DONTWAIT, NULL, NULL);
        }
        ret = xsk_ring_prod__reserve(&self->umem->fq, *pkts_recvd, &self->idx_fq);
    }

    return 0;
}

int
xdp_sock_release_batch (xdp_sock_t *self, uint32_t pkts_recvd)
{
    xsk_ring_prod__submit(&self->umem->fq, pkts_recvd);
    xsk_ring_cons__release(&self->rx, pkts_recvd);

    return 0;
}

int
xdp_sock_recv (xdp_sock_t *self, char *buffer, size_t *buffer_size)
{
    __u64 addr = xsk_ring_cons__rx_desc(&self->rx, self->idx_rx)->addr;
    *buffer_size = xsk_ring_cons__rx_desc(&self->rx, self->idx_rx++)->len;
    __u64 orig = xsk_umem__extract_addr(addr);

    addr = xsk_umem__add_offset_to_addr(addr);
    /* This can be avoided but it will be tricky to pass data through python bindings */
    memcpy(buffer, xsk_umem__get_data(self->umem->buffer, addr), *buffer_size);

    *xsk_ring_prod__fill_addr(&self->umem->fq, self->idx_fq++) = orig;
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

static volatile sig_atomic_t stop = false;

static void sig_handler(int sig)
{
    stop = true;
}

static void hex_dump(void *pkt, size_t length)
{
    const unsigned char *address = (unsigned char *)pkt;
    const unsigned char *line = address;
    size_t line_size = 32;
    unsigned char c;
    char buf[32];
    int i = 0;

    printf("length = %zu\n", length);
    printf("%s | ", buf);
    while (length-- > 0) {
        printf("%02X ", *address++);
        if (!(++i % line_size) || (length == 0 && i % line_size)) {
            if (length == 0) {
                while (i++ % line_size)
                    printf("__ ");
            }
            printf(" | ");  /* right close */
            while (line < address) {
                c = *line++;
                printf("%c", (c < 33 || c == 255) ? 0x2E : c);
            }
            printf("\n");
            if (length > 0)
                printf("%s | ", buf);
        }
    }
    printf("\n");
}

void
xdp_sock_test (bool verbose)
{
    struct pollfd fd;
    int ret = 0;
    int i = 0;

    uint32_t batch_size = 64;
    uint32_t pkts_recvd = 0;
    const char *xdp_prog_path = "xdp_sock_bpf.o";

    char i_buffer[9000];
    size_t i_buffer_size = 0;

    XDP_LOG_MSG(XDP_LOG_INFO, " * xdp_sock: ");

    //  @selftest
    xdp_iface_t *xdp_iface = xdp_iface_new (XDP_IFACE_DEFAULT);
    assert (xdp_iface);

    ret = xdp_iface_load_program(xdp_iface, xdp_prog_path);
    if (0 != ret) {
        XDP_LOG_MSG(XDP_LOG_ERROR, "Failed to load program (%s)!", xdp_prog_path);
        goto exit;
    }

    xdp_sock_t *self = xdp_sock_new (xdp_iface);
    xdp_sock_lookup_bpf_map(self, xdp_iface, "xsks_map", 4, 4);

    xdp_sock_set_sockopt(self, SO_PREFER_BUSY_POLL, 1);
    xdp_sock_set_sockopt(self, SO_BUSY_POLL, 20);
    xdp_sock_set_sockopt(self, SO_BUSY_POLL_BUDGET, batch_size);

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    fd.fd = xdp_sock_get_fd(self);
    fd.events = POLLIN;
    for (;;) {
        if (stop)
            break;

        ret = poll(&fd, 1, 1000);
        if (ret <= 0)
            continue;

        ret = xdp_sock_get_batch (self, &pkts_recvd, batch_size);
        if (0 != ret)
            continue;

        for (i = 0; i < pkts_recvd; i ++)
        {
            xdp_sock_recv (self, i_buffer, &i_buffer_size);
            hex_dump(i_buffer, i_buffer_size);
        }

        xdp_sock_release_batch(self, pkts_recvd);
    }

    xdp_sock_destroy (&self);
    xdp_iface_unload_program(xdp_iface);
exit:
    xdp_iface_destroy (&xdp_iface);

    //  @end
    XDP_LOG_MSG(XDP_LOG_INFO, "OK\n");
}
