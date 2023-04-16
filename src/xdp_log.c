/******************************** INCLUDE FILES *******************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "xdp_log.h"

/*********************************** DEFINES **********************************/
#define XDP_LOG_BUFFER_SIZE         4096

#define RED()    (fprintf(stdout, "\033[1;31m"))
#define GREEN() (fprintf(stdout, "\033[0;32m"))
#define YELLOW() (fprintf(stdout, "\033[1;33m"))
#define BLUE() (fprintf(stdout, "\033[0;34m"))
#define PURPLE() (fprintf(stdout, "\033[0;35m"))
#define CYAN() (fprintf(stdout, "\033[0;36m"))
#define WHITE() (fprintf(stdout, "\033[0;37m"))
#define RESET()  (fprintf(stdout, "\033[0m"))

/******************************* DATA *****************************************/
static xdp_log_lvl_t current_log_level = XDP_LOG_INFO;

static const char *xdp_log_lvl_str[XDP_LOG_LVLS] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL"
};
/***************************** LOCAL FUNCTIONS ********************************/
static
void xdp_log_col_line(xdp_log_lvl_t level)
{
    switch(level)
    {
        /* Fall through */
        case XDP_LOG_TRACE:
        case XDP_LOG_DEBUG:
            WHITE();
            break;

        case XDP_LOG_INFO:
            WHITE();
            break;

        case XDP_LOG_WARNING:
            YELLOW();
            break;

        case XDP_LOG_ERROR:
        case XDP_LOG_CRITICAL:
            RED();
            break;

        default:
            RESET();
            break;
    }

    return;
}

/************************* INTERFACE FUNCTIONS ********************************/
void xdp_log_level_set(xdp_log_lvl_t level)
{
    current_log_level = level;
}

void xdp_log_msg(const char *module, int line, xdp_log_lvl_t level, const char *format, ...)
{
    if (level < current_log_level)
        return;

    char timestamp[100];

    char buffer[XDP_LOG_BUFFER_SIZE];
    ssize_t buffer_len;

    int size1 = 0;
    int size = 0;
    time_t now = 0;

    now = time(0);
    strftime(timestamp, 100, "%d/%m/%Y %H:%M.%S", localtime(&now));

    // Write first part of log message
    memset(buffer, 0x00, XDP_LOG_BUFFER_SIZE);
    size1 = sprintf(buffer, " [%s] %s ", timestamp, xdp_log_lvl_str[level]);
    size = size1 + sprintf(buffer + size1, "(%d) %s: ", line, module);

    va_list argp;
    va_start(argp, format);
    size += vsnprintf(buffer + size, XDP_LOG_BUFFER_SIZE - size, format, argp);
    va_end(argp);

    xdp_log_col_line(level);
    /* print to stdout */
    fprintf(stdout, "%s\n", buffer);
    RESET();
}
