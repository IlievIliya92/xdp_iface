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
#include <sched.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <linux/if_link.h>

#include <xdp/xsk.h>
#include <xdp/libxdp.h>

#include "xdp_log.h"
#include "xdpiface_classes.h"

/******************************** LOCAL DEFINES *******************************/
#define XDP_MODULE_NAME "xdp_sock"
#define MAX_SOCKS 4

#define XDP_IFACE_XSK_FRAMES              (4 * 4096)
#define XDP_IFACE_XSK_FRAMESIZE           (XSK_UMEM__DEFAULT_FRAME_SIZE)

#define XDP_SCHED_PRI__DEFAULT  0
/********************************* TYPEDEFS ***********************************/

typedef struct _xsk_umem_info_t {
    struct xsk_ring_prod fq;
    struct xsk_ring_cons cq;
    struct xsk_umem *umem;
    void *buffer;
} xsk_umem_info_t;

//  Structure of xdp_sock class

struct _xdp_sock_t {
    __u32 idx_fq; /* producer queue */
    __u32 idx_rx;
    struct xsk_ring_cons rx;
    __u32 idx_tx;
    __u32 idx_tx_batch;
    struct xsk_ring_prod tx;
    __u32 pending_frames_tx;
    struct xsk_socket *xsk;

    xsk_umem_info_t *umem;
    void *bufs;

    /* Schedule priority */
    int schprio;
};

/********************************* LOCAL DATA *********************************/

/******************************* LOCAL FUNCTIONS ******************************/

static inline xsk_umem_info_t *
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

static inline int
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

static inline void
xdp_sock_tx_send_trigger(xdp_sock_t *self, int frames_send)
{
    __u32 idx;
    unsigned int rcvd;
    int ret = 0;

    if (!self->pending_frames_tx)
        return;

    if (xsk_ring_prod__needs_wakeup(&self->tx)) {
        ret = sendto(xsk_socket__fd(self->xsk), NULL, 0, MSG_DONTWAIT, NULL, 0);
        if (ret != 0)
            XDP_LOG_MSG(XDP_LOG_ERROR, "TX failed %s", strerror(errno));
    }

    rcvd = xsk_ring_cons__peek(&self->umem->cq, frames_send, &idx);
    if (rcvd > 0) {
        xsk_ring_cons__release(&self->umem->cq, rcvd);
        self->pending_frames_tx -= rcvd;
    }
}

/***************************** INTERFACE FUNCTIONS ****************************/

//  --------------------------------------------------------------------------
/**
 *
 * Create new xdp_sock object.
 *
 * Returns:
 *     On success new xdp_sock object, or NULL if the new xdp_sock could not be created.
 */
xdp_sock_t *
xdp_sock_new (xdp_iface_t *xdp_interface)
{
    assert(xdp_interface);

    struct xsk_socket_config cfg;
    struct sched_param schparam;
    /* the standard round-robin time-sharing policy; */
    static int schpolicy = SCHED_OTHER;
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

    self->idx_fq = 0;
    self->idx_tx = 0;
    self->pending_frames_tx = 0;
    self->idx_rx = 0;
    self->schprio = XDP_SCHED_PRI__DEFAULT;

    /**
     *  Configure sched priority for better wake-up accuracy
     * https://man7.org/linux/man-pages/man2/sched_setscheduler.2.html
     */
    memset(&schparam, 0, sizeof(schparam));
    schparam.sched_priority = self->schprio;
    ret = sched_setscheduler(0, schpolicy, &schparam);
    if (ret) {
        XDP_LOG_MSG(XDP_LOG_ERROR, "Error(%d) in setting priority(%d): %s\n",
            errno, self->schprio, strerror(errno));
        free(self);
        return NULL;
    }

    return self;
}


//  --------------------------------------------------------------------------
/**
 *
 * Destroy xdp_sock object. You must use this for any tcp server created via the
 * xdp_sock_new method.
 *
 * Parameters:
 *      self_p (xdp_sock_t **): pointer to xdp_sock_t object reference,
 *                               so the destructor can nullify it
 *
 * Returns:
 *      None (void)
 */
void
xdp_sock_destroy (xdp_sock_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        xdp_sock_t *self = *self_p;
        //  Free class properties here
        xsk_socket__delete(self->xsk);
        (void)xsk_umem__delete(self->umem->umem);
        free(self->umem);

        munmap(self->bufs, XDP_IFACE_XSK_FRAMES * XDP_IFACE_XSK_FRAMESIZE);
        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

/**
 *
 * Attach XSKS map to socket fd
 *
 * Parameters:
 *      self_p (xdp_sock_t *): pointer to xdp_sock_t object
 *      xdp_interface (xdp_iface_t *): pointer to xdp_iface_t object
 *      map_name (const char *): map name
 *      key_size (uint32_t): map keys size
 *      value_size (uint32_t): map values size
 *
 * Returns:
 *      0 on success, err value on failiure
 */
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


/**
 *
 * Set socket option
 *
 * Parameters:
 *      self_p (xdp_sock_t *): pointer to xdp_sock_t object
 *      opt_type (int): option type
 *      opt_value (int): option value
 *
 * Returns:
 *      0 on success, err value on failiure
 */
int
xdp_sock_set_sockopt (xdp_sock_t *self, int opt_type, int opt_value)
{
    int ret = 0;

    ret = setsockopt(xsk_socket__fd(self->xsk), SOL_SOCKET, opt_type,
               (void *)&opt_value, sizeof(opt_value));

    return ret;
}


/**
 *
 * Get socket descriptor
 *
 * Parameters:
 *      self_p (xdp_sock_t *): pointer to xdp_sock_t object
 *
 * Returns:
 *      socket descriptor
 */
int
xdp_sock_get_fd (xdp_sock_t *self)
{
    return xsk_socket__fd(self->xsk);
}

/**
 *
 * Get size, number of frames received
 *
 * Parameters:
 *      self_p (xdp_sock_t *): pointer to xdp_sock_t object
 *      frames_rcvd (uint32_t *): number of received frames
 *      nb (uint32_t): maximum number of frames to receive
 *
 * Returns:
 *      0 on success, -1 on failiure
 */
int
xdp_sock_rx_batch_get_size (xdp_sock_t *self, uint32_t *frames_rcvd, uint32_t nb)
{
    int ret = 0;
    self->idx_rx = 0;
    self->idx_fq = 0;

    *frames_rcvd = xsk_ring_cons__peek(&self->rx, nb, &self->idx_rx);
    if (0 == *frames_rcvd) {
        if (xsk_ring_prod__needs_wakeup(&self->umem->fq)) {
            recvfrom(xsk_socket__fd(self->xsk), NULL, 0, MSG_DONTWAIT, NULL, NULL);
        }

        return -1;
    }

    ret = xsk_ring_prod__reserve(&self->umem->fq, *frames_rcvd, &self->idx_fq);
    while (ret != *frames_rcvd) {
        if (ret < 0)
        {
            XDP_LOG_MSG(XDP_LOG_ERROR, "Sock get batch failed failed....");
            return -1;
        }

        if (xsk_ring_prod__needs_wakeup(&self->umem->fq)) {
            recvfrom(xsk_socket__fd(self->xsk), NULL, 0, MSG_DONTWAIT, NULL, NULL);
        }
        ret = xsk_ring_prod__reserve(&self->umem->fq, *frames_rcvd, &self->idx_fq);
    }

    return 0;
}

/**
 *
 * Release received batch
 *
 * Parameters:
 *      self_p (xdp_sock_t *): pointer to xdp_sock_t object
 *      frames_rcvd (uint32_t): number of received frames to release
 *
 * Returns:
 *      0 on success
 */
int
xdp_sock_rx_batch_release (xdp_sock_t *self, uint32_t frames_rcvd)
{
    xsk_ring_prod__submit(&self->umem->fq, frames_rcvd);
    xsk_ring_cons__release(&self->rx, frames_rcvd);

    return 0;
}


/**
 *
 * Read/receive a frame from the RX batch
 *
 * Parameters:
 *      self_p (xdp_sock_t *): pointer to xdp_sock_t object
 *      buffer (char *): receive buffer
 *      buffer_size (size_t *): size of the received buffer
 *
 * Returns:
 *      0 on success
 */
int
xdp_sock_recv (xdp_sock_t *self, char *buffer, size_t *buffer_size)
{
    __u64 addr = xsk_ring_cons__rx_desc(&self->rx, self->idx_rx)->addr;
    *buffer_size = xsk_ring_cons__rx_desc(&self->rx, self->idx_rx++)->len;
    __u64 orig = xsk_umem__extract_addr(addr);

    addr = xsk_umem__add_offset_to_addr(addr);
    /* This can be avoided but it will be tricky to pass data through python bindings */
    memcpy(buffer, xsk_umem__get_data(self->umem->buffer, addr), *buffer_size);
    XDP_LOG_HEXDUMP(XDP_LOG_TRACE, "Inbound buffer", buffer, *buffer_size);

    *xsk_ring_prod__fill_addr(&self->umem->fq, self->idx_fq++) = orig;

    return 0;
}


/**
 *
 * Set size of the TX batch
 *
 * Parameters:
 *      self_p (xdp_sock_t *): pointer to xdp_sock_t object
 *      nb (uint32_t): number of slots to prepare in the tx ring
 *
 * Returns:
 *      0 on success, -1 on failiure
 */
int
xdp_sock_tx_batch_set_size (xdp_sock_t *self, uint32_t nb)
{
    int ret = 0;

    while(xsk_ring_prod__reserve(&self->tx, nb, &self->idx_tx_batch) < nb) {
        ret = sendto(xsk_socket__fd(self->xsk), NULL, 0, MSG_DONTWAIT, NULL, 0);
        if (ret >= 0 || errno == ENOBUFS || errno == EAGAIN ||
            errno == EBUSY || errno == ENETDOWN)
            return -1;
    }

    return 0;
}


/**
 *
 * Trigger send of the frames in the TX batch
 *
 * Parameters:
 *      self_p (xdp_sock_t *): pointer to xdp_sock_t object
 *      frames_send (uint32_t): number of frames to send
 *
 * Returns:
 *      0 on success
 */
int
xdp_sock_tx_batch_release (xdp_sock_t *self, uint32_t frames_send)
{
    int opt_retries = 3;
    bool pending = false;

    /**
     * TODO: rework this part
     */
    xsk_ring_prod__submit(&self->tx, frames_send);
    self->pending_frames_tx += frames_send;
    xdp_sock_tx_send_trigger(self, frames_send);

    if (self->pending_frames_tx)
    {
        do {
            pending = false;
            if (self->pending_frames_tx) {
                xdp_sock_tx_send_trigger(self, frames_send);
                pending = !!self->pending_frames_tx;
            }
            XDP_LOG_MSG(XDP_LOG_INFO, "Pending: %d", pending);
        } while (pending && opt_retries-- > 0);
    }
    self->idx_tx = 0;

    return 0;
}

/**
 *
 * Send/place a frame in the TX ring
 *
 * Parameters:
 *      self_p (xdp_sock_t *): pointer to xdp_sock_t object
 *      buffer (char *): buffer to send
 *      buffer_size (size_t): size of the buffer to send
 *
 * Returns:
 *      0 on success
 */
int
xdp_sock_send (xdp_sock_t *self, char *buffer, size_t buffer_size)
{
    if (self->idx_tx >= 64) {
        XDP_LOG_MSG(XDP_LOG_WARNING, "TX batch full!");
        return -1;
    }

    struct xdp_desc *tx_desc = xsk_ring_prod__tx_desc(&self->tx,
                                  (self->idx_tx_batch + self->idx_tx));
    tx_desc->addr = self->idx_tx * XDP_IFACE_XSK_FRAMESIZE;
    tx_desc->len = buffer_size;

    XDP_LOG_HEXDUMP(XDP_LOG_TRACE, "Outbound buffer", buffer, buffer_size);
    memcpy(xsk_umem__get_data(self->umem->buffer, tx_desc->addr), buffer, buffer_size);
    self->idx_tx++;

    return 0;
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

    uint32_t batch_size = 64;
    uint32_t frames_rcvd = 0;
    XDP_LOG_MSG(XDP_LOG_INFO, " * xdp_sock: ");

    //  @selftest
    xdp_iface_t *xdp_iface = xdp_iface_new (XDP_IFACE_DEFAULT);
    if (xdp_iface == NULL) {
        XDP_LOG_MSG(XDP_LOG_ERROR, "Failed to create new xdp iface!");
        return;
    }

    ret = xdp_iface_load_program(xdp_iface, XDP_IFACE_XDP_PROG_DEFAULT);
    if (0 != ret) {
        XDP_LOG_MSG(XDP_LOG_ERROR, "Failed to load program (%s)!", XDP_IFACE_XDP_PROG_DEFAULT);
        goto exit;
    }

    xdp_sock_t *self = xdp_sock_new (xdp_iface);
    if (self == NULL) {
        XDP_LOG_MSG(XDP_LOG_ERROR, "Failed to create new xdp socket!");
        goto unload;
    }

    xdp_sock_lookup_bpf_map(self, xdp_iface, XDP_SOCK_XSKS_MAP_DEFAULT, 4, 4);

    xdp_sock_set_sockopt(self, XDP_SOCK_SO_PREFER_BUSY_POLL, 1);
    xdp_sock_set_sockopt(self, XDP_SOCK_SO_BUSY_POLL, 20);
    xdp_sock_set_sockopt(self, XDP_SOCK_SO_BUSY_POLL_BUDGET, batch_size);

    xdp_sock_destroy (&self);

unload:
    xdp_iface_unload_program(xdp_iface);
exit:
    xdp_iface_destroy (&xdp_iface);

    //  @end
    XDP_LOG_MSG(XDP_LOG_INFO, "OK\n");
}
