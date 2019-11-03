#include "fsw/buffer.h"
#include "fsw/fsw.h"
#include "gtest/gtest.h"

using namespace std;
using fsw::Buffer;

TEST(buffer, append)
{
    char src_buffer[] = "aa";
    Buffer *buffer1 = new Buffer(1024);
    Buffer *buffer2 = new Buffer(1024);

    // function append test
    buffer1->append(src_buffer, strlen(src_buffer));
    buffer1->append(src_buffer, strlen(src_buffer));
    ASSERT_EQ(strcmp(buffer1->c_buffer(), "aaaa"), 0);
    ASSERT_EQ(buffer1->length(), 4);

    // function append test
    buffer2->append(buffer1);
    buffer2->append(buffer1);
    ASSERT_EQ(strcmp(buffer2->c_buffer(), "aaaaaaaa"), 0);
    ASSERT_EQ(buffer2->length(), 8);

    Buffer *buffer3 = new Buffer(1024);
    buffer3->append(src_buffer, strlen(src_buffer));

    // function append test
    Buffer *buffer4 = new Buffer(19);
    std::string s1("This is test string");
    buffer4->append(s1);
    ASSERT_EQ(buffer4->length(), 19);
    ASSERT_EQ(strcmp(buffer4->c_buffer(), s1.c_str()), 0);

    // clear function test
    Buffer *buffer5 = new Buffer(1024);
    char test_buffer[] = "The really interesting part of the show is yet to come";
    buffer5->append(test_buffer, strlen(test_buffer));
    buffer5->clear();

    // Determines whether the length is 0
    ASSERT_EQ(buffer5->length(), 0);
    // Determines whether the c_buffer is empty
    ASSERT_EQ(strcmp(buffer5->c_buffer(), ""), 0);

    // call append() function again
    char src_str[] = "abc";
    buffer5->append(src_str, strlen(src_str));
    ASSERT_EQ(buffer5->length(), 3);
    ASSERT_EQ(strcmp(buffer5->c_buffer(), "abc"), 0);
}

TEST(buffer, dup)
{
    char src_buffer[] = "aa";
    Buffer *buffer1 = new Buffer(1024);
    Buffer *buffer2;

    // function append test
    buffer1->append(src_buffer, strlen(src_buffer));
    buffer2 = buffer1->dup();
    ASSERT_EQ(buffer2->length(), 2);
    ASSERT_EQ(strcmp(buffer2->c_buffer(), buffer1->c_buffer()), 0);
    ASSERT_NE(buffer2->c_buffer(), buffer1->c_buffer());
}