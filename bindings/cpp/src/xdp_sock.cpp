#include "xdp_sock.hpp"

XdpSock::XdpSock(XdpIface *interface) {
    self = xdp_sock_new (interface->self);
    xdp_iface = interface;
}

XdpSock::~XdpSock() {
    xdp_sock_destroy (&self);
}

int XdpSock::lookupBpfMap(XdpIface *interface, const std::string map_name,
    uint32_t key_size, uint32_t value_size) {
    return xdp_sock_lookup_bpf_map (self, xdp_iface->self, map_name.c_str(),
        key_size, value_size);
}

int XdpSock::setSockopt (int opt_type, int opt_value) {
    return xdp_sock_set_sockopt (self, opt_type, opt_value);
}

int XdpSock::getFd () {
    return xdp_sock_get_fd (self);
}

int XdpSock::rxBatchGetSize(uint32_t *frames_recvd, uint32_t nb) {
    return xdp_sock_rx_batch_get_size (self, frames_recvd, nb);
}

int XdpSock::rxBatchRelease (uint32_t frames_recvd) {
    return xdp_sock_rx_batch_release (self, frames_recvd);
}

int XdpSock::recv (char *buffer, size_t *buffer_size) {
    return xdp_sock_recv (self, buffer, buffer_size);
}

int XdpSock::txBatchSetSize (uint32_t nb) {
    return xdp_sock_tx_batch_set_size (self, nb);
}

int XdpSock::txBatchRelease (uint32_t frames_send) {
    return xdp_sock_tx_batch_release (self, frames_send);
}

int XdpSock::send (char *buffer, size_t buffer_size) {
    return xdp_sock_send (self, buffer, buffer_size);
}
