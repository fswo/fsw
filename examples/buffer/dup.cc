#include "fsw/buffer.h"
#include "fsw/fsw.h"

using namespace std;
using fsw::Buffer;

int main(int argc, char const *argv[])
{
    char src_buffer[] = "aa";
    Buffer *buffer1 = new Buffer(1024);
    Buffer *buffer2;

    // function append test
    buffer1->append(src_buffer, strlen(src_buffer));
    buffer2 = buffer1->dup();
    assert(buffer2->length() == 2);
    assert(strcmp(buffer2->c_buffer(), buffer1->c_buffer()) == 0);
    assert(buffer2->c_buffer() != buffer1->c_buffer());

    return 0;
}