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

#include "gtest/gtest.h"
#include "group_with_data/group_with_data_cpp.h"

// using namespace std;
using namespace group::with::data;

static const std::uint32_t TAG_1 = 32;
static const std::uint64_t ENTRIES_COUNT = 2;

static const char TAG_GROUP_1_IDX_0[] = { 'T', 'a', 'g', 'G', 'r', 'o', 'u', 'p', '0' };
static const char TAG_GROUP_1_IDX_1[] = { 'T', 'a', 'g', 'G', 'r', 'o', 'u', 'p', '1' };
static const std::size_t TAG_GROUP_1_IDX_0_LENGTH = sizeof(TAG_GROUP_1_IDX_0);
static const std::size_t TAG_GROUP_1_IDX_1_LENGTH = sizeof(TAG_GROUP_1_IDX_1);

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
        m_msg1.wrapForEncode(buffer, offset, bufferLength);

        m_msg1.Tag1(TAG_1);

        TestMessage1Groups::Entries &entries = m_msg1.EntriesCount(ENTRIES_COUNT);

        entries.next()
            .TagGroup1Set(TAG_GROUP_1_IDX_0)
            .TagGroup2(TAG_GROUP_2_IDX_0);

        entries.varDataFieldSet(VAR_DATA_FIELD_IDX_0, VAR_DATA_FIELD_IDX_0_LENGTH);

        entries.next()
            .TagGroup1Set(TAG_GROUP_1_IDX_1)
            .TagGroup2(TAG_GROUP_2_IDX_1);

        entries.varDataFieldSet(VAR_DATA_FIELD_IDX_1, VAR_DATA_FIELD_IDX_1_LENGTH);

        return m_msg1.encodedLength();
    }

    std::uint64_t encodeTestMessage2(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        m_msg2.wrapForEncode(buffer, offset, bufferLength);

        m_msg2.Tag1(TAG_1);

        TestMessage2Groups::Entries &entries = m_msg2.EntriesCount(ENTRIES_COUNT);

        entries.next()
            .TagGroup1Set(TAG_GROUP_1_IDX_0)
            .TagGroup2(TAG_GROUP_2_IDX_0);

        entries.varDataField1Set(VAR_DATA_FIELD_1_IDX_0, VAR_DATA_FIELD_1_IDX_0_LENGTH);
        entries.varDataField2Set(VAR_DATA_FIELD_2_IDX_0, VAR_DATA_FIELD_2_IDX_0_LENGTH);

        entries.next()
            .TagGroup1Set(TAG_GROUP_1_IDX_1)
            .TagGroup2(TAG_GROUP_2_IDX_1);

        entries.varDataField1Set(VAR_DATA_FIELD_1_IDX_1, VAR_DATA_FIELD_1_IDX_1_LENGTH);
        entries.varDataField2Set(VAR_DATA_FIELD_2_IDX_1, VAR_DATA_FIELD_2_IDX_1_LENGTH);

        return m_msg2.encodedLength();
    }

    std::uint64_t encodeTestMessage3(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        m_msg3.wrapForEncode(buffer, offset, bufferLength);

        m_msg3.Tag1(TAG_1);

        TestMessage3Groups::Entries &entries = m_msg3.EntriesCount(ENTRIES_COUNT);

        entries.next()
            .TagGroup1Set(TAG_GROUP_1_IDX_0);

        TestMessage3Groups::EntriesGroups::NestedEntries &nestedEntries0 = entries.NestedEntriesCount(NESTED_ENTRIES_COUNT);

        nestedEntries0.next()
            .TagGroup2(TAG_GROUP_2_IDX_0);

        nestedEntries0.varDataFieldNestedSet(VAR_DATA_FIELD_NESTED_IDX_0, VAR_DATA_FIELD_NESTED_IDX_0_LENGTH);

        nestedEntries0.next()
            .TagGroup2(TAG_GROUP_2_IDX_1);

        nestedEntries0.varDataFieldNestedSet(VAR_DATA_FIELD_NESTED_IDX_1, VAR_DATA_FIELD_NESTED_IDX_1_LENGTH);

        nestedEntries0.next()
            .TagGroup2(TAG_GROUP_2_IDX_2);

        nestedEntries0.varDataFieldNestedSet(VAR_DATA_FIELD_NESTED_IDX_2, VAR_DATA_FIELD_NESTED_IDX_2_LENGTH);

        entries.varDataFieldSet(VAR_DATA_FIELD_IDX_0, VAR_DATA_FIELD_IDX_0_LENGTH);

        entries.next()
            .TagGroup1Set(TAG_GROUP_1_IDX_1);

        TestMessage3Groups::EntriesGroups::NestedEntries &nestedEntries1 = entries.NestedEntriesCount(NESTED_ENTRIES_COUNT);

        nestedEntries1.next()
            .TagGroup2(TAG_GROUP_2_IDX_3);

        nestedEntries1.varDataFieldNestedSet(VAR_DATA_FIELD_NESTED_IDX_3, VAR_DATA_FIELD_NESTED_IDX_3_LENGTH);

        nestedEntries1.next()
            .TagGroup2(TAG_GROUP_2_IDX_4);

        nestedEntries1.varDataFieldNestedSet(VAR_DATA_FIELD_NESTED_IDX_4, VAR_DATA_FIELD_NESTED_IDX_4_LENGTH);

        nestedEntries1.next()
            .TagGroup2(TAG_GROUP_2_IDX_5);

        nestedEntries1.varDataFieldNestedSet(VAR_DATA_FIELD_NESTED_IDX_5, VAR_DATA_FIELD_NESTED_IDX_5_LENGTH);

        entries.varDataFieldSet(VAR_DATA_FIELD_IDX_1, VAR_DATA_FIELD_IDX_1_LENGTH);

        return m_msg3.encodedLength();
    }

    std::uint64_t encodeTestMessage4(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        m_msg4.wrapForEncode(buffer, offset, bufferLength);

        m_msg4.Tag1(TAG_1);

        TestMessage4Groups::Entries &entries = m_msg4.EntriesCount(ENTRIES_COUNT);

        entries.next();

        entries.varDataField1Set(VAR_DATA_FIELD_1_IDX_0, VAR_DATA_FIELD_1_IDX_0_LENGTH);
        entries.varDataField2Set(VAR_DATA_FIELD_2_IDX_0, VAR_DATA_FIELD_2_IDX_0_LENGTH);

        entries.next();

        entries.varDataField1Set(VAR_DATA_FIELD_1_IDX_1, VAR_DATA_FIELD_1_IDX_1_LENGTH);
        entries.varDataField2Set(VAR_DATA_FIELD_2_IDX_1, VAR_DATA_FIELD_2_IDX_1_LENGTH);

        return m_msg4.encodedLength();
    }

    TestMessage1 m_msg1;
    TestMessage2 m_msg2;
    TestMessage3 m_msg3;
    TestMessage4 m_msg4;
};

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage1Correctly)
{
    char buffer[2048];
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage1(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(TestMessage1::sbeBlockLength(), 16);
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

    TestMessage1 msg1Decoder(buffer, sizeof(buffer), TestMessage1::sbeBlockLength(), TestMessage1::sbeSchemaVersion());

    EXPECT_EQ(msg1Decoder.Tag1(), TAG_1);

    TestMessage1Groups::Entries &entries = msg1Decoder.Entries();
    EXPECT_EQ(entries.count(), ENTRIES_COUNT);

    ASSERT_TRUE(entries.hasNext());
    entries.next();

    EXPECT_EQ(entries.TagGroup1Length(), TAG_GROUP_1_IDX_0_LENGTH);
    EXPECT_EQ(std::string(entries.TagGroup1(), static_cast<std::size_t>(entries.TagGroup1Length())), std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    EXPECT_EQ(entries.TagGroup2(), TAG_GROUP_2_IDX_0);
    EXPECT_EQ(entries.varDataFieldLength(), VAR_DATA_FIELD_IDX_0_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField(), VAR_DATA_FIELD_IDX_0_LENGTH), VAR_DATA_FIELD_IDX_0);

    ASSERT_TRUE(entries.hasNext());
    entries.next();

    EXPECT_EQ(entries.TagGroup1Length(), TAG_GROUP_1_IDX_1_LENGTH);
    EXPECT_EQ(std::string(entries.TagGroup1(), static_cast<std::size_t>(entries.TagGroup1Length())), std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));
    EXPECT_EQ(entries.TagGroup2(), TAG_GROUP_2_IDX_1);
    EXPECT_EQ(entries.varDataFieldLength(), VAR_DATA_FIELD_IDX_1_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField(), VAR_DATA_FIELD_IDX_1_LENGTH), VAR_DATA_FIELD_IDX_1);

    EXPECT_EQ(msg1Decoder.encodedLength(), expectedTestMessage1Size);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage2Correctly)
{
    char buffer[2048];
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage2(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(TestMessage2::sbeBlockLength(), 16);
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

    TestMessage2 msg2Decoder(buffer, sizeof(buffer), TestMessage2::sbeBlockLength(), TestMessage2::sbeSchemaVersion());

    EXPECT_EQ(msg2Decoder.Tag1(), TAG_1);

    TestMessage2Groups::Entries &entries = msg2Decoder.Entries();
    EXPECT_EQ(entries.count(), ENTRIES_COUNT);

    ASSERT_TRUE(entries.hasNext());
    entries.next();

    EXPECT_EQ(entries.TagGroup1Length(), TAG_GROUP_1_IDX_0_LENGTH);
    EXPECT_EQ(std::string(entries.TagGroup1(), static_cast<std::size_t>(entries.TagGroup1Length())), std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    EXPECT_EQ(entries.TagGroup2(), TAG_GROUP_2_IDX_0);
    EXPECT_EQ(entries.varDataField1Length(), VAR_DATA_FIELD_1_IDX_0_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField1(), VAR_DATA_FIELD_1_IDX_0_LENGTH), VAR_DATA_FIELD_1_IDX_0);
    EXPECT_EQ(entries.varDataField2Length(), VAR_DATA_FIELD_2_IDX_0_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField2(), VAR_DATA_FIELD_2_IDX_0_LENGTH), VAR_DATA_FIELD_2_IDX_0);

    ASSERT_TRUE(entries.hasNext());
    entries.next();

    EXPECT_EQ(entries.TagGroup1Length(), TAG_GROUP_1_IDX_1_LENGTH);
    EXPECT_EQ(std::string(entries.TagGroup1(), static_cast<std::size_t>(entries.TagGroup1Length())), std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));
    EXPECT_EQ(entries.TagGroup2(), TAG_GROUP_2_IDX_1);
    EXPECT_EQ(entries.varDataField1Length(), VAR_DATA_FIELD_1_IDX_1_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField1(), VAR_DATA_FIELD_1_IDX_1_LENGTH), VAR_DATA_FIELD_1_IDX_1);
    EXPECT_EQ(entries.varDataField2Length(), VAR_DATA_FIELD_2_IDX_1_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField2(), VAR_DATA_FIELD_2_IDX_1_LENGTH), VAR_DATA_FIELD_2_IDX_1);

    EXPECT_EQ(msg2Decoder.encodedLength(), expectedTestMessage2Size);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage3Correctly)
{
    char buffer[2048];
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage3(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(TestMessage1::sbeBlockLength(), 16);
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

    TestMessage3 msg3Decoder(buffer, sizeof(buffer), TestMessage3::sbeBlockLength(), TestMessage3::sbeSchemaVersion());

    EXPECT_EQ(msg3Decoder.Tag1(), TAG_1);

    TestMessage3Groups::Entries &entries = msg3Decoder.Entries();
    EXPECT_EQ(entries.count(), ENTRIES_COUNT);

    ASSERT_TRUE(entries.hasNext());
    entries.next();

    EXPECT_EQ(entries.TagGroup1Length(), TAG_GROUP_1_IDX_0_LENGTH);
    EXPECT_EQ(std::string(entries.TagGroup1(), static_cast<std::size_t>(entries.TagGroup1Length())), std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));

    TestMessage3Groups::EntriesGroups::NestedEntries &nestedEntries0 = entries.NestedEntries();
    EXPECT_EQ(nestedEntries0.count(), NESTED_ENTRIES_COUNT);

    ASSERT_TRUE(nestedEntries0.hasNext());
    nestedEntries0.next();

    EXPECT_EQ(nestedEntries0.TagGroup2(), TAG_GROUP_2_IDX_0);
    EXPECT_EQ(nestedEntries0.varDataFieldNestedLength(), VAR_DATA_FIELD_NESTED_IDX_0_LENGTH);
    EXPECT_EQ(std::string(nestedEntries0.varDataFieldNested(), VAR_DATA_FIELD_NESTED_IDX_0_LENGTH), VAR_DATA_FIELD_NESTED_IDX_0);

    ASSERT_TRUE(nestedEntries0.hasNext());
    nestedEntries0.next();

    EXPECT_EQ(nestedEntries0.TagGroup2(), TAG_GROUP_2_IDX_1);
    EXPECT_EQ(nestedEntries0.varDataFieldNestedLength(), VAR_DATA_FIELD_NESTED_IDX_1_LENGTH);
    EXPECT_EQ(std::string(nestedEntries0.varDataFieldNested(), VAR_DATA_FIELD_NESTED_IDX_1_LENGTH), VAR_DATA_FIELD_NESTED_IDX_1);

    ASSERT_TRUE(nestedEntries0.hasNext());
    nestedEntries0.next();

    EXPECT_EQ(nestedEntries0.TagGroup2(), TAG_GROUP_2_IDX_2);
    EXPECT_EQ(nestedEntries0.varDataFieldNestedLength(), VAR_DATA_FIELD_NESTED_IDX_2_LENGTH);
    EXPECT_EQ(std::string(nestedEntries0.varDataFieldNested(), VAR_DATA_FIELD_NESTED_IDX_2_LENGTH), VAR_DATA_FIELD_NESTED_IDX_2);

    EXPECT_EQ(entries.varDataFieldLength(), VAR_DATA_FIELD_IDX_0_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField(), VAR_DATA_FIELD_IDX_0_LENGTH), VAR_DATA_FIELD_IDX_0);

    ASSERT_TRUE(entries.hasNext());
    entries.next();

    TestMessage3Groups::EntriesGroups::NestedEntries &nestedEntries1 = entries.NestedEntries();
    EXPECT_EQ(nestedEntries1.count(), NESTED_ENTRIES_COUNT);

    ASSERT_TRUE(nestedEntries1.hasNext());
    nestedEntries1.next();

    EXPECT_EQ(nestedEntries1.TagGroup2(), TAG_GROUP_2_IDX_3);
    EXPECT_EQ(nestedEntries1.varDataFieldNestedLength(), VAR_DATA_FIELD_NESTED_IDX_3_LENGTH);
    EXPECT_EQ(std::string(nestedEntries1.varDataFieldNested(), VAR_DATA_FIELD_NESTED_IDX_3_LENGTH), VAR_DATA_FIELD_NESTED_IDX_3);

    ASSERT_TRUE(nestedEntries1.hasNext());
    nestedEntries1.next();

    EXPECT_EQ(nestedEntries1.TagGroup2(), TAG_GROUP_2_IDX_4);
    EXPECT_EQ(nestedEntries1.varDataFieldNestedLength(), VAR_DATA_FIELD_NESTED_IDX_4_LENGTH);
    EXPECT_EQ(std::string(nestedEntries1.varDataFieldNested(), VAR_DATA_FIELD_NESTED_IDX_4_LENGTH), VAR_DATA_FIELD_NESTED_IDX_4);

    ASSERT_TRUE(nestedEntries1.hasNext());
    nestedEntries1.next();

    EXPECT_EQ(nestedEntries1.TagGroup2(), TAG_GROUP_2_IDX_5);
    EXPECT_EQ(nestedEntries1.varDataFieldNestedLength(), VAR_DATA_FIELD_NESTED_IDX_5_LENGTH);
    EXPECT_EQ(std::string(nestedEntries1.varDataFieldNested(), VAR_DATA_FIELD_NESTED_IDX_5_LENGTH), VAR_DATA_FIELD_NESTED_IDX_5);

    EXPECT_EQ(entries.varDataFieldLength(), VAR_DATA_FIELD_IDX_1_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField(), VAR_DATA_FIELD_IDX_1_LENGTH), VAR_DATA_FIELD_IDX_1);

    EXPECT_EQ(msg3Decoder.encodedLength(), expectedTestMessage3Size);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage4Correctly)
{
    char buffer[2048];
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage4(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(TestMessage4::sbeBlockLength(), 16);
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

    TestMessage4 msg4Decoder(buffer, sizeof(buffer), TestMessage4::sbeBlockLength(), TestMessage4::sbeSchemaVersion());

    EXPECT_EQ(msg4Decoder.Tag1(), TAG_1);

    TestMessage4Groups::Entries &entries = msg4Decoder.Entries();
    EXPECT_EQ(entries.count(), ENTRIES_COUNT);

    ASSERT_TRUE(entries.hasNext());
    entries.next();

    EXPECT_EQ(entries.varDataField1Length(), VAR_DATA_FIELD_1_IDX_0_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField1(), VAR_DATA_FIELD_1_IDX_0_LENGTH), VAR_DATA_FIELD_1_IDX_0);
    EXPECT_EQ(entries.varDataField2Length(), VAR_DATA_FIELD_2_IDX_0_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField2(), VAR_DATA_FIELD_2_IDX_0_LENGTH), VAR_DATA_FIELD_2_IDX_0);

    ASSERT_TRUE(entries.hasNext());
    entries.next();

    EXPECT_EQ(entries.varDataField1Length(), VAR_DATA_FIELD_1_IDX_1_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField1(), VAR_DATA_FIELD_1_IDX_1_LENGTH), VAR_DATA_FIELD_1_IDX_1);
    EXPECT_EQ(entries.varDataField2Length(), VAR_DATA_FIELD_2_IDX_1_LENGTH);
    EXPECT_EQ(std::string(entries.varDataField2(), VAR_DATA_FIELD_2_IDX_1_LENGTH), VAR_DATA_FIELD_2_IDX_1);

    EXPECT_EQ(msg4Decoder.encodedLength(), expectedTestMessage4Size);
}
