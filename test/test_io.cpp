#include "test_io.h"
#include <string.h>
#include <stdexcept>

using namespace test;


//------------------------------------------------------------------------------------------------------------
size_t BufferIO::Read(void *a_buffer, size_t a_size)
{
    return DoRead(a_buffer, a_size);
}

//------------------------------------------------------------------------------------------------------------
void BufferIO::Write(const void *a_buffer, size_t a_size)
{
    DoWrite(a_buffer, a_size);
}

//------------------------------------------------------------------------------------------------------------
size_t BufferIO::DoRead(void *a_buffer, size_t a_size)
{
    m_on_read(a_buffer, a_size);

    const size_t size = std::min(a_size, m_input.size());
    ::memcpy(a_buffer, &m_input[0], size);  /* Flawfinder: ignore */
    m_input.erase(0, size);

    return size;
}

//------------------------------------------------------------------------------------------------------------
void BufferIO::DoWrite(const void *a_buffer, size_t a_size)
{
    m_on_write(a_buffer, a_size);

    m_output.append(static_cast<const char *>(a_buffer), a_size);
}

//------------------------------------------------------------------------------------------------------------
size_t test::ChunkIo::Read(void *a_buffer, size_t a_size)
{
    return BufferIO::DoRead(a_buffer, std::min(a_size, m_read_chunk_size));
}

//------------------------------------------------------------------------------------------------------------
void test::ChunkIo::Write(const void *a_buffer, size_t a_size)
{
    const char *buffer = static_cast<const char *>(a_buffer);
    for (size_t size = 0; size < a_size; ) {
        const size_t chunk = std::min(a_size, m_write_chunk_size);
        BufferIO::DoWrite(buffer, chunk);
        buffer += chunk;
        size += chunk;
    }
}
