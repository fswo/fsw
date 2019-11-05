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
    void append(char *str, size_t length);
    void append(std::string str);
    void append(int value);
    void append(Buffer *buffer);
    Buffer* dup();
    bool equal(Buffer *target);
    bool deep_equal(Buffer *target);

    inline size_t length()
    {
        return _length;
    }

    inline size_t size()
    {
        return _size;
    }

    inline char* c_buffer()
    {
        return _buffer;
    }

    inline void clear()
    {
        _length = 0;
        _buffer[0] = 0;
    }

private:
    /**
     * size represents a valid character that can be stored, excluding the last \0
     */
    size_t _size;
    size_t _length;
    char *_buffer;
};
}

#endif	/* BUFFER_H */