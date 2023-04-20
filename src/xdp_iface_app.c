/*  =========================================================================
    xdp_iface_app - description

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
    xdp_iface_app -
@discuss
@end
*/
#include <sys/socket.h>
#include <signal.h>
#include <poll.h>

#include "xdpiface_classes.h"

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

int main (int argc, char *argv [])
{
    //  Insert main code here
    struct pollfd fd;
    int ret = 0;
    int i = 0;

    uint32_t batch_size = 30;
    uint32_t frames_rcvd = 0;
    const char *xdp_prog_path = "xdp_sock_bpf.o";

    char i_buffer[9000];
    size_t i_buffer_size = 0;

    size_t o_buffer_size = 1000;
    char o_buffer[9000];
    memset(o_buffer, 0x55, o_buffer_size);

    printf(" * xdp_sock: ");

    //  @selftest
    xdp_iface_t *xdp_iface = xdp_iface_new (XDP_IFACE_DEFAULT);
    assert (xdp_iface);

    ret = xdp_iface_load_program(xdp_iface, xdp_prog_path);
    if (0 != ret) {
        printf( "Failed to load program (%s)!", xdp_prog_path);
        goto exit;
    }

    xdp_sock_t *xdp_sock = xdp_sock_new (xdp_iface);
    xdp_sock_lookup_bpf_map(xdp_sock, xdp_iface, "xsks_map", 4, 4);

    xdp_sock_set_sockopt(xdp_sock, SO_PREFER_BUSY_POLL, 1);
    xdp_sock_set_sockopt(xdp_sock, SO_BUSY_POLL, 20);
    xdp_sock_set_sockopt(xdp_sock, SO_BUSY_POLL_BUDGET, batch_size);

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    xdp_sock_tx_batch_set_size(xdp_sock, batch_size);
    for (i = 0; i < batch_size; i++)
        xdp_sock_send (xdp_sock, o_buffer, o_buffer_size);
    xdp_sock_tx_batch_release(xdp_sock, batch_size);

    fd.fd = xdp_sock_get_fd(xdp_sock);
    fd.events = POLLIN;
    for (;;) {
        if (stop)
            break;

        ret = poll(&fd, 1, 1000);
        if (ret <= 0)
            continue;

        ret = xdp_sock_rx_batch_get_size (xdp_sock, &frames_rcvd, batch_size);
        if (0 != ret)
            continue;

        for (i = 0; i < frames_rcvd; i ++)
        {
            xdp_sock_recv (xdp_sock, i_buffer, &i_buffer_size);
            hex_dump(i_buffer, i_buffer_size);
        }

        xdp_sock_rx_batch_release(xdp_sock, frames_rcvd);
    }

    xdp_sock_destroy (&xdp_sock);
    xdp_iface_unload_program(xdp_iface);
exit:
    xdp_iface_destroy (&xdp_iface);

    return 0;
}
