#include "xdp_log.hpp"

//
void XdpLog::levelSet (int level)
{
    xdp_log_level_set(level);
}

