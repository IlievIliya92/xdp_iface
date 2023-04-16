#ifndef XDP_LOG_H_INCLUDED
#define XDP_LOG_H_INCLUDED
/******************************** INCLUDE FILES *******************************/
#include <stdarg.h>
#include <string.h>

/*********************************** DEFINES **********************************/
typedef enum _xdp_log_lvl_t
{
  XDP_LOG_TRACE,
  XDP_LOG_DEBUG,
  XDP_LOG_INFO,
  XDP_LOG_WARNING,
  XDP_LOG_ERROR,
  XDP_LOG_CRITICAL,
  XDP_LOG_LVLS
} xdp_log_lvl_t;

#define XDP_LOG_MSG(level, msg, ...) xdp_log_msg(XDP_MODULE_NAME, __LINE__, level, msg, ## __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************************* INTERFACE FUNCTION PROTOTYPES **********************/
void xdp_log_level_set(xdp_log_lvl_t level);
void xdp_log_msg(const char *module, int line, xdp_log_lvl_t level, const char *format_str, ...);


#ifdef __cplusplus
}
#endif /* _cplusplus */

#endif /* XDP_LOG_H_INCLUDED */

