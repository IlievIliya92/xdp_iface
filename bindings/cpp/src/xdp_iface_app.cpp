#include <sys/socket.h>
#include <cstring>
#include <iostream>

#include "xdp_iface.hpp"
#include "xdp_sock.hpp"

static void hexdump(void *ptr, int buflen) {
    unsigned char *buf = (unsigned char*)ptr;
    int i, j;
    for (i=0; i<buflen; i+=16) {
        printf("%06x: ", i);
        for (j=0; j<16; j++)
            if (i+j < buflen)
                printf("%02x ", buf[i+j]);
            else
                printf("   ");
        printf(" ");
        for (j=0; j<16; j++)
            if (i+j < buflen)
                printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
        printf("\n");
    }
}

int main (int argc, char *argv [])
{
    //  @selftest
    uint32_t batch_size = 30;
    uint32_t frames_rcvd = 0;

    size_t o_buffer_size = 1000;
    char o_buffer[9000];
    memset(o_buffer, 0x55, o_buffer_size);

    char i_buffer[9000];
    size_t i_buffer_size = 0;

    XdpIface xdp_iface(XDP_IFACE_DEFAULT);
    xdp_iface.loadProgram(XDP_IFACE_XDP_PROG_DEFAULT);

    XdpSock xdp_sock(&xdp_iface);
    xdp_sock.lookupBpfMap(&xdp_iface, XDP_SOCK_XSKS_MAP_DEFAULT, 4, 4);

    xdp_sock.setSockopt(XDP_SOCK_SO_PREFER_BUSY_POLL, 1);
    xdp_sock.setSockopt(XDP_SOCK_SO_BUSY_POLL, 20);
    xdp_sock.setSockopt(XDP_SOCK_SO_BUSY_POLL_BUDGET, batch_size);

    xdp_sock.txBatchSetSize(batch_size);
    for (int i = 0; i < batch_size; i++)
        xdp_sock.send (o_buffer, o_buffer_size);
    xdp_sock.txBatchRelease(batch_size);

    xdp_sock.rxBatchGetSize (&frames_rcvd, batch_size);
    for (int i = 0; i < frames_rcvd; i ++)
    {
        xdp_sock.recv (i_buffer, &i_buffer_size);
        hexdump(i_buffer, i_buffer_size);
    }
    xdp_sock.rxBatchRelease(frames_rcvd);
    std::cout << "Frames received: " << frames_rcvd << std::endl;

    xdp_iface.unloadProgram();

    return 0;
}
