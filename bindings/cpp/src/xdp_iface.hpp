#ifndef __XDP_IFACE_CPP_H__
#define __XDP_IFACE_CPP_H__

#include "xdpiface.hpp"
#include <string>

class XdpIface {
    public:
        XdpIface(const std::string);
        ~XdpIface();

    int loadProgram(const std::string);
    void unloadProgram();
    const std::string getName();
    void *getProg();

    xdp_iface_t *self;
};

#endif /* __XDP_IFACE_CPP_H__ */