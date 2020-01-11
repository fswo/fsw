#pragma once

#include "fsw.h"

enum fswErrorCode
{
    /**
     * connection error
     */
    FSW_ERROR_SESSION_CLOSED_BY_SERVER = 1001,
    FSW_ERROR_SESSION_CLOSED_BY_CLIENT,
};

const char* fsw_strerror(int code);
