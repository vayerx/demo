#include "file_handle.h"
#include "logger.h"
#include "exception.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>


//------------------------------------------------------------------------------------------------------------
FileIO::FileIO(int a_handle)
    : m_handle(a_handle)
{
}

//----------------------------------------------------------------------------------------------------------------------
size_t FileIO::Read(void *a_buffer, size_t a_buffer_size) {
    size_t read_size = 0;
    char *buffer = static_cast<char *>(a_buffer);

    while (read_size < a_buffer_size) {
        // read data until EAGAIN (EWOULDBLOCK) or a_buffer_size
        const ssize_t res = ::read(m_handle, buffer + read_size, a_buffer_size - read_size);
        if (res == -1) {
            if (errno != EAGAIN) {
                throw TSystemError("read failed");
            }

            break;
        }

        // Check if the connection has been closed by the client
        if (res == 0) {
            if (read_size == 0) {
                throw std::runtime_error("no data to read (connection has been closed?)");
            } else {
                // some data has been read
                break;
            }
        }

        read_size += res;
    }

    return read_size;
}

//------------------------------------------------------------------------------------------------------------
void FileIO::Write(const void *a_buffer, size_t a_size) {
    const ssize_t res = ::write(m_handle, a_buffer, a_size);
    if (res == -1) {
        throw TSystemError("write failed");
    }
}


//------------------------------------------------------------------------------------------------------------
FileIO &GetStdoutIO() {
    static FileIO io(STDOUT_FILENO);
    return io;
}

//------------------------------------------------------------------------------------------------------------
FileIO &GetStderrIO() {
    static FileIO io(STDERR_FILENO);
    return io;
}


//------------------------------------------------------------------------------------------------------------
FileDescriptor::~FileDescriptor() {
    if (m_handle != -1 && ::close(m_handle) == -1) {
        LOG_ERR("FileDescriptor: can't close socket: " << ::strerror(errno));
    }
}
