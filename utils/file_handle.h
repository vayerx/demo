#pragma once

#include "generic_io.h"

//------------------------------------------------------------------------------------------------------------
// Doesn't take ownership
class FileIO
    : public GenericIO
{
public:
    explicit FileIO(int a_handle);

    size_t  Read(void *a_buffer, size_t a_size) override;
    void    Write(const void *a_buffer, size_t a_size) override;

    int GetHandle() const { return m_handle; }
        operator int() const { return m_handle; }

protected:
    int m_handle;
};

FileIO &GetStdoutIO();
FileIO &GetStderrIO();

//------------------------------------------------------------------------------------------------------------
// Takes ownership of file descriptor
class FileDescriptor
    : public FileIO
{
public:
    using FileIO::FileIO;
    ~FileDescriptor();

    // non-copyable
    FileDescriptor(FileDescriptor &) = delete;
    FileDescriptor &operator=(FileDescriptor &) = delete;
};

