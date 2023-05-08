/******************************** INCLUDE FILES *******************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

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

/****************************** TYPEDEFS **************************************/
typedef struct _xdp_log_priv_t {
    int log_lvl;
} xdp_log_priv_t;
#define XDP_LOG_INIT_DFLT {XDP_LOG_INFO}

/******************************* DATA *****************************************/
static xdp_log_priv_t _xdp_log_priv = XDP_LOG_INIT_DFLT;

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
void xdp_log_col_line(int level)
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

static void
xdp_log_hexdump_format(void *buffer, size_t buffer_len)
{
    const unsigned char *address = (unsigned char *)buffer;
    const unsigned char *line = address;
    size_t line_size = 32;
    unsigned char c;
    char buf[32];
    int i = 0;

    fprintf(stdout, " Buffer length = %zu\n", buffer_len);
    fprintf(stdout, "%s | ", buf);
    while (buffer_len-- > 0) {
        fprintf(stdout, "%02X ", *address++);
        if (!(++i % line_size) || (buffer_len == 0 && i % line_size)) {
            if (buffer_len == 0) {
                while (i++ % line_size)
                    fprintf(stdout, "__ ");
            }
            fprintf(stdout, " | ");  /* right close */
            while (line < address) {
                c = *line++;
                fprintf(stdout, "%c", (c < 33 || c == 255) ? 0x2E : c);
            }
            fprintf(stdout, "\n");
            if (buffer_len > 0)
                fprintf(stdout, "%s | ", buf);
        }
    }
    fprintf(stdout, "\n");
}

/************************* INTERFACE FUNCTIONS ********************************/
void
xdp_log_level_set(int level)
{
    _xdp_log_priv.log_lvl = level;
}

void
xdp_log_msg(const char *module, int line, int level, const char *format, ...)
{
    if (level < _xdp_log_priv.log_lvl)
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

void
xdp_log_hexdump (const char *module, int line, int level, const char *description,
    void *buffer, size_t buffer_len)
{
    if (level < _xdp_log_priv.log_lvl)
        return;

    xdp_log_msg(module, line, level, description);
    xdp_log_hexdump_format(buffer, buffer_len);
}

void
xdp_log_test (bool verbose)
{
    int lvl = XDP_LOG_TRACE;
    char buff[100];
    memset(buff, 0xAA, 100);

    xdp_log_level_set(XDP_LOG_TRACE);

    for (lvl = XDP_LOG_TRACE; lvl < XDP_LOG_LVLS; lvl++)
        xdp_log_msg("LOG_MODULE", __LINE__, lvl, "test for: %s", xdp_log_lvl_str[lvl]);

    xdp_log_hexdump("LOG_MODULE", __LINE__, XDP_LOG_INFO, "test hexdump", buff, 100);
}
