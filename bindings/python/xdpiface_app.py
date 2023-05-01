import select
import signal
import time
import threading

import xdpiface

from ctypes import c_int, create_string_buffer, c_size_t

STOP = False
def sig_handler(sig, frame):
    global STOP
    del sig, frame

    STOP = True

def send_frames(xdp_sock):
    global STOP
    frame_size = 1500
    batch_size = 30
    batches_to_send = 100
    batches_to_sent = 0
    farmes_sent = 0

    o_buffer = create_string_buffer(frame_size)
    o_buffer.raw = bytearray([0x55] * frame_size)
    o_buffer_size = c_size_t(frame_size)

    while True:
        if STOP:
            break

        if batches_to_send == batches_to_sent:
            break

        xdp_sock.tx_batch_set_size(batch_size);
        for i in range(batch_size):
            xdp_sock.send (o_buffer, o_buffer_size);
        xdp_sock.tx_batch_release(batch_size);
        farmes_sent += batch_size
        batches_to_sent += 1

        time.sleep(0.01)

    print(f"--- Batches sent: {batches_to_sent}, frames sent: {farmes_sent}")
    STOP = True

def main():
    batch_size = 30

    xdp_iface = xdpiface.XdpIface(xdpiface.XdpIface.DEFAULT.encode())
    xdp_iface.load_program(xdpiface.XdpIface.XDP_PROG_DEFAULT.encode())

    xdp_sock = xdpiface.XdpSock(xdp_iface)
    xdp_sock.lookup_bpf_map(xdp_iface, xdpiface.XdpSock.XSKS_MAP_DEFAULT.encode(), 4, 4)

    xdp_sock.set_sockopt(xdpiface.XdpSock.SO_PREFER_BUSY_POLL, 1)
    xdp_sock.set_sockopt(xdpiface.XdpSock.SO_BUSY_POLL, 20)
    xdp_sock.set_sockopt(xdpiface.XdpSock.SO_BUSY_POLL_BUDGET, batch_size)

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    xdp_sock_fd = xdp_sock.get_fd()
    poller = select.poll()
    poller.register(xdp_sock_fd, select.POLLIN)

    frames_recd = c_int(0)
    i_buffer = create_string_buffer(9000)
    i_buffer_size = c_size_t(0)

    thread = threading.Thread(target=send_frames, args=(xdp_sock, ));
    thread.start()

    frames_received = 0
    while True:
        if STOP:
            break

        evts = poller.poll(1000)
        for sock, evt in evts:
            if evt and select.POLLIN:
                if sock == xdp_sock_fd:
                    ret = xdp_sock.rx_batch_get_size (frames_recd, batch_size);
                    if ret:
                        continue

                    frames_received += frames_recd.value
                    for i in range(frames_recd.value):
                        xdp_sock.recv (i_buffer, i_buffer_size)
                        print(f"--- Frame length: {i_buffer_size.value}, frames received: {frames_received}")
                        # print(hexdump(i_buffer.raw[:i_buffer_size.value]))
                    xdp_sock.rx_batch_release(frames_recd.value);

    thread.join()
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
