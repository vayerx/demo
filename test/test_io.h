#pragma once

#include <utils/generic_io.h>

#include <string>
#include <boost/signals2/signal.hpp>


namespace test {

//------------------------------------------------------------------------------------------------------------
class BufferIO
    : public GenericIO
{
public:
    typedef boost::signals2::signal<void (void *a_buffer, size_t a_size)> OnRead;
    typedef boost::signals2::signal<void (const void *a_buffer, size_t a_size)> OnWrite;

    size_t  Read(void *a_buffer, size_t a_size) override;
    void    Write(const void *a_buffer, size_t a_size) override;

    size_t  DoRead(void *a_buffer, size_t a_size);
    void    DoWrite(const void *a_buffer, size_t a_size);

    void SetInput(const std::string &a_data) { m_input = a_data; }
    void SetOutput(const std::string &a_data) { m_output = a_data; }

    std::string AquireInput() { return std::move(m_input); }
    std::string AquireOutput() { return std::move(m_output); }

    template <typename Handler>
    void RegisterOnRead(Handler a_handler) { m_on_read.connect(a_handler); }

    template <typename Handler>
    void RegisterOnWrite(Handler a_handler) { m_on_write.connect(a_handler); }

private:
    std::string m_input, m_output;
    OnRead  m_on_read;
    OnWrite m_on_write;
};

//------------------------------------------------------------------------------------------------------------
class ChunkIo
    : public BufferIO
{
public:
    size_t  Read(void *a_buffer, size_t a_size) override;
    void    Write(const void *a_buffer, size_t a_size) override;

    void SetReadChunkSize(size_t a_size) { m_read_chunk_size = a_size; }
    void SetWriteChunkSize(size_t a_size) { m_write_chunk_size = a_size; }

private:
    size_t m_read_chunk_size = 16;
    size_t m_write_chunk_size = 4096;
};

} // namespace test
