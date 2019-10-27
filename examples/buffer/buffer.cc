#include "fsw/buffer.h"
#include "fsw/fsw.h"

using namespace std;
using fsw::Buffer;

int main(int argc, char const *argv[])
{
    char src_buffer[] = "aa";
    Buffer *buffer1 = new Buffer(1024);
    Buffer *buffer2 = new Buffer(1024);

    // function append test
    buffer1->append(src_buffer, strlen(src_buffer));
    buffer1->append(src_buffer, strlen(src_buffer));
    assert(strcmp(buffer1->c_buffer(), "aaaa") == 0);
    assert(buffer1->length() == 4);

    // function append test
    buffer2->append(buffer1);
    buffer2->append(buffer1);
    assert(strcmp(buffer2->c_buffer(), "aaaaaaaa") == 0);
    assert(buffer2->length() == 8);

    Buffer *buffer3 = new Buffer(1024);
    buffer3->append(src_buffer, strlen(src_buffer));

    // function append test
    Buffer *buffer4 = new Buffer(19);
    std::string s1("This is test string");
    buffer4->append(s1);
    assert(buffer4->length() == 19);
    assert(strcmp(buffer4->c_buffer(), s1.c_str()) == 0);

    // clear function test
    Buffer *buffer5 = new Buffer(1024);
    char test_buffer[] = "The really interesting part of the show is yet to come";
    buffer5->append(test_buffer, strlen(test_buffer));
    buffer5->clear();

    // Determines whether the length is 0
    assert(buffer5->length() == 0);
    // Determines whether the c_buffer is empty
    assert(strcmp(buffer5->c_buffer(), "") == 0);

    // call append() function again
    char src_str[] = "abc";
    buffer5->append(src_str, strlen(src_str));
    assert(buffer5->length() == 3);
    assert(strcmp(buffer5->c_buffer(), "abc") == 0);

    return 0;
}