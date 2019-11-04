#ifndef BUFFER_H
#define BUFFER_H

#include "fsw.h"

namespace fsw
{
/**
 * Binary secure buffer
 */
class Buffer
{
public:
    Buffer(size_t size);
    ~Buffer();
    size_t length();
    size_t size();
    char* c_buffer();
    void append(char *str, size_t length);
    void append(std::string str);
    void append(int value);
    void append(Buffer *buffer);
    void clear();
    Buffer* dup();
    bool equal(Buffer *target);
    bool deep_equal(Buffer *target);

private:
    size_t _size;
    size_t _length;
    char *_buffer;
};
}

#endif	/* BUFFER_H */