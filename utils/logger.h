#pragma once

#include <iostream>

/// @todo synchronized logging
#define LOG_ERR(X)          (std::cerr << X << std::endl)

#ifdef _DEBUG
#   define DBG(X)           (X)
#   define DBGLOG(X)        (std::cerr << X << std::endl)
#else
#   define DBG(X)
#   define DBGLOG(X)
#endif
