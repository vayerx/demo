#pragma once

#include <stdexcept>


class TSystemError : public std::runtime_error {
public:
    TSystemError();
    explicit TSystemError(int code);
    explicit TSystemError(const std::string &a_msg);
};
