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

Buffer* Buffer::append(char *str, size_t length)
{
    if (_length + length > _size)
    {
        fswError("buffer capacity is not enough");
    }

    memcpy(_buffer + _length, str, length);
    _length += length;
    _buffer[_length] = 0;
    return this;
}

Buffer* Buffer::append(std::string str)
{
    return append((char *)str.c_str(), str.length());
}

Buffer* Buffer::append(int value)
{
    auto str = std::to_string(value);
    return append(str);
}

Buffer* Buffer::append(Buffer *buffer)
{
    return append(buffer->c_buffer(), buffer->length());
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

bool Buffer::equal(std::string target)
{
    if (_length != target.length() || memcmp(_buffer, target.c_str(), _length) != 0)
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