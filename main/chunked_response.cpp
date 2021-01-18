#include <stdlib.h>
#include <string.h>
#include "chunked_response.h"

#define CHUNK_BUF_STARTING_SIZE 1024

ChunkedResponse::ChunkedResponse()
    : buf(NULL)
    , buf_len(CHUNK_BUF_STARTING_SIZE)
    , write_ptr(0)
{
    buf = (char*)malloc(buf_len);
    clear();
}

ChunkedResponse::~ChunkedResponse()
{
    free(buf);
}

void ChunkedResponse::write_chunk(void* chunk, size_t len)
{
    if (write_ptr + len > buf_len) {
        buf_len += len;
        buf = (char*)realloc((void*)buf, buf_len);
    }
    memcpy(buf + write_ptr, chunk, len);
    write_ptr += len;
}

void ChunkedResponse::clear()
{
    write_ptr = 0;
    memset(buf, 0x00, buf_len);
}
