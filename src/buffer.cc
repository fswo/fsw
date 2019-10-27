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
    if (_length + str.length() > _size)
    {
        fswError("buffer capacity is not enough");
    }

    memcpy(_buffer + _length, str.c_str(), str.length());
    _length += str.length();
    _buffer[_length] = 0;
}

void Buffer::append(Buffer *buffer)
{
    if (_length + buffer->length() > _size)
    {
        fswError("buffer capacity is not enough");
    }

    memcpy(_buffer + _length, buffer->c_buffer(), buffer->length());
    _length += buffer->length();
    _buffer[_length] = 0;
}

void Buffer::clear()
{
    _length = 0;
    _buffer[0] = 0;
}