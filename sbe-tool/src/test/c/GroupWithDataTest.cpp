/*
 * Copyright 2013-2020 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <group_with_data/group_with_data.h>

#define GWD(name) group_with_data_##name

static const std::uint32_t TAG_1 = 32;
static const std::uint64_t ENTRIES_COUNT = 2;

static const char TAG_GROUP_1_IDX_0[] = { 'T', 'a', 'g', 'G', 'r', 'o', 'u', 'p', '0' };
static const char TAG_GROUP_1_IDX_1[] = { 'T', 'a', 'g', 'G', 'r', 'o', 'u', 'p', '1' };
static const std::uint64_t TAG_GROUP_1_IDX_0_LENGTH = sizeof(TAG_GROUP_1_IDX_0);
static const std::uint64_t TAG_GROUP_1_IDX_1_LENGTH = sizeof(TAG_GROUP_1_IDX_1);

static const std::int64_t TAG_GROUP_2_IDX_0 = -120;
static const std::int64_t TAG_GROUP_2_IDX_1 = 120;
static const std::int64_t TAG_GROUP_2_IDX_2 = 75;

static const std::int64_t TAG_GROUP_2_IDX_3 = 76;
static const std::int64_t TAG_GROUP_2_IDX_4 = 77;
static const std::int64_t TAG_GROUP_2_IDX_5 = 78;

static const char *VAR_DATA_FIELD_IDX_0 = "neg idx 0";
static const std::uint64_t VAR_DATA_FIELD_IDX_0_LENGTH = 9;
static const char *VAR_DATA_FIELD_IDX_1 = "idx 1 positive";
static const std::uint64_t VAR_DATA_FIELD_IDX_1_LENGTH = 14;

static const std::uint64_t NESTED_ENTRIES_COUNT = 3;

static const char *VAR_DATA_FIELD_NESTED_IDX_0 = "zero";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_0_LENGTH = 4;
static const char *VAR_DATA_FIELD_NESTED_IDX_1 = "one";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_1_LENGTH = 3;
static const char *VAR_DATA_FIELD_NESTED_IDX_2 = "two";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_2_LENGTH = 3;

static const char *VAR_DATA_FIELD_NESTED_IDX_3 = "three";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_3_LENGTH = 5;
static const char *VAR_DATA_FIELD_NESTED_IDX_4 = "four";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_4_LENGTH = 4;
static const char *VAR_DATA_FIELD_NESTED_IDX_5 = "five";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_5_LENGTH = 4;

static const char *VAR_DATA_FIELD_1_IDX_0 = "neg idx 0";
static const std::uint64_t VAR_DATA_FIELD_1_IDX_0_LENGTH = 9;
static const char *VAR_DATA_FIELD_1_IDX_1 = "idx 1 positive";
static const std::uint64_t VAR_DATA_FIELD_1_IDX_1_LENGTH = 14;

static const char *VAR_DATA_FIELD_2_IDX_0 = "negative index 0";
static const std::uint64_t VAR_DATA_FIELD_2_IDX_0_LENGTH = 16;
static const char *VAR_DATA_FIELD_2_IDX_1 = "index 1 pos";
static const std::uint64_t VAR_DATA_FIELD_2_IDX_1_LENGTH = 11;

static const std::uint64_t expectedTestMessage1Size = 78;
static const std::uint64_t expectedTestMessage2Size = 107;
static const std::uint64_t expectedTestMessage3Size = 145;
static const std::uint64_t expectedTestMessage4Size = 73;

class GroupWithDataTest : public testing::Test
{
public:

    std::uint64_t encodeTestMessage1(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!GWD(TestMessage1_wrap_for_encode)(&m_msg1, buffer, offset, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(TestMessage1_Tag1_set)(&m_msg1, TAG_1);

        GWD(TestMessage1_Entries) entries;
        if (!GWD(TestMessage1_Entries_count_set)(&m_msg1, &entries, ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(TestMessage1_Entries_next)(&entries);
        GWD(TestMessage1_Entries_TagGroup1)(&entries).set_buffer(TAG_GROUP_1_IDX_0);
        GWD(TestMessage1_Entries_TagGroup2_set)(&entries, TAG_GROUP_2_IDX_0);

        GWD(TestMessage1_Entries_varDataField_set)(&entries, VAR_DATA_FIELD_IDX_0, VAR_DATA_FIELD_IDX_0_LENGTH);

        GWD(TestMessage1_Entries_next)(&entries);
        GWD(TestMessage1_Entries_TagGroup1)(&entries).set_buffer(TAG_GROUP_1_IDX_1);
        GWD(TestMessage1_Entries_TagGroup2_set)(&entries, TAG_GROUP_2_IDX_1);

        GWD(TestMessage1_Entries_varDataField_set)(&entries, VAR_DATA_FIELD_IDX_1, VAR_DATA_FIELD_IDX_1_LENGTH);


        return GWD(TestMessage1_encoded_length)(&m_msg1);
    }

    std::uint64_t encodeTestMessage2(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!GWD(TestMessage2_wrap_for_encode)(&m_msg2, buffer, offset, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(TestMessage2_Tag1_set)(&m_msg2, TAG_1);

        GWD(TestMessage2_Entries) entries;
        if (!GWD(TestMessage2_Entries_count_set)(&m_msg2, &entries, ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(TestMessage2_Entries_next)(&entries);
        GWD(TestMessage2_Entries_TagGroup1)(&entries).set_buffer(TAG_GROUP_1_IDX_0);
        GWD(TestMessage2_Entries_TagGroup2_set)(&entries, TAG_GROUP_2_IDX_0);

        GWD(TestMessage2_Entries_varDataField1_set)(&entries, VAR_DATA_FIELD_1_IDX_0, VAR_DATA_FIELD_1_IDX_0_LENGTH);
        GWD(TestMessage2_Entries_varDataField2_set)(&entries, VAR_DATA_FIELD_2_IDX_0, VAR_DATA_FIELD_2_IDX_0_LENGTH);

        GWD(TestMessage2_Entries_next)(&entries);
        GWD(TestMessage2_Entries_TagGroup1)(&entries).set_buffer(TAG_GROUP_1_IDX_1);
        GWD(TestMessage2_Entries_TagGroup2_set)(&entries, TAG_GROUP_2_IDX_1);

        GWD(TestMessage2_Entries_varDataField1_set)(&entries, VAR_DATA_FIELD_1_IDX_1, VAR_DATA_FIELD_1_IDX_1_LENGTH);
        GWD(TestMessage2_Entries_varDataField2_set)(&entries, VAR_DATA_FIELD_2_IDX_1, VAR_DATA_FIELD_2_IDX_1_LENGTH);

        return GWD(TestMessage2_encoded_length)(&m_msg2);
    }

    std::uint64_t encodeTestMessage3(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!GWD(TestMessage3_wrap_for_encode)(&m_msg3, buffer, offset, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(TestMessage3_Tag1_set)(&m_msg3, TAG_1);

        GWD(TestMessage3_Entries) entries;
        if (!GWD(TestMessage3_Entries_count_set)(&m_msg3, &entries, ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(TestMessage3_Entries_next)(&entries);
        GWD(TestMessage3_Entries_TagGroup1)(&entries).set_buffer(TAG_GROUP_1_IDX_0);

        GWD(TestMessage3_Entries_NestedEntries) nestedEntries0;
        if (!GWD(TestMessage3_Entries_NestedEntries_count_set)(&entries, &nestedEntries0, NESTED_ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries0);
        GWD(TestMessage3_Entries_NestedEntries_TagGroup2_set)(&nestedEntries0, TAG_GROUP_2_IDX_0);

        GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_set)(&nestedEntries0, VAR_DATA_FIELD_NESTED_IDX_0, VAR_DATA_FIELD_NESTED_IDX_0_LENGTH);

        GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries0);
        GWD(TestMessage3_Entries_NestedEntries_TagGroup2_set)(&nestedEntries0, TAG_GROUP_2_IDX_1);

        GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_set)(&nestedEntries0, VAR_DATA_FIELD_NESTED_IDX_1, VAR_DATA_FIELD_NESTED_IDX_1_LENGTH);

        GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries0);
        GWD(TestMessage3_Entries_NestedEntries_TagGroup2_set)(&nestedEntries0, TAG_GROUP_2_IDX_2);

        GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_set)(&nestedEntries0, VAR_DATA_FIELD_NESTED_IDX_2, VAR_DATA_FIELD_NESTED_IDX_2_LENGTH);

        GWD(TestMessage3_Entries_varDataField_set)(&entries, VAR_DATA_FIELD_IDX_0, VAR_DATA_FIELD_IDX_0_LENGTH);

        GWD(TestMessage3_Entries_next)(&entries);
        GWD(TestMessage3_Entries_TagGroup1)(&entries).set_buffer(TAG_GROUP_1_IDX_1);

        GWD(TestMessage3_Entries_NestedEntries) nestedEntries1;
        if (!GWD(TestMessage3_Entries_NestedEntries_count_set)(&entries, &nestedEntries1, NESTED_ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries1);
        GWD(TestMessage3_Entries_NestedEntries_TagGroup2_set)(&nestedEntries1, TAG_GROUP_2_IDX_3);

        GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_set)(&nestedEntries1, VAR_DATA_FIELD_NESTED_IDX_3, VAR_DATA_FIELD_NESTED_IDX_3_LENGTH);

        GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries1);
        GWD(TestMessage3_Entries_NestedEntries_TagGroup2_set)(&nestedEntries1, TAG_GROUP_2_IDX_4);


        GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_set)(&nestedEntries1, VAR_DATA_FIELD_NESTED_IDX_4, VAR_DATA_FIELD_NESTED_IDX_4_LENGTH);

        GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries1);
        GWD(TestMessage3_Entries_NestedEntries_TagGroup2_set)(&nestedEntries1, TAG_GROUP_2_IDX_5);

        GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_set)(&nestedEntries1, VAR_DATA_FIELD_NESTED_IDX_5, VAR_DATA_FIELD_NESTED_IDX_5_LENGTH);

        GWD(TestMessage3_Entries_varDataField_set)(&entries, VAR_DATA_FIELD_IDX_1, VAR_DATA_FIELD_IDX_1_LENGTH);

        return GWD(TestMessage3_encoded_length)(&m_msg3);
    }

    std::uint64_t encodeTestMessage4(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!GWD(TestMessage4_wrap_for_encode)(&m_msg4, buffer, offset, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(TestMessage4_Tag1_set)(&m_msg4, TAG_1);

        GWD(TestMessage4_Entries) entries;
        if (!GWD(TestMessage4_Entries_count_set)(&m_msg4, &entries, ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(TestMessage4_Entries_next)(&entries);

        GWD(TestMessage4_Entries_varDataField1_set)(&entries, VAR_DATA_FIELD_1_IDX_0, VAR_DATA_FIELD_1_IDX_0_LENGTH);
        GWD(TestMessage4_Entries_varDataField2_set)(&entries, VAR_DATA_FIELD_2_IDX_0, VAR_DATA_FIELD_2_IDX_0_LENGTH);

        GWD(TestMessage4_Entries_next)(&entries);

        GWD(TestMessage4_Entries_varDataField1_set)(&entries, VAR_DATA_FIELD_1_IDX_1, VAR_DATA_FIELD_1_IDX_1_LENGTH);
        GWD(TestMessage4_Entries_varDataField2_set)(&entries, VAR_DATA_FIELD_2_IDX_1, VAR_DATA_FIELD_2_IDX_1_LENGTH);

        return GWD(TestMessage4_encoded_length)(&m_msg4);
    }

    GWD(TestMessage1) m_msg1;
    GWD(TestMessage2) m_msg2;
    GWD(TestMessage3) m_msg3;
    GWD(TestMessage4) m_msg4;
};

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage1Correctly)
{
    char buffer[2048];
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage1(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(GWD(TestMessage1_sbe_block_length)(), 16);
    offset += 16;  // root blockLength of 16

    // entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), TAG_GROUP_1_IDX_0_LENGTH + sizeof(std::int64_t));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_0_LENGTH), std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    offset += TAG_GROUP_1_IDX_0_LENGTH;
    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_0);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_IDX_0_LENGTH), VAR_DATA_FIELD_IDX_0);
    offset += VAR_DATA_FIELD_IDX_0_LENGTH;

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_1_LENGTH), std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));
    offset += TAG_GROUP_1_IDX_1_LENGTH;
    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_1);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_IDX_1_LENGTH), VAR_DATA_FIELD_IDX_1);
    offset += VAR_DATA_FIELD_IDX_1_LENGTH;

    EXPECT_EQ(sz, offset);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeAndDecodeTestMessage1Correctly)
{
    char buffer[2048];
    std::uint64_t sz = encodeTestMessage1(buffer, 0, sizeof(buffer));

    EXPECT_EQ(sz, expectedTestMessage1Size);

    GWD(TestMessage1) msg1Decoder;
    if (!GWD(TestMessage1_reset)(&msg1Decoder, buffer, 0, sizeof(buffer), GWD(TestMessage1_sbe_block_length)(), GWD(TestMessage1_sbe_schema_version)()))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(GWD(TestMessage1_Tag1)(&msg1Decoder), TAG_1);

    GWD(TestMessage1_Entries) entries;
    if (!GWD(TestMessage1_Entries_get)(&msg1Decoder, &entries))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(TestMessage1_Entries_count)(&entries), ENTRIES_COUNT);

    ASSERT_TRUE(GWD(TestMessage1_Entries_has_next)(&entries));
    GWD(TestMessage1_Entries_next)(&entries);

    EXPECT_EQ(GWD(TestMessage1_Entries_TagGroup1_length)(), TAG_GROUP_1_IDX_0_LENGTH);
    EXPECT_EQ(GWD(TestMessage1_Entries_TagGroup1)(&entries).str(), std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    EXPECT_EQ(GWD(TestMessage1_Entries_TagGroup2)(&entries), TAG_GROUP_2_IDX_0);
    EXPECT_EQ(GWD(TestMessage1_Entries_varDataField_length)(&entries), VAR_DATA_FIELD_IDX_0_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage1_Entries_varDataField)(&entries)), VAR_DATA_FIELD_IDX_0);

    ASSERT_TRUE(GWD(TestMessage1_Entries_has_next)(&entries));
    GWD(TestMessage1_Entries_next)(&entries);

    EXPECT_EQ(GWD(TestMessage1_Entries_TagGroup1_length)(), TAG_GROUP_1_IDX_1_LENGTH);
    EXPECT_EQ(GWD(TestMessage1_Entries_TagGroup1)(&entries).str(), std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));
    EXPECT_EQ(GWD(TestMessage1_Entries_TagGroup2)(&entries), TAG_GROUP_2_IDX_1);
    EXPECT_EQ(GWD(TestMessage1_Entries_varDataField_length)(&entries), VAR_DATA_FIELD_IDX_1_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage1_Entries_varDataField)(&entries)), VAR_DATA_FIELD_IDX_1);

    EXPECT_EQ(GWD(TestMessage1_encoded_length)(&msg1Decoder), expectedTestMessage1Size);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage2Correctly)
{
    char buffer[2048];
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage2(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(GWD(TestMessage2_sbe_block_length)(), 16);
    offset += 16;  // root blockLength of 16

    // entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), TAG_GROUP_1_IDX_0_LENGTH + sizeof(std::int64_t));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_0_LENGTH), std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    offset += TAG_GROUP_1_IDX_0_LENGTH;
    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_0);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_1_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_1_IDX_0_LENGTH), VAR_DATA_FIELD_1_IDX_0);
    offset += VAR_DATA_FIELD_1_IDX_0_LENGTH;
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_2_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_2_IDX_0_LENGTH), VAR_DATA_FIELD_2_IDX_0);
    offset += VAR_DATA_FIELD_2_IDX_0_LENGTH;

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_1_LENGTH), std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));
    offset += TAG_GROUP_1_IDX_1_LENGTH;
    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_1);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_1_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_1_IDX_1_LENGTH), VAR_DATA_FIELD_1_IDX_1);
    offset += VAR_DATA_FIELD_1_IDX_1_LENGTH;
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_2_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_2_IDX_1_LENGTH), VAR_DATA_FIELD_2_IDX_1);
    offset += VAR_DATA_FIELD_2_IDX_1_LENGTH;

    EXPECT_EQ(sz, offset);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeAndDecodeTestMessage2Correctly)
{
    char buffer[2048];
    std::uint64_t sz = encodeTestMessage2(buffer, 0, sizeof(buffer));

    EXPECT_EQ(sz, expectedTestMessage2Size);

    GWD(TestMessage2) msg2Decoder;
    if (!GWD(TestMessage2_reset)(&msg2Decoder, buffer, 0, sizeof(buffer), GWD(TestMessage2_sbe_block_length)(), GWD(TestMessage2_sbe_schema_version)()))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(GWD(TestMessage2_Tag1)(&msg2Decoder), TAG_1);

    GWD(TestMessage2_Entries) entries;
    if (!GWD(TestMessage2_Entries_get)(&msg2Decoder, &entries))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(TestMessage2_Entries_count)(&entries), ENTRIES_COUNT);

    ASSERT_TRUE(GWD(TestMessage2_Entries_has_next)(&entries));
    GWD(TestMessage2_Entries_next)(&entries);

    EXPECT_EQ(GWD(TestMessage2_Entries_TagGroup1_length)(), TAG_GROUP_1_IDX_0_LENGTH);
    EXPECT_EQ(GWD(TestMessage2_Entries_TagGroup1)(&entries).str(), std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    EXPECT_EQ(GWD(TestMessage2_Entries_TagGroup2)(&entries), TAG_GROUP_2_IDX_0);
    EXPECT_EQ(GWD(TestMessage2_Entries_varDataField1_length)(&entries), VAR_DATA_FIELD_1_IDX_0_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage2_Entries_varDataField1)(&entries)), VAR_DATA_FIELD_1_IDX_0);
    EXPECT_EQ(GWD(TestMessage2_Entries_varDataField2_length)(&entries), VAR_DATA_FIELD_2_IDX_0_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage2_Entries_varDataField2)(&entries)), VAR_DATA_FIELD_2_IDX_0);

    ASSERT_TRUE(GWD(TestMessage2_Entries_has_next)(&entries));
    GWD(TestMessage2_Entries_next)(&entries);

    EXPECT_EQ(GWD(TestMessage2_Entries_TagGroup1_length)(), TAG_GROUP_1_IDX_1_LENGTH);
    EXPECT_EQ(GWD(TestMessage2_Entries_TagGroup1)(&entries).str(), std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));


    EXPECT_EQ(GWD(TestMessage2_Entries_TagGroup2)(&entries), TAG_GROUP_2_IDX_1);
    EXPECT_EQ(GWD(TestMessage2_Entries_varDataField1_length)(&entries), VAR_DATA_FIELD_1_IDX_1_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage2_Entries_varDataField1)(&entries)), VAR_DATA_FIELD_1_IDX_1);
    EXPECT_EQ(GWD(TestMessage2_Entries_varDataField2_length)(&entries), VAR_DATA_FIELD_2_IDX_1_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage2_Entries_varDataField2)(&entries)), VAR_DATA_FIELD_2_IDX_1);

    EXPECT_EQ(GWD(TestMessage2_encoded_length)(&msg2Decoder), expectedTestMessage2Size);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage3Correctly)
{
    char buffer[2048];
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage3(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(GWD(TestMessage1_sbe_block_length)(), 16);
    offset += 16;  // root blockLength of 16

    // entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), TAG_GROUP_1_IDX_0_LENGTH);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_0_LENGTH), std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    offset += TAG_GROUP_1_IDX_0_LENGTH;

    // nested entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), sizeof(std::int64_t));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), NESTED_ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_0);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_0_LENGTH), VAR_DATA_FIELD_NESTED_IDX_0);
    offset += VAR_DATA_FIELD_NESTED_IDX_0_LENGTH;

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_1);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_1_LENGTH), VAR_DATA_FIELD_NESTED_IDX_1);
    offset += VAR_DATA_FIELD_NESTED_IDX_1_LENGTH;

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_2);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_2_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_2_LENGTH), VAR_DATA_FIELD_NESTED_IDX_2);
    offset += VAR_DATA_FIELD_NESTED_IDX_2_LENGTH;

    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_IDX_0_LENGTH), VAR_DATA_FIELD_IDX_0);
    offset += VAR_DATA_FIELD_IDX_0_LENGTH;

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_1_LENGTH), std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));
    offset += TAG_GROUP_1_IDX_1_LENGTH;

    // nested entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), sizeof(std::int64_t));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), NESTED_ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_3);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_3_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_3_LENGTH), VAR_DATA_FIELD_NESTED_IDX_3);
    offset += VAR_DATA_FIELD_NESTED_IDX_3_LENGTH;

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_4);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_4_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_4_LENGTH), VAR_DATA_FIELD_NESTED_IDX_4);
    offset += VAR_DATA_FIELD_NESTED_IDX_4_LENGTH;

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_5);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_5_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_5_LENGTH), VAR_DATA_FIELD_NESTED_IDX_5);
    offset += VAR_DATA_FIELD_NESTED_IDX_5_LENGTH;

    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_IDX_1_LENGTH), VAR_DATA_FIELD_IDX_1);
    offset += VAR_DATA_FIELD_IDX_1_LENGTH;

    EXPECT_EQ(sz, offset);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeAndDecodeTestMessage3Correctly)
{
    char buffer[2048];
    std::uint64_t sz = encodeTestMessage3(buffer, 0, sizeof(buffer));

    EXPECT_EQ(sz, expectedTestMessage3Size);

    GWD(TestMessage3) msg3Decoder;
    if (!GWD(TestMessage3_reset)(&msg3Decoder, buffer, 0, sizeof(buffer), GWD(TestMessage3_sbe_block_length)(), GWD(TestMessage3_sbe_schema_version)()))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(GWD(TestMessage3_Tag1)(&msg3Decoder), TAG_1);

    GWD(TestMessage3_Entries) entries;
    if (!GWD(TestMessage3_Entries_get)(&msg3Decoder, &entries))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(TestMessage3_Entries_count)(&entries), ENTRIES_COUNT);

    ASSERT_TRUE(GWD(TestMessage3_Entries_has_next)(&entries));
    GWD(TestMessage3_Entries_next)(&entries);

    EXPECT_EQ(GWD(TestMessage3_Entries_TagGroup1_length)(), TAG_GROUP_1_IDX_0_LENGTH);
    EXPECT_EQ(GWD(TestMessage3_Entries_TagGroup1)(&entries).str(), std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));

    GWD(TestMessage3_Entries_NestedEntries) nestedEntries0;
    if (!GWD(TestMessage3_Entries_NestedEntries_get)(&entries, &nestedEntries0))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_count)(&nestedEntries0), NESTED_ENTRIES_COUNT);

    ASSERT_TRUE(GWD(TestMessage3_Entries_NestedEntries_has_next)(&nestedEntries0));
    GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries0);

    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_TagGroup2)(&nestedEntries0), TAG_GROUP_2_IDX_0);
    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_length)(&nestedEntries0), VAR_DATA_FIELD_NESTED_IDX_0_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested)(&nestedEntries0)), VAR_DATA_FIELD_NESTED_IDX_0);

    ASSERT_TRUE(GWD(TestMessage3_Entries_NestedEntries_has_next)(&nestedEntries0));
    GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries0);

    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_TagGroup2)(&nestedEntries0), TAG_GROUP_2_IDX_1);
    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_length)(&nestedEntries0), VAR_DATA_FIELD_NESTED_IDX_1_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested)(&nestedEntries0)), VAR_DATA_FIELD_NESTED_IDX_1);

    ASSERT_TRUE(GWD(TestMessage3_Entries_NestedEntries_has_next)(&nestedEntries0));
    GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries0);

    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_TagGroup2)(&nestedEntries0), TAG_GROUP_2_IDX_2);
    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_length)(&nestedEntries0), VAR_DATA_FIELD_NESTED_IDX_2_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested)(&nestedEntries0)), VAR_DATA_FIELD_NESTED_IDX_2);

    EXPECT_EQ(GWD(TestMessage3_Entries_varDataField_length)(&entries), VAR_DATA_FIELD_IDX_0_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage3_Entries_varDataField)(&entries)), VAR_DATA_FIELD_IDX_0);

    ASSERT_TRUE(GWD(TestMessage3_Entries_has_next)(&entries));
    GWD(TestMessage3_Entries_next)(&entries);

    GWD(TestMessage3_Entries_NestedEntries) nestedEntries1;
    if (!GWD(TestMessage3_Entries_NestedEntries_get)(&entries, &nestedEntries1))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_count)(&nestedEntries1), NESTED_ENTRIES_COUNT);

    ASSERT_TRUE(GWD(TestMessage3_Entries_NestedEntries_has_next)(&nestedEntries1));
    GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries1);

    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_TagGroup2)(&nestedEntries1), TAG_GROUP_2_IDX_3);
    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_length)(&nestedEntries1), VAR_DATA_FIELD_NESTED_IDX_3_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested)(&nestedEntries1)), VAR_DATA_FIELD_NESTED_IDX_3);

    ASSERT_TRUE(GWD(TestMessage3_Entries_NestedEntries_has_next)(&nestedEntries1));
    GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries1);

    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_TagGroup2)(&nestedEntries1), TAG_GROUP_2_IDX_4);
    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_length)(&nestedEntries1), VAR_DATA_FIELD_NESTED_IDX_4_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested)(&nestedEntries1)), VAR_DATA_FIELD_NESTED_IDX_4);

    ASSERT_TRUE(GWD(TestMessage3_Entries_NestedEntries_has_next)(&nestedEntries1));
    GWD(TestMessage3_Entries_NestedEntries_next)(&nestedEntries1);

    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_TagGroup2)(&nestedEntries1), TAG_GROUP_2_IDX_5);
    EXPECT_EQ(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested_length)(&nestedEntries1), VAR_DATA_FIELD_NESTED_IDX_5_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage3_Entries_NestedEntries_varDataFieldNested)(&nestedEntries1)), VAR_DATA_FIELD_NESTED_IDX_5);

    EXPECT_EQ(GWD(TestMessage3_Entries_varDataField_length)(&entries), VAR_DATA_FIELD_IDX_1_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage3_Entries_varDataField)(&entries)), VAR_DATA_FIELD_IDX_1);

    EXPECT_EQ(GWD(TestMessage3_encoded_length)(&msg3Decoder), expectedTestMessage3Size);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage4Correctly)
{
    char buffer[2048];
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage4(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(GWD(TestMessage4_sbe_block_length)(), 16);
    offset += 16;  // root blockLength of 16

    // entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), 0);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_1_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_1_IDX_0_LENGTH), VAR_DATA_FIELD_1_IDX_0);
    offset += VAR_DATA_FIELD_1_IDX_0_LENGTH;
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_2_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_2_IDX_0_LENGTH), VAR_DATA_FIELD_2_IDX_0);
    offset += VAR_DATA_FIELD_2_IDX_0_LENGTH;

    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_1_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_1_IDX_1_LENGTH), VAR_DATA_FIELD_1_IDX_1);
    offset += VAR_DATA_FIELD_1_IDX_1_LENGTH;
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_2_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_2_IDX_1_LENGTH), VAR_DATA_FIELD_2_IDX_1);
    offset += VAR_DATA_FIELD_2_IDX_1_LENGTH;

    EXPECT_EQ(sz, offset);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeAndDecodeTestMessage4Correctly)
{
    char buffer[2048];
    std::uint64_t sz = encodeTestMessage4(buffer, 0, sizeof(buffer));

    EXPECT_EQ(sz, expectedTestMessage4Size);

    GWD(TestMessage4) msg4Decoder;
    if (!GWD(TestMessage4_reset)(&msg4Decoder, buffer, 0, sizeof(buffer), GWD(TestMessage4_sbe_block_length)(), GWD(TestMessage4_sbe_schema_version)()))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(GWD(TestMessage4_Tag1)(&msg4Decoder), TAG_1);

    GWD(TestMessage4_Entries) entries;
    if (!GWD(TestMessage4_Entries_get)(&msg4Decoder, &entries))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(TestMessage4_Entries_count)(&entries), ENTRIES_COUNT);

    ASSERT_TRUE(GWD(TestMessage4_Entries_has_next)(&entries));
    GWD(TestMessage4_Entries_next)(&entries);

    EXPECT_EQ(GWD(TestMessage4_Entries_varDataField1_length)(&entries), VAR_DATA_FIELD_1_IDX_0_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage4_Entries_varDataField1)(&entries)), VAR_DATA_FIELD_1_IDX_0);
    EXPECT_EQ(GWD(TestMessage4_Entries_varDataField2_length)(&entries), VAR_DATA_FIELD_2_IDX_0_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage4_Entries_varDataField2)(&entries)), VAR_DATA_FIELD_2_IDX_0);

    ASSERT_TRUE(GWD(TestMessage4_Entries_has_next)(&entries));
    GWD(TestMessage4_Entries_next)(&entries);

    EXPECT_EQ(GWD(TestMessage4_Entries_varDataField1_length)(&entries), VAR_DATA_FIELD_1_IDX_1_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage4_Entries_varDataField1)(&entries)), VAR_DATA_FIELD_1_IDX_1);
    EXPECT_EQ(GWD(TestMessage4_Entries_varDataField2_length)(&entries), VAR_DATA_FIELD_2_IDX_1_LENGTH);
    EXPECT_EQ(sbe_view_to_string(GWD(TestMessage4_Entries_varDataField2)(&entries)), VAR_DATA_FIELD_2_IDX_1);

    EXPECT_EQ(GWD(TestMessage4_encoded_length)(&msg4Decoder), expectedTestMessage4Size);
}
