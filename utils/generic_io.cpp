#include "generic_io.h"
#include "logger.h"

//------------------------------------------------------------------------------------------------------------
GenericIO::Buffer GenericIO::Read(size_t a_block_size) {
    Buffer buffer;

    buffer.reserve(a_block_size);

    for (size_t recvd_size = 0;;) {
        buffer.resize(buffer.size() + a_block_size);

        const size_t size = Read(&buffer[recvd_size], sizeof(a_block_size));
        recvd_size += size;

        if (size != sizeof(a_block_size)) {
            buffer.resize(buffer.size() - (a_block_size - size));
            break;
        }
    }

    DBGLOG(buffer.size() << " bytes accumulated: " << std::string(buffer.begin(), buffer.end()));
    return buffer;
}
