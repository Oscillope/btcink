#ifndef CHUNKED_RESPONSE_H
#define CHUNKED_RESPONSE_H
#include <stddef.h>

class ChunkedResponse {
public:
    ChunkedResponse(void);
    ~ChunkedResponse(void);

    void write_chunk(void* chunk, size_t len);
    size_t get_len(void) { return buf_len; }
    char* get_data(void) { return buf; }
    void clear(void);

private:
    char* buf;
    size_t buf_len;
    size_t write_ptr;
};

#endif //CHUNKED_RESPONSE_H
