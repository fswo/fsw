#include "fsw/fsw.h"
#include "fsw/websocket_frame.h"
#include "gtest/gtest.h"
#include <iostream>

using fsw::websocket::Frame;
using fsw::websocket::FrameHeader;

static int ret[4];
static int ii = 0;


TEST(websocket_frame, header_size)
{
    uint16_t size = sizeof(FrameHeader);
    ASSERT_EQ(size, 2);
}
