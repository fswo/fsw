#include "buffer.h"
#include "log.h"

using fsw::Buffer;

Buffer::Buffer(size_t size):
    _size(size)
{
    _length = 0;
    _buffer = new char[_size + 1]();
}

Buffer::~Buffer()
{
    delete[] _buffer;
}

size_t Buffer::length()
{
    return _length;
}

/**
 * size represents a valid character that can be stored, excluding the last \0
 */
size_t Buffer::size()
{
    return _size;
}

char* Buffer::c_buffer()
{
    return _buffer;
}

void Buffer::append(char *str, size_t length)
{
    if (_length + length > _size)
    {
        fswError("buffer capacity is not enough");
    }

    memcpy(_buffer + _length, str, length);
    _length += length;
    _buffer[_length] = 0;
}

void Buffer::append(std::string str)
{
    append((char *)str.c_str(), str.length());
}

void Buffer::append(int value)
{
    auto str = std::to_string(value);
    append(str);
}

void Buffer::append(Buffer *buffer)
{
    append(buffer->c_buffer(), buffer->length());
}

void Buffer::clear()
{
    _length = 0;
    _buffer[0] = 0;
}

Buffer* Buffer::dup()
{
    Buffer *ret_buffer = new Buffer(_length);
    ret_buffer->append(this);
    return ret_buffer;
}

bool Buffer::equal(Buffer *target)
{
    if (_length != target->length() || memcmp(_buffer, target->c_buffer(), _length) != 0)
    {
        return false;
    }
    return true;
}

bool Buffer::deep_equal(Buffer *target)
{
    if (
        this != target
        || _size != target->size()
        || _length != target->length()
        || _buffer != target->c_buffer()
        || memcmp(_buffer, target->c_buffer(), _length) != 0
    )
    {
        return false;
    }
    return true;
}