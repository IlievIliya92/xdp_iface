#include "xdp_iface.hpp"

XdpIface::XdpIface(const std::string interface) {
    self = xdp_iface_new (interface.c_str());
}

XdpIface::~XdpIface() {
    xdp_iface_destroy (&self);
}

int XdpIface::loadProgram(const std::string xdpProgPath) {
    int rv = 0;
    rv = xdp_iface_load_program (self, xdpProgPath.c_str());

    return rv;
}

void XdpIface::unloadProgram() {
    xdp_iface_unload_program (self);
}

const std::string XdpIface::getName() {
    return (const std::string) xdp_iface_get_name (self);
}

void *XdpIface::getProg() {
    return (void *)xdp_iface_get_prog (self);
}
