#pragma once

#include <string>

//--------------------------------------------------------------------------------------------------------
class GenericIO
{
public:
    typedef std::string   Buffer;

    virtual ~GenericIO() = default;

    Buffer  Read(size_t a_block_size = 4096);

    virtual size_t  Read(void *a_buffer, size_t a_size) = 0;
    virtual void    Write(const void *a_buffer, size_t a_size) = 0;
};
