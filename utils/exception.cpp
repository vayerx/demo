#include "exception.h"
#include <errno.h>
#include <string.h>
#include <string>


TSystemError::TSystemError()
    : std::runtime_error(strerror(errno))
{
}

TSystemError::TSystemError(int code)
    : std::runtime_error(strerror(code))
{
}

TSystemError::TSystemError(const std::string& a_msg)
    : std::runtime_error(a_msg + ": " + strerror(errno))
{
}
