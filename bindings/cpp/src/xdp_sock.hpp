#ifndef __XDP_SOCK_CPP_H__
#define __XDP_SOCK_CPP_H__

#include "xdpiface.hpp"
#include <string>

class XdpSock {
    private:
        XdpIface *xdp_iface;

    public:
        XdpSock(XdpIface *interface);
        ~XdpSock();

    int lookupBpfMap(XdpIface *interface, const std::string, uint32_t, uint32_t);
    int setSockopt (int, int);
    int getFd ();
    int rxBatchGetSize(uint32_t *, uint32_t);
    int rxBatchRelease (uint32_t);
    int recv (char *, size_t *);
    int txBatchSetSize (uint32_t);
    int txBatchRelease (uint32_t);
    int send (char *, size_t);

    xdp_sock_t *self;
};

#endif /* __XDP_SOCK_CPP_H__ */