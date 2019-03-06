#ifndef _EGSIP_UTIL_H_
#define _EGSIP_UTIL_H_

#include <stdarg.h>
#include "egsip_def.h"

#ifndef NULL
#define NULL ((void *)0)
#endif


// 调试信息输出辅助接口 ----------------------------------------------------

enum EGSIP_LOG_LEVEL //调试信息打印级别
{
    EGSIP_LOG_ERROR  = 0,
    EGSIP_LOG_INFO   = 1,
    EGSIP_LOG_DEBUG  = 2,
};
extern int egsip_log_level;
int egsip_log_print(int level, const char * format, ...);
#define egsip_log_debug(fmt, args...)  egsip_log_print(EGSIP_LOG_DEBUG,"[EGSIP %s:%d] "fmt,__func__, __LINE__, ##args)
#define egsip_log_info(fmt, args...)   egsip_log_print(EGSIP_LOG_INFO, "[EGSIP %s:%d] "fmt,__func__, __LINE__, ##args)
#define egsip_log_error(fmt, args...)  egsip_log_print(EGSIP_LOG_ERROR,"[EGSIP %s:%d] "fmt,__func__, __LINE__, ##args)

// -------------------------------------------------------------------------



// 字符串辅助接口 ----------------------------------------------------

// 类似C snprintf函数
int egsip_snprintf(char *buf, unsigned int size, const char *fmt, ...);

//类似C vsnprintf函数
int egsip_vsnprintf(char *buf, unsigned int size, const char *fmt, va_list args);

//类似C vsnprintf函数
int egsip_sprintf(char *buf, const char *fmt, ...);

// -------------------------------------------------------------------------



#endif

