# Examples

The following examples illustrate how to send & receive a batch of 30 frames using
the `xdpiface` lib.

````{tab} C
```C
#include <sys/socket.h>
#include "xdpiface_classes.h"

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
    int i = 0;
    int ret = 0;

    uint32_t batch_size = 30;
    uint32_t frames_rcvd = 0;
    const char *xdp_prog_path = "./xdp_sock_bpf.o";

    char i_buffer[9000];
    size_t i_buffer_size = 0;

    size_t o_buffer_size = 1000;
    char o_buffer[9000];
    memset(o_buffer, 0x55, o_buffer_size);

    xdp_iface_t *xdp_iface = xdp_iface_new ("lo");
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

    xdp_sock_tx_batch_set_size(xdp_sock, batch_size);
    for (i = 0; i < batch_size; i++)
        xdp_sock_send (xdp_sock, o_buffer, o_buffer_size);
    xdp_sock_tx_batch_release(xdp_sock, batch_size);

    xdp_sock_rx_batch_get_size (xdp_sock, &frames_rcvd, batch_size);
    for (i = 0; i < frames_rcvd; i ++)
    {
        xdp_sock_recv (xdp_sock, i_buffer, &i_buffer_size);
        hex_dump(i_buffer, i_buffer_size);
    }
    xdp_sock_rx_batch_release(xdp_sock, frames_rcvd);

    xdp_sock_destroy (&xdp_sock);
    xdp_iface_unload_program(xdp_iface);
exit:
    xdp_iface_destroy (&xdp_iface);

    return 0;
}
```
````

````{tab} C++
```c++
#include <sys/socket.h>
#include <cstring>
#include <iostream>

#include "xdpiface.hpp"

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
    uint32_t batch_size = 30;
    uint32_t frames_rcvd = 0;

    size_t o_buffer_size = 1000;
    char o_buffer[9000];
    memset(o_buffer, 0x55, o_buffer_size);

    char i_buffer[9000];
    size_t i_buffer_size = 0;

    const std::string xdp_prog_path = "./xdp_sock_bpf.o";
    const std::string interface = "lo";

    XdpIface xdp_iface(interface);
    xdp_iface.loadProgram(xdp_prog_path);

    XdpSock xdp_sock(&xdp_iface);
    xdp_sock.lookupBpfMap(&xdp_iface, "xsks_map", 4, 4);

    xdp_sock.setSockopt(SO_PREFER_BUSY_POLL, 1);
    xdp_sock.setSockopt(SO_BUSY_POLL, 20);
    xdp_sock.setSockopt(SO_BUSY_POLL_BUDGET, batch_size);

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
```
````

````{tab} Python
```python
import xdpiface

from ctypes import c_int, create_string_buffer, c_size_t

def main():
    batch_size = 30

    xdp_iface = xdpiface.XdpIface("lo")
    xdp_iface.load_program("./xdp_sock_bpf.o")

    xdp_sock = xdpiface.XdpSock(xdp_iface)
    xdp_sock.lookup_bpf_map(xdp_iface, b"xsks_map", 4, 4)

    xdp_sock.set_sockopt(xdpiface.SO_PREFER_BUSY_POLL, 1)
    xdp_sock.set_sockopt(xdpiface.SO_BUSY_POLL, 20)
    xdp_sock.set_sockopt(xdpiface.SO_BUSY_POLL_BUDGET, batch_size)

    frames_recd = c_int(0)
    i_buffer = create_string_buffer(9000)
    i_buffer_size = c_size_t(0)
    frames_received = 0
    o_buffer = create_string_buffer(1500)
    o_buffer.raw = bytearray([0x55] * 1500)
    o_buffer_size = c_size_t(1500)

    xdp_sock.tx_batch_set_size(batch_size);
    for i in range(batch_size):
        xdp_sock.send (o_buffer, o_buffer_size);
    xdp_sock.tx_batch_release(batch_size);

    xdp_sock.rx_batch_get_size (frames_recd, batch_size);
    for i in range(frames_recd.value):
        xdp_sock.recv (i_buffer, i_buffer_size)
        print(hexdump(i_buffer.raw[:i_buffer_size.value]))
    xdp_sock.rx_batch_release(frames_recd.value);

    xdp_iface.unload_program()

class hexdump:
    def __init__(self, buf, off=0):
        self.buf = buf
        self.off = off

    def __iter__(self):
        last_bs, last_line = None, None
        for i in range(0, len(self.buf), 16):
            bs = bytearray(self.buf[i : i + 16])
            line = "{:08x}  {:23}  {:23}  |{:16}|".format(
                self.off + i,
                " ".join(("{:02x}".format(x) for x in bs[:8])),
                " ".join(("{:02x}".format(x) for x in bs[8:])),
                "".join((chr(x) if 32 <= x < 127 else "." for x in bs)),
            )
            if bs == last_bs:
                line = "*"
            if bs != last_bs or line != last_line:
                yield line
            last_bs, last_line = bs, line
        yield "{:08x}".format(self.off + len(self.buf))

    def __str__(self):
        return "\n".join(self)

    def __repr__(self):
        return "\n".join(self)

if __name__ == '__main__':
    main()
```
````

````{tab} Golang
```go
package main

import (
    "testing"
    "encoding/hex"
    "bytes"
    "fmt"

    "xdpiface" // This may vary
)

func main() {
    batch_size := uint32(30)
    var frames_rcvd uint32 = 0

    var oBufferSize int = 1000
    oBuffer := bytes.Repeat([]byte{0x55}, oBufferSize)

    var iBufferSize int = 0
    iBuffer := make([]byte, 9000)

    xdp_iface, err := XdpIfaceNew("lo")
    if err != nil {
        t.Errorf("Failed to create XDP iface")
    }
    defer xdp_iface.Destroy()

    xdp_iface.LoadProgram("./xdp_sock_bpf.o")

    xdp_sock, err := XdpSockNew(xdp_iface)
    if err != nil {
        t.Errorf("Failed to create XDP sock")
    }
    defer xdp_sock.Destroy()
    xdp_sock.LoopUpBpfMap(xdp_iface, "xsks_map", 4, 4)

    xdp_sock.SetSockopt(SO_PREFER_BUSY_POLL, 1)
    xdp_sock.SetSockopt(SO_BUSY_POLL, 20)
    xdp_sock.SetSockopt(SO_BUSY_POLL_BUDGET, int(batch_size))

    xdp_sock.TxBatchSetSize(batch_size);
    for i := 1; i <= int(batch_size); i++ {
        xdp_sock.Send (oBuffer, oBufferSize)
    }
    xdp_sock.TxBatchRelease(batch_size);

    xdp_sock.RxBatchGetSize (&frames_rcvd, batch_size);
    for i := 1; i <= int(frames_rcvd); i ++ {
        xdp_sock.Recv (iBuffer, &iBufferSize)
        fmt.Println(hex.Dump(iBuffer[:iBufferSize]))
    }
    xdp_sock.RxBatchRelease(frames_rcvd)

    xdp_iface.UnloadProgram()
}
```
````
