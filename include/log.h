#pragma once

#include "fsw.h"
#include "config.h"

#define FSW_OK 0
#define FSW_ERR -1

#define FSW_LOG_BUFFER_SIZE 1024
#define FSW_LOG_DATE_STRLEN  64

#define FSW_DEBUG_MSG_SIZE 512
#define FSW_TRACE_MSG_SIZE 512
#define FSW_WARN_MSG_SIZE 512
#define FSW_ERROR_MSG_SIZE 512

extern char fsw_debug[FSW_DEBUG_MSG_SIZE];
extern char fsw_trace[FSW_TRACE_MSG_SIZE];
extern char fsw_warn[FSW_WARN_MSG_SIZE];
extern char fsw_error[FSW_ERROR_MSG_SIZE];

#if FSW_DEBUG
#define fswDebug(str, ...)                                                         \
    snprintf(fsw_debug, FSW_DEBUG_MSG_SIZE, "%s: " str " in %s on line %d.", __func__, ##__VA_ARGS__, __FILE__, __LINE__); \
    fswLog_put(FSW_LOG_DEBUG, fsw_debug);
#else
#define fswDebug(str,...)
#endif

#if FSW_TRACE
#define fswTrace(str, ...)                                                         \
    snprintf(fsw_trace, FSW_TRACE_MSG_SIZE, "%s: " str " in %s on line %d.", __func__, ##__VA_ARGS__, __FILE__, __LINE__); \
    fswLog_put(FSW_LOG_TRACE, fsw_trace);
#else
#define fswTrace(str,...)
#endif

#define fswWarn(str, ...)                                                         \
    snprintf(fsw_warn, FSW_WARN_MSG_SIZE, "%s: " str " in %s on line %d.", __func__, ##__VA_ARGS__, __FILE__, __LINE__); \
    fswLog_put(FSW_LOG_WARNING, fsw_warn);

#define fswError(str, ...)                                                         \
    snprintf(fsw_error, FSW_ERROR_MSG_SIZE, "%s: " str " in %s on line %d.", __func__, ##__VA_ARGS__, __FILE__, __LINE__); \
    fswLog_put(FSW_LOG_ERROR, fsw_error); \
    exit(-1);

enum fswLog_level
{
    FSW_LOG_DEBUG = 0,
    FSW_LOG_TRACE,
    FSW_LOG_INFO,
    FSW_LOG_NOTICE,
    FSW_LOG_WARNING,
    FSW_LOG_ERROR,
};

void fswLog_put(int level, char *cnt);
