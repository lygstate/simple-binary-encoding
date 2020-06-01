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
#ifndef _OTF_IRDECODER_H
#define _OTF_IRDECODER_H

#if defined(WIN32) || defined(_WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#define fileno _fileno
#define read _read
#define stat _stat64
#else
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/stat.h>
#endif /* WIN32 */

#include <memory>
#include <exception>
#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>

#include "uk_co_real_logic_sbe_ir_generated/uk_co_real_logic_sbe_ir_generated_cpp.h"
#include "Token.h"

using namespace sbe::otf;

namespace sbe { namespace otf {

class IrDecoder
{
public:
    IrDecoder() :
        m_length(0)
    {
    }

    int decode(char *irBuffer, std::uint64_t length)
    {
        m_length = length;
        if (m_length == 0)
        {
            return -1;
        }
        std::unique_ptr<char[]> buffer(new char[static_cast<size_t>(m_length)]);
        m_buffer = std::move(buffer);
        std::memcpy(m_buffer.get(), irBuffer, static_cast<size_t>(m_length));

        return decodeIr();
    }

    int decode(const char *filename)
    {
        long long fileSize = getFileSize(filename);

        if (fileSize < 0)
        {
            return -1;
        }

        m_length = static_cast<std::uint64_t>(fileSize);
        if (m_length == 0)
        {
            return -1;
        }
        std::unique_ptr<char[]> buffer(new char[static_cast<size_t>(m_length)]);
        m_buffer = std::move(buffer);

        if (readFileIntoBuffer(m_buffer.get(), filename, m_length) < 0)
        {
            return -1;
        }

        return decodeIr();
    }

    std::shared_ptr<std::vector<Token>> header()
    {
        return m_headerTokens;
    }

    std::vector<std::shared_ptr<std::vector<Token>>> messages()
    {
        return m_messages;
    }

    std::shared_ptr<std::vector<Token>> message(int id, int version)
    {
        std::shared_ptr<std::vector<Token>> result;

        std::for_each(m_messages.begin(), m_messages.end(),
            [&](std::shared_ptr<std::vector<Token>> tokens)
            {
                Token& token = tokens->at(0);

                if (token.signal() == Signal::BEGIN_MESSAGE && token.fieldId() == id && token.tokenVersion() == version)
                {
                    result = tokens;
                }
            });

        return result;
    }

    std::shared_ptr<std::vector<Token>> message(int id)
    {
        std::shared_ptr<std::vector<Token>> result;

        std::for_each(m_messages.begin(), m_messages.end(),
            [&](std::shared_ptr<std::vector<Token>> tokens)
            {
                Token& token = tokens->at(0);

                if (token.signal() == Signal::BEGIN_MESSAGE && token.fieldId() == id)
                {
                    result = tokens;
                }
            });

        return result;
    }

protected:
    // OS specifics
    static long long getFileSize(const char *filename)
    {
        struct stat fileStat;

        if (::stat(filename, &fileStat) != 0)
        {
            return -1;
        }

        return fileStat.st_size;
    }

    static int readFileIntoBuffer(char *buffer, const char *filename, std::uint64_t length)
    {
        FILE *fptr = ::fopen(filename, "rb");
        std::uint64_t remaining = length;

        if (nullptr == fptr)
        {
            return -1;
        }

        int fd = fileno(fptr);
        while (remaining > 0)
        {
            unsigned int bytes = static_cast<unsigned int>(4098 < remaining ? 4098 : remaining);
            long long sz = ::read(fd, buffer + (length - remaining), bytes);
            remaining -= sz;
            if (sz < 0)
            {
                break;
            }
        }

        fclose(fptr);

        return remaining == 0 ? 0 : -1;
    }

private:
    std::shared_ptr<std::vector<Token>> m_headerTokens;
    std::vector<std::shared_ptr<std::vector<Token>>> m_messages;
    std::unique_ptr<char[]> m_buffer;
    std::uint64_t m_length;
    int m_id;

    int decodeIr()
    {
        using namespace uk::co::real_logic::sbe::ir::generated;

        FrameCodec frame;
        std::uint64_t offset = 0;
        char tmp[256];

        frame.wrapForDecode(m_buffer.get(), offset, frame.sbeBlockLength(), frame.sbeSchemaVersion(), m_length);

        frame.packageNameGet(tmp, sizeof(tmp));

        if (frame.irVersion() != 0)
        {
            return -1;
        }

        frame.namespaceNameGet(tmp, sizeof(tmp));
        frame.semanticVersionGet(tmp, sizeof(tmp));

        offset += frame.encodedLength();

        m_headerTokens.reset(new std::vector<Token>());

        std::uint64_t headerLength = readHeader(offset);

        m_id = frame.irId();

        offset += headerLength;

        while (offset < m_length)
        {
            offset += readMessage(offset);
        }

        return 0;
    }

    std::uint64_t decodeAndAddToken(std::shared_ptr<std::vector<Token>>& tokens, std::uint64_t offset)
    {
        using namespace uk::co::real_logic::sbe::ir::generated;

        TokenCodec tokenCodec;
        tokenCodec.wrapForDecode(m_buffer.get(), offset, tokenCodec.sbeBlockLength(), tokenCodec.sbeSchemaVersion(), m_length);

        Signal signal = static_cast<Signal>(tokenCodec.signal());
        PrimitiveType type = static_cast<PrimitiveType>(tokenCodec.primitiveType());
        Presence presence = static_cast<Presence>(tokenCodec.presence());
        ByteOrder byteOrder = static_cast<ByteOrder>(tokenCodec.byteOrder());
        std::int32_t tokenOffset = tokenCodec.tokenOffset();
        std::int32_t tokenSize = tokenCodec.tokenSize();
        std::int32_t id = tokenCodec.fieldId();
        std::int32_t version = tokenCodec.tokenVersion();
        std::int32_t componentTokenCount = tokenCodec.componentTokenCount();
        std::int32_t arrayCapacity = tokenCodec.arrayCapacity();
        char tmpBuffer[256];
        std::uint64_t tmpLen = 0;

        tmpLen = tokenCodec.nameGet(tmpBuffer, sizeof(tmpBuffer));
        std::string name(tmpBuffer, static_cast<std::size_t>(tmpLen));

        tmpLen = tokenCodec.constValueGet(tmpBuffer, sizeof(tmpBuffer));
        PrimitiveValue constValue(type, tmpLen, tmpBuffer);

        tmpLen = tokenCodec.lsbValueGet(tmpBuffer, sizeof(tmpBuffer));
        PrimitiveValue lsbValue(type, tmpLen, tmpBuffer);

        tmpLen = tokenCodec.msbValueGet(tmpBuffer, sizeof(tmpBuffer));
        PrimitiveValue msbValue(type, tmpLen, tmpBuffer);

        tmpLen = tokenCodec.minValueGet(tmpBuffer, sizeof(tmpBuffer));
        PrimitiveValue minValue(type, tmpLen, tmpBuffer);

        tmpLen = tokenCodec.maxValueGet(tmpBuffer, sizeof(tmpBuffer));
        PrimitiveValue maxValue(type, tmpLen, tmpBuffer);

        tmpLen = tokenCodec.nullValueGet(tmpBuffer, sizeof(tmpBuffer));
        PrimitiveValue nullValue(type, tmpLen, tmpBuffer);

        tmpLen = tokenCodec.characterEncodingGet(tmpBuffer, sizeof(tmpBuffer));
        std::string characterEncoding(tmpBuffer, static_cast<size_t>(tmpLen));

        tmpLen = tokenCodec.epochGet(tmpBuffer, sizeof(tmpBuffer));
        std::string epoch(tmpBuffer, static_cast<size_t>(tmpLen));

        tmpLen = tokenCodec.timeUnitGet(tmpBuffer, sizeof(tmpBuffer));
        std::string timeUnit(tmpBuffer, static_cast<size_t>(tmpLen));

        tmpLen = tokenCodec.semanticTypeGet(tmpBuffer, sizeof(tmpBuffer));
        std::string semanticType(tmpBuffer, static_cast<size_t>(tmpLen));

        tmpLen = tokenCodec.descriptionGet(tmpBuffer, sizeof(tmpBuffer));
        std::string description(tmpBuffer, static_cast<size_t>(tmpLen));

        tmpLen = tokenCodec.referencedNameGet(tmpBuffer, sizeof(tmpBuffer));
        std::string referencedName(tmpBuffer, static_cast<size_t>(tmpLen));

        Encoding encoding(
            type, presence, byteOrder, minValue, maxValue, nullValue,
            constValue, lsbValue, msbValue,
            characterEncoding, epoch, timeUnit, semanticType);

        Token token(tokenOffset, id, version, tokenSize, componentTokenCount, arrayCapacity, signal, name, description, encoding);

        tokens->push_back(token);

        return tokenCodec.encodedLength();
    }

    std::uint64_t readHeader(std::uint64_t offset)
    {
        std::uint64_t size = 0;

        while (offset + size < m_length)
        {
            size += decodeAndAddToken(m_headerTokens, offset + size);

            Token& token = m_headerTokens->back();

            if (token.signal() == Signal::END_COMPOSITE)
            {
                break;
            }
        }

        return size;
    }

    std::uint64_t readMessage(std::uint64_t offset)
    {
        std::uint64_t size = 0;

        std::shared_ptr<std::vector<Token>> tokensForMessage(new std::vector<Token>());

        while (offset + size < m_length)
        {
            size += decodeAndAddToken(tokensForMessage, offset + size);

            Token& token = tokensForMessage->back();

            if (token.signal() == Signal::END_MESSAGE)
            {
                break;
            }
        }

        m_messages.push_back(tokensForMessage);

        return size;
    }
};

}}

#endif
