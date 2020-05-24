
#include <cstring>
#include <iostream>
#include <memory>

#include <gtest/gtest.h>

#include "sbec.h"

TEST(StringView, get_set_testing) {
    char buffer[128];
    sbe_string_view view;
    view.data = buffer;
    view.length = sizeof(buffer);
    memset(buffer, 0, sizeof(buffer));
    EXPECT_EQ(sbe_string_view_set(view, NULL, 129), false);
    EXPECT_EQ(sbe_string_view_set_str(view, "Hello, the world"), true);
    EXPECT_EQ(strcmp(view.data,  "Hello, the world"), 0);
}

TEST(ByteOrder, encoded_data_view_set_get) {
    int16_t buffer[128];
    int16_t buffer_le[128];
    sbe_int16_view_be view;
    sbe_int16_view_le view_le;
    int16_t val = 1339;
    int16_t valOut;
    int16_t valDirectOut;

    view.data = buffer;
    view.length = sizeof(buffer) / sizeof(buffer[0]);
    EXPECT_EQ(sbe_int16_view_be_set(view, NULL, 129), false);
    sbe_int16_view_be_set(view, &val, 1);
    sbe_int16_view_be_get(view, &valOut, 1);
    EXPECT_EQ(val, valOut);
    valDirectOut = *(int16_t*)view.data;
    EXPECT_EQ(val == valDirectOut, __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__);
    EXPECT_EQ(valDirectOut, SBE_BIG_ENDIAN_ENCODE_16(val));


    view_le.data = buffer_le;
    view_le.length = sizeof(buffer_le) / sizeof(buffer_le[0]);
    sbe_int16_view_le_set(view_le, &val, 1);
    EXPECT_EQ(sbe_int16_view_le_set(view_le, NULL, 129), false);
    valOut = 0;
    sbe_int16_view_le_get(view_le, &valOut, 1);
    EXPECT_EQ(val, valOut);
    valDirectOut = *(int16_t*)view_le.data;
    EXPECT_EQ(val == valDirectOut, __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);
    EXPECT_EQ(valDirectOut, SBE_LITTLE_ENDIAN_ENCODE_16(val));
}