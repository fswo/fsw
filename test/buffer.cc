#include "fsw/buffer.h"
#include "fsw/fsw.h"
#include "gtest/gtest.h"

using namespace std;
using fsw::Buffer;

TEST(buffer, construct)
{
    Buffer buffer(16);
    ASSERT_EQ(buffer.size(), 16);
    ASSERT_EQ(buffer.length(), 0);
    ASSERT_NE(buffer.c_buffer(), nullptr);
}

TEST(buffer, length)
{
    char src_buffer[] = "fsw";
    Buffer buffer(16);
    buffer.append(src_buffer, sizeof(src_buffer) - 1);
    ASSERT_EQ(buffer.length(), 3);
}

TEST(buffer, size)
{
    char src_buffer[] = "fsw";
    Buffer buffer(16);
    buffer.append(src_buffer, sizeof(src_buffer) - 1);
    ASSERT_EQ(buffer.size(), 16);
}

TEST(buffer, c_buffer)
{
    char src_buffer[] = "fsw";
    Buffer buffer(16);
    buffer.append(src_buffer, sizeof(src_buffer) - 1);
    ASSERT_EQ(strcmp(buffer.c_buffer(), "fsw"), 0);
}

TEST(buffer, clear)
{
    char src_buffer[] = "fsw";
    Buffer buffer(16);
    buffer.append(src_buffer, sizeof(src_buffer) - 1);
    buffer.clear();
    // Determines whether the length is 0
    ASSERT_EQ(buffer.length(), 0);
    // Determines whether the c_buffer is empty
    ASSERT_EQ(strcmp(buffer.c_buffer(), ""), 0);
}

TEST(buffer, append_c_string)
{
    char src_buffer[] = "fsw";
    Buffer buffer(16);
    buffer.append(src_buffer, sizeof(src_buffer) - 1);
    ASSERT_EQ(strcmp(buffer.c_buffer(), src_buffer), 0);
    ASSERT_EQ(buffer.length(), sizeof(src_buffer) - 1);
}

TEST(buffer, append_cpp_string)
{
    std::string str = "fsw";
    Buffer buffer(16);
    buffer.append(str);
    ASSERT_EQ(strcmp(buffer.c_buffer(), str.c_str()), 0);
    ASSERT_EQ(buffer.length(), str.length());
}

TEST(buffer, append_int)
{
    char str1[] = "1";
    char str2[] = "12";
    Buffer buffer(16);

    buffer.append(1);
    ASSERT_EQ(buffer.length(), 1);
    ASSERT_EQ(strcmp(buffer.c_buffer(), str1), 0);

    buffer.append(2);
    ASSERT_EQ(buffer.length(), 2);
    ASSERT_EQ(strcmp(buffer.c_buffer(), str2), 0);
}

TEST(buffer, append_buffer)
{
    std::string str = "fsw";
    Buffer buffer1(16);
    Buffer buffer2(16);

    buffer1.append(str);
    buffer2.append(&buffer1);
    ASSERT_EQ(strcmp(buffer2.c_buffer(), "fsw"), 0);
    ASSERT_EQ(buffer2.length(), str.length());
}

TEST(buffer, dup)
{
    char src_buffer[] = "aa";
    Buffer *buffer1 = new Buffer(1024);
    Buffer *buffer2;

    buffer1->append(src_buffer, strlen(src_buffer));
    buffer2 = buffer1->dup();
    ASSERT_EQ(buffer2->length(), 2);
    ASSERT_EQ(strcmp(buffer2->c_buffer(), buffer1->c_buffer()), 0);
    ASSERT_NE(buffer2->c_buffer(), buffer1->c_buffer());
}

TEST(buffer, equal)
{
    char src_buffer1[] = "aa";
    char src_buffer2[] = "bb";
    Buffer buffer1(16);
    Buffer buffer2(16);
    Buffer buffer3(16);

    buffer1.append(src_buffer1, sizeof(src_buffer1) - 1);
    buffer2.append(src_buffer1, sizeof(src_buffer1) - 1);
    ASSERT_TRUE(buffer1.equal(&buffer2));
    buffer3.append(src_buffer2, sizeof(src_buffer2) - 1);
    ASSERT_FALSE(buffer1.equal(&buffer3));
}

TEST(buffer, deep_equal)
{
    char src_buffer1[] = "aa";
    char src_buffer2[] = "bb";
    Buffer buffer1(16);
    Buffer buffer2(16);
    Buffer buffer3(32);

    buffer1.append(src_buffer1, sizeof(src_buffer1) - 1);
    buffer2.append(src_buffer1, sizeof(src_buffer1) - 1);

    ASSERT_TRUE(buffer1.deep_equal(&buffer1));
    ASSERT_FALSE(buffer1.deep_equal(&buffer2));
    ASSERT_FALSE(buffer1.deep_equal(&buffer3));
}