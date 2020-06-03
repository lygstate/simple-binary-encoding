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
package uk.co.real_logic.sbe.generation.cpp;

import org.agrona.Strings;
import org.agrona.Verify;
import org.agrona.generation.OutputManager;
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.generation.CodeGenerator;
import uk.co.real_logic.sbe.generation.Generators;
import uk.co.real_logic.sbe.ir.Encoding;
import uk.co.real_logic.sbe.ir.GenerationUtil;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.ir.Signal;
import uk.co.real_logic.sbe.ir.Token;

import java.io.IOException;
import java.io.Writer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

import static uk.co.real_logic.sbe.generation.Generators.toLowerFirstChar;
import static uk.co.real_logic.sbe.generation.Generators.toUpperFirstChar;
import static uk.co.real_logic.sbe.generation.cpp.CppUtil.*;
import static uk.co.real_logic.sbe.generation.c.CUtil.generateSbecHeader;
import static uk.co.real_logic.sbe.ir.GenerationUtil.*;

/**
 * Codec generator for the C++11 programming language with conditional complication for C++14 and C++17 features.
 */
@SuppressWarnings("MethodLength")
public class CppGenerator implements CodeGenerator
{
    private static final String BASE_INDENT = "";
    private static final String INDENT = "    ";

    private final Ir ir;
    private final OutputManager outputManager;

    public CppGenerator(final Ir ir, final OutputManager outputManager)
    {
        Verify.notNull(ir, "ir");
        Verify.notNull(outputManager, "outputManager");

        this.ir = ir;
        this.outputManager = outputManager;
    }

    public static class Formatter
    {
        private final StringBuilder sb;
        private final Writer out;

        Formatter(final StringBuilder sb)
        {
            this.sb = sb;
            this.out = null;
        }

        Formatter(final Writer out)
        {
            this.sb = null;
            this.out = out;
        }

        public void format(final String format, final Object... args)
        {
            final String formattedString = String.format(format, args);
            if (this.sb != null)
            {
                this.sb.append(formattedString);
            }
            if (this.out != null)
            {
                try
                {
                    this.out.write(formattedString);
                }
                catch (final IOException ex)
                {
                }
            }
        }
    }

    public static Formatter formatter(final StringBuilder sb)
    {
        return new Formatter(sb);
    }

    public static Formatter formatter(final Writer out)
    {
        return new Formatter(out);
    }

    public void generateTypeStubs(final StringBuilder out)
    {
        for (final List<Token> tokens : GenerationUtil.sortTypes(ir))
        {
            switch (tokens.get(0).signal())
            {
                case BEGIN_ENUM:
                    generateEnum(out, tokens);
                    break;

                case BEGIN_SET:
                    generateChoiceSet(out, tokens);
                    break;

                case BEGIN_COMPOSITE:
                    generateComposite(out, tokens);
                    break;

                default:
                    break;
            }
        }
    }

    public void generate() throws IOException
    {
        final String filename = formatScope(ir.namespaces()) + "_cpp";
        try (Writer out = outputManager.createOutput(filename))
        {
            final StringBuilder sb = new StringBuilder();
            sb.append(generateFileHeader(ir.namespaces(), filename));
            generateTypeStubs(sb);

            for (final List<Token> tokens : ir.messages())
            {

                final ArrayList<MessageItem> messageItemList = new ArrayList<MessageItem>();
                GenerationUtil.getMessageItemList(messageItemList, null, tokens);
                for (int i = 0; i < messageItemList.size(); i += 1)
                {
                    generateMessageItem(sb, messageItemList.get(i), BASE_INDENT);
                }
            }
            sb.append(CppUtil.closingBraces(ir.namespaces().length)).append("#endif\n");
            out.append(sb);
        }
    }

    private void generateMessageItem(final StringBuilder sb, final MessageItem messageItem, final String indent)
    {
        final Token rootToken = messageItem.rootToken;

        sb.append(CppUtil.openingBraces(generateMessageItemNamespace(messageItem)));

        final StringBuilder sbClassType = new StringBuilder();

        if (rootToken.signal() == Signal.BEGIN_GROUP)
        {
            generateGroupClassHeader(sbClassType, messageItem, indent);
        }
        else
        {
            sbClassType.append(generateClassDeclaration(messageItemClassName(messageItem)));
            sbClassType.append(generateMessageFlyweightCode(messageItemClassName(messageItem), messageItem.rootToken));
        }
        final StringBuilder sbLengthType = new StringBuilder();

        formatter(sbLengthType).format(
            indent + "struct %1$s\n" +
            indent + "{\n",
            messageItemClassName(messageItem) + "LengthParam"
        );

        generateProperties(sbLengthType, sbClassType, messageItem, indent);

        sbLengthType.append(indent).append("};\n\n");
        sbClassType.append(indent).append("};\n\n");

        if (!messageItem.isConst())
        {
            sb.append(sbLengthType);
        }
        sb.append(sbClassType);

        sb.append(CppUtil.closingBraces(generateMessageItemNamespace(messageItem).size()));
    }

    private static String messageItemClassName(final MessageItem messageItem)
    {
        final String formattedName = formatClassName(messageItem.rootToken.name());
        return formattedName;
    }


    private static String messageItemFullClassName(final MessageItem messageItem)
    {
        final ArrayList<String> names = generateMessageItemNamespace(messageItem);
        names.add(messageItemClassName(messageItem));
        return String.join("::", names);
    }

    private static ArrayList<String> generateMessageItemNamespace(final MessageItem messageItem)
    {
        final ArrayList<String> result = new ArrayList<String>();
        for (final String name : messageItem.classPath)
        {
            result.add(formatClassName(name) + "Groups");
        }
        return result;
    }

    private static void generateGroupClassHeader(
        final StringBuilder sb,
        final MessageItem messageItem,
        final String indent)
    {
        final String dimensionsClassName = formatClassName(messageItem.tokens.get(1).name());
        final int dimensionHeaderLength = messageItem.tokens.get(1).encodedLength();
        final int blockLength = messageItem.rootToken.encodedLength();
        final Token blockLengthToken = Generators.findFirst("blockLength", messageItem.tokens, 0);
        final Token numInGroupToken = Generators.findFirst("numInGroup", messageItem.tokens, 0);
        final String cppTypeBlockLength = cppTypeName(blockLengthToken.encoding().primitiveType());
        final String cppTypeNumInGroup = cppTypeName(numInGroupToken.encoding().primitiveType());

        new Formatter(sb).format("\n" +
            indent + "class %1$s\n" +
            indent + "{\n" +
            indent + "private:\n" +
            indent + "    char *m_buffer;\n" +
            indent + "    std::uint64_t m_bufferLength;\n" +
            indent + "    std::uint64_t m_initialPosition;\n" +
            indent + "    std::uint64_t *m_positionPtr;\n" +
            indent + "    std::uint64_t m_blockLength;\n" +
            indent + "    std::uint64_t m_count;\n" +
            indent + "    std::uint64_t m_index;\n" +
            indent + "    std::uint64_t m_offset;\n" +
            indent + "    std::uint64_t m_actingVersion;\n\n" +

            indent + "    SBE_NODISCARD std::uint64_t *sbePositionPtr() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return m_positionPtr;\n" +
            indent + "    }\n\n" +

            indent + "public:\n",
            messageItemClassName(messageItem));

        new Formatter(sb).format(
            indent + "    inline void wrapForDecode(\n" +
            indent + "        char *buffer,\n" +
            indent + "        std::uint64_t *pos,\n" +
            indent + "        const std::uint64_t actingVersion,\n" +
            indent + "        const std::uint64_t bufferLength)\n" +
            indent + "    {\n" +
            indent + "        %2$s dimensions(buffer, *pos, bufferLength, actingVersion);\n" +
            indent + "        m_buffer = buffer;\n" +
            indent + "        m_bufferLength = bufferLength;\n" +
            indent + "        m_blockLength = dimensions.blockLength();\n" +
            indent + "        m_count = dimensions.numInGroup();\n" +
            indent + "        m_index = 0;\n" +
            indent + "        m_actingVersion = actingVersion;\n" +
            indent + "        m_initialPosition = *pos;\n" +
            indent + "        m_positionPtr = pos;\n" +
            indent + "        *m_positionPtr = *m_positionPtr + %1$d;\n" +
            indent + "    }\n",
            dimensionHeaderLength, dimensionsClassName);

        final long minCount = numInGroupToken.encoding().applicableMinValue().longValue();
        final String minCheck = minCount > 0 ? "count < " + minCount + " || " : "";

        new Formatter(sb).format("\n" +
            indent + "    inline void wrapForEncode(\n" +
            indent + "        char *buffer,\n" +
            indent + "        const %3$s count,\n" +
            indent + "        std::uint64_t *pos,\n" +
            indent + "        const std::uint64_t actingVersion,\n" +
            indent + "        const std::uint64_t bufferLength)\n" +
            indent + "    {\n" +
            indent + "        if (%5$scount > %6$d)\n" +
            indent + "        {\n" +
            indent + "            sbe_throw_errnum(E110, \"count outside of allowed range [E110]\");\n" +
            indent + "            return ;\n" +
            indent + "        }\n" +
            indent + "        m_buffer = buffer;\n" +
            indent + "        m_bufferLength = bufferLength;\n" +
            indent + "        %7$s dimensions(buffer, *pos, bufferLength, actingVersion);\n" +
            indent + "        dimensions.blockLength((%1$s)%2$d);\n" +
            indent + "        dimensions.numInGroup((%3$s)count);\n" +
            indent + "        m_index = 0;\n" +
            indent + "        m_count = count;\n" +
            indent + "        m_blockLength = %2$d;\n" +
            indent + "        m_actingVersion = actingVersion;\n" +
            indent + "        m_initialPosition = *pos;\n" +
            indent + "        m_positionPtr = pos;\n" +
            indent + "        *m_positionPtr = *m_positionPtr + %4$d;\n" +
            indent + "    }\n",
            cppTypeBlockLength,
            blockLength,
            cppTypeNumInGroup,
            dimensionHeaderLength,
            minCheck,
            numInGroupToken.encoding().applicableMaxValue().longValue(),
            dimensionsClassName);

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t sbeHeaderSize() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %1$d;\n" +
            indent + "    }\n\n" +

            indent + "    static SBE_CONSTEXPR std::uint64_t sbeBlockLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n\n" +

            indent + "    SBE_NODISCARD std::uint64_t sbePosition() const SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return *m_positionPtr;\n" +
            indent + "    }\n\n" +

            indent + "    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)\n" +
            indent + "    std::uint64_t sbeCheckPosition(const std::uint64_t position)\n" +
            indent + "    {\n" +
            indent + "        if (SBE_BOUNDS_CHECK_EXPECT((position > m_bufferLength), false))\n" +
            indent + "        {\n" +
            indent + "            sbe_throw_errnum(E100, \"buffer too short [E100]\");\n" +
            indent + "            return UINT64_MAX;\n" +
            indent + "        }\n" +
            indent + "        return position;\n" +
            indent + "    }\n\n" +

            indent + "    void sbePosition(const std::uint64_t position)\n" +
            indent + "    {\n" +
            indent + "        *m_positionPtr = sbeCheckPosition(position);\n" +
            indent + "    }\n\n" +

            indent + "    SBE_NODISCARD inline std::uint64_t count() const SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return m_count;\n" +
            indent + "    }\n\n" +

            indent + "    SBE_NODISCARD inline bool hasNext() const SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return m_index < m_count;\n" +
            indent + "    }\n\n" +

            indent + "    inline %3$s &next()\n" +
            indent + "    {\n" +
            indent + "        if (m_index >= m_count)\n" +
            indent + "        {\n" +
            indent + "            sbe_throw_errnum(E108, \"index >= count [E108]\");\n" +
            indent + "            return *this;\n" +
            indent + "        }\n" +
            indent + "        m_offset = *m_positionPtr;\n" +
            indent + "        if (SBE_BOUNDS_CHECK_EXPECT(((m_offset + m_blockLength) > m_bufferLength), false))\n" +
            indent + "        {\n" +
            indent + "            sbe_throw_errnum(E108, \"buffer too short for next group index [E108]\");\n" +
            indent + "            return *this;\n" +
            indent + "        }\n" +
            indent + "        *m_positionPtr = m_offset + m_blockLength;\n" +
            indent + "        ++m_index;\n\n" +

            indent + "        return *this;\n" +
            indent + "    }\n",
            dimensionHeaderLength,
            blockLength,
            messageItemFullClassName(messageItem));

        sb.append("\n")
            .append(indent).append("    inline std::uint64_t resetCountToIndex() SBE_NOEXCEPT\n")
            .append(indent).append("    {\n")
            .append(indent).append("        m_count = m_index;\n")
            .append(indent).append("        ").append(dimensionsClassName)
            .append(" dimensions(m_buffer, m_initialPosition, m_bufferLength, m_actingVersion);\n")
            .append(indent).append("        dimensions.numInGroup((").append(cppTypeNumInGroup).append(")m_count);\n")
            .append(indent).append("        return m_count;\n")
            .append(indent).append("    }\n");

        sb.append("\n")
            .append(indent).append("#if __cplusplus < 201103L\n")
            .append(indent).append("    template<class Func> inline void forEach(Func& func)\n")
            .append(indent).append("    {\n")
            .append(indent).append("        while (hasNext())\n")
            .append(indent).append("        {\n")
            .append(indent).append("            next();\n")
            .append(indent).append("            func(*this);\n")
            .append(indent).append("        }\n")
            .append(indent).append("    }\n\n")

            .append(indent).append("#else\n")
            .append(indent).append("    template<class Func> inline void forEach(Func&& func)\n")
            .append(indent).append("    {\n")
            .append(indent).append("        while (hasNext())\n")
            .append(indent).append("        {\n")
            .append(indent).append("            next();\n")
            .append(indent).append("            func(*this);\n")
            .append(indent).append("        }\n")
            .append(indent).append("    }\n\n")

            .append(indent).append("#endif\n");
    }

    private static void generateGroupProperty(
        final StringBuilder sb,
        final String className,
        final String propertyName,
        final Token token,
        final String cppTypeForNumInGroup,
        final String indent)
    {

        new Formatter(sb).format("\n" +
            indent + "private:\n" +
            indent + "    %1$s m_%2$s;\n\n" +

            indent + "public:\n",
            className,
            propertyName);

        new Formatter(sb).format(
            indent + "    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t %1$sId() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            token.id());

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD inline %1$s &%2$s()\n" +
            indent + "    {\n" +
            indent + "        m_%2$s.wrapForDecode(m_buffer, sbePositionPtr(), m_actingVersion, m_bufferLength);\n" +
            indent + "        return m_%2$s;\n" +
            indent + "    }\n",
            className,
            propertyName);

        new Formatter(sb).format("\n" +
            indent + "    %1$s &%2$sCount(const %3$s count)\n" +
            indent + "    {\n" +
            indent + "        m_%2$s.wrapForEncode(" +
            "m_buffer, count, sbePositionPtr(), m_actingVersion, m_bufferLength);\n" +
            indent + "        return m_%2$s;\n" +
            indent + "    }\n",
            className,
            propertyName,
            cppTypeForNumInGroup);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t %1$sSinceVersion() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n\n" +

            indent + "    SBE_NODISCARD bool %1$sInActingVersion() const SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return m_actingVersion >= %1$sSinceVersion();\n" +
            indent + "    }\n",
            propertyName,
            token.version());
    }

    private void generateVarData(
        final StringBuilder sb, final String className, final List<Token> tokens, final String indent)
    {
        for (int i = 0, size = tokens.size(); i < size;)
        {
            final Token token = tokens.get(i);
            if (token.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + token);
            }

            final String propertyName = toUpperFirstChar(token.name());
            final Token lengthToken = Generators.findFirst("length", tokens, i);
            final Token varDataToken = Generators.findFirst("varData", tokens, i);
            final String characterEncoding = varDataToken.encoding().characterEncoding();
            final int lengthOfLengthField = lengthToken.encodedLength();
            final String lengthCppType = cppTypeName(lengthToken.encoding().primitiveType());
            final String lengthByteOrderStr = formatByteOrderEncoding(
                lengthToken.encoding().byteOrder(), lengthToken.encoding().primitiveType());

            generateFieldMetaAttributeMethod(sb, token, indent);

            generateVarDataDescriptors(
                sb, token, propertyName, characterEncoding, lengthToken, lengthOfLengthField, lengthCppType, indent);

            new Formatter(sb).format("\n" +
                indent + "    std::uint64_t skip%1$s()\n" +
                indent + "    {\n" +
                "%2$s" +
                indent + "        std::uint64_t lengthOfLengthField = %3$d;\n" +
                indent + "        std::uint64_t lengthPosition = sbePosition();\n" +
                indent + "        %5$s lengthFieldValue;\n" +
                indent + "        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(%5$s));\n" +
                indent + "        std::uint64_t dataLength = %4$s(lengthFieldValue);\n" +
                indent + "        sbePosition(lengthPosition + lengthOfLengthField + dataLength);\n" +
                indent + "        return dataLength;\n" +
                indent + "    }\n",
                propertyName,
                generateArrayFieldNotPresentCondition(token.version(), indent),
                lengthOfLengthField,
                lengthByteOrderStr,
                lengthCppType);

            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD const char *%1$s()\n" +
                indent + "    {\n" +
                "%2$s" +
                indent + "        %4$s lengthFieldValue;\n" +
                indent + "        std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(%4$s));\n" +
                indent + "        const char *fieldPtr = m_buffer + sbePosition() + %3$d;\n" +
                indent + "        sbePosition(sbePosition() + %3$d + %5$s(lengthFieldValue));\n" +
                indent + "        return fieldPtr;\n" +
                indent + "    }\n",
                formatPropertyName(propertyName),
                generateTypeFieldNotPresentCondition(token.version(), indent),
                lengthOfLengthField,
                lengthCppType,
                lengthByteOrderStr);

            new Formatter(sb).format("\n" +
                indent + "    std::uint64_t get%1$s(char *dst, const std::uint64_t length)\n" +
                indent + "    {\n" +
                "%2$s" +
                indent + "        std::uint64_t lengthOfLengthField = %3$d;\n" +
                indent + "        std::uint64_t lengthPosition = sbePosition();\n" +
                indent + "        sbePosition(lengthPosition + lengthOfLengthField);\n" +
                indent + "        %5$s lengthFieldValue;\n" +
                indent + "        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(%5$s));\n" +
                indent + "        std::uint64_t dataLength = %4$s(lengthFieldValue);\n" +
                indent + "        std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;\n" +
                indent + "        std::uint64_t pos = sbePosition();\n" +
                indent + "        sbePosition(pos + dataLength);\n" +
                indent + "        std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));\n" +
                indent + "        return bytesToCopy;\n" +
                indent + "    }\n",
                propertyName,
                generateArrayFieldNotPresentCondition(token.version(), indent),
                lengthOfLengthField,
                lengthByteOrderStr,
                lengthCppType);

            new Formatter(sb).format("\n" +
                indent + "    %5$s &put%1$s(const char *src, const %3$s length)\n" +
                indent + "    {\n" +
                indent + "        std::uint64_t lengthOfLengthField = %2$d;\n" +
                indent + "        std::uint64_t lengthPosition = sbePosition();\n" +
                indent + "        %3$s lengthFieldValue = %4$s(length);\n" +
                indent + "        sbePosition(lengthPosition + lengthOfLengthField);\n" +
                indent + "        std::memcpy(m_buffer + lengthPosition, &lengthFieldValue, sizeof(%3$s));\n" +
                indent + "        if (length != %3$s(0))\n" +
                indent + "        {\n" +
                indent + "            std::uint64_t pos = sbePosition();\n" +
                indent + "            sbePosition(pos + length);\n" +
                indent + "            std::memcpy(m_buffer + pos, src, length);\n" +
                indent + "        }\n" +
                indent + "        return *this;\n" +
                indent + "    }\n",
                propertyName,
                lengthOfLengthField,
                lengthCppType,
                lengthByteOrderStr,
                className);

            new Formatter(sb).format("\n" +
                indent + "    std::string get%1$sAsString()\n" +
                indent + "    {\n" +
                "%2$s" +
                indent + "        std::uint64_t lengthOfLengthField = %3$d;\n" +
                indent + "        std::uint64_t lengthPosition = sbePosition();\n" +
                indent + "        sbePosition(lengthPosition + lengthOfLengthField);\n" +
                indent + "        %5$s lengthFieldValue;\n" +
                indent + "        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(%5$s));\n" +
                indent + "        std::uint64_t dataLength = %4$s(lengthFieldValue);\n" +
                indent + "        std::uint64_t pos = sbePosition();\n" +
                indent + "        const std::string result(m_buffer + pos, static_cast<std::size_t>(dataLength));\n" +
                indent + "        sbePosition(pos + dataLength);\n" +
                indent + "        return result;\n" +
                indent + "    }\n",
                propertyName,
                generateStringNotPresentCondition(token.version(), indent),
                lengthOfLengthField,
                lengthByteOrderStr,
                lengthCppType);

            generateJsonEscapedStringGetter(sb, token, indent, propertyName);

            new Formatter(sb).format("\n" +
                indent + "    #if __cplusplus >= 201703L\n" +
                indent + "    std::string_view get%1$sAsStringView()\n" +
                indent + "    {\n" +
                "%2$s" +
                indent + "        std::uint64_t lengthOfLengthField = %3$d;\n" +
                indent + "        std::uint64_t lengthPosition = sbePosition();\n" +
                indent + "        sbePosition(lengthPosition + lengthOfLengthField);\n" +
                indent + "        %5$s lengthFieldValue;\n" +
                indent + "        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(%5$s));\n" +
                indent + "        std::uint64_t dataLength = %4$s(lengthFieldValue);\n" +
                indent + "        std::uint64_t pos = sbePosition();\n" +
                indent + "        const std::string_view result(m_buffer + pos, dataLength);\n" +
                indent + "        sbePosition(pos + dataLength);\n" +
                indent + "        return result;\n" +
                indent + "    }\n" +
                indent + "    #endif\n",
                propertyName,
                generateStringViewNotPresentCondition(token.version(), indent),
                lengthOfLengthField,
                lengthByteOrderStr,
                lengthCppType);

            new Formatter(sb).format("\n" +
                indent + "    %1$s &put%2$s(const std::string& str)\n" +
                indent + "    {\n" +
                indent + "        if (str.length() > %4$d)\n" +
                indent + "        {\n" +
                indent + "            sbe_throw_errnum(E109, \"std::string too long for length type [E109]\");\n" +
                indent + "            return *this;\n" +
                indent + "        }\n" +
                indent + "        return put%2$s(str.data(), static_cast<%3$s>(str.length()));\n" +
                indent + "    }\n",
                className,
                propertyName,
                lengthCppType,
                lengthToken.encoding().applicableMaxValue().longValue());

            new Formatter(sb).format("\n" +
                indent + "    #if __cplusplus >= 201703L\n" +
                indent + "    %1$s &put%2$s(const std::string_view str)\n" +
                indent + "    {\n" +
                indent + "        if (str.length() > %4$d)\n" +
                indent + "        {\n" +
                indent + "            sbe_throw_errnum(E109, \"std::string too long for length type [E109]\");\n" +
                indent + "            return *this;\n" +
                indent + "        }\n" +
                indent + "        return put%2$s(str.data(), static_cast<%3$s>(str.length()));\n" +
                indent + "    }\n" +
                indent + "    #endif\n",
                className,
                propertyName,
                lengthCppType,
                lengthToken.encoding().applicableMaxValue().longValue());

            i += token.componentTokenCount();
        }
    }

    private void generateVarDataDescriptors(
        final StringBuilder sb,
        final Token token,
        final String propertyName,
        final String characterEncoding,
        final Token lengthToken,
        final Integer sizeOfLengthField,
        final String lengthCppType,
        final String indent)
    {
        new Formatter(sb).format("\n" +
            indent + "    static const char *%1$sCharacterEncoding() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return \"%2$s\";\n" +
            indent + "    }\n",
            toLowerFirstChar(propertyName),
            characterEncoding);

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t %1$sSinceVersion() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n\n" +

            indent + "    bool %1$sInActingVersion() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return m_actingVersion >= %1$sSinceVersion();\n" +
            indent + "    }\n\n" +

            indent + "    static SBE_CONSTEXPR std::uint16_t %1$sId() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %3$d;\n" +
            indent + "    }\n",
            toLowerFirstChar(propertyName),
            token.version(),
            token.id());

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t %sHeaderLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %d;\n" +
            indent + "    }\n",
            toLowerFirstChar(propertyName),
            sizeOfLengthField);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD %4$s %1$sLength() const\n" +
            indent + "    {\n" +
            "%2$s" +
            indent + "        %4$s length;\n" +
            indent + "        std::memcpy(&length, m_buffer + sbePosition(), sizeof(%4$s));\n" +
            indent + "        return %3$s(length);\n" +
            indent + "    }\n",
            toLowerFirstChar(propertyName),
            generateArrayFieldNotPresentCondition(token.version(), BASE_INDENT),
            formatByteOrderEncoding(lengthToken.encoding().byteOrder(), lengthToken.encoding().primitiveType()),
            lengthCppType);
    }

    private void generateChoiceSet(final StringBuilder out, final List<Token> tokens)
    {
        final String bitSetName = formatClassName(tokens.get(0).applicableTypeName());

        {
            out.append(generateClassDeclaration(bitSetName));
            out.append(generateFixedFlyweightCode(bitSetName, tokens.get(0).encodedLength()));

            new Formatter(out).format("\n" +
                "    %1$s &clear()\n" +
                "    {\n" +
                "        %2$s zero = 0;\n" +
                "        std::memcpy(m_buffer + m_offset, &zero, sizeof(%2$s));\n" +
                "        return *this;\n" +
                "    }\n",
                bitSetName,
                cppTypeName(tokens.get(0).encoding().primitiveType()));

            new Formatter(out).format("\n" +
                "    SBE_NODISCARD bool isEmpty() const\n" +
                "    {\n" +
                "        %1$s val;\n" +
                "        std::memcpy(&val, m_buffer + m_offset, sizeof(%1$s));\n" +
                "        return 0 == val;\n" +
                "    }\n",
                cppTypeName(tokens.get(0).encoding().primitiveType()));

            new Formatter(out).format("\n" +
                "    SBE_NODISCARD %1$s rawValue() const\n" +
                "    {\n" +
                "        %1$s val;\n" +
                "        std::memcpy(&val, m_buffer + m_offset, sizeof(%1$s));\n" +
                "        return val;\n" +
                "    }\n",
                cppTypeName(tokens.get(0).encoding().primitiveType()));

            new Formatter(out).format("\n" +
                "    %1$s &rawValue(%2$s value)\n" +
                "    {\n" +
                "        std::memcpy(m_buffer + m_offset, &value, sizeof(%2$s));\n" +
                "        return *this;\n" +
                "    }\n",
                bitSetName,
                cppTypeName(tokens.get(0).encoding().primitiveType()));

            out.append(generateChoices(bitSetName, tokens.subList(1, tokens.size() - 1)));
            out.append(generateChoicesDisplay(bitSetName, tokens.subList(1, tokens.size() - 1)));
            out.append("};\n\n");
        }
    }

    private void generateEnum(final StringBuilder out, final List<Token> tokens)
    {
        final Token enumToken = tokens.get(0);
        final String enumName = formatClassName(tokens.get(0).applicableTypeName());

        {
            out.append(generateEnumDeclaration(enumName));

            out.append(generateEnumValues(tokens.subList(1, tokens.size() - 1), enumToken));

            out.append(generateEnumLookupMethod(tokens.subList(1, tokens.size() - 1), enumToken));

            out.append(generateEnumDisplay(tokens.subList(1, tokens.size() - 1), enumToken));

            out.append("};\n\n");
        }
    }

    private void generateComposite(final StringBuilder out, final List<Token> tokens)
    {
        final String compositeName = formatClassName(tokens.get(0).applicableTypeName());
        {
            out.append(generateClassDeclaration(compositeName));
            out.append(generateFixedFlyweightCode(compositeName, tokens.get(0).encodedLength()));

            out.append(generateCompositePropertyElements(
                compositeName, tokens.subList(1, tokens.size() - 1), BASE_INDENT));

            out.append(generateCompositeDisplay(
                tokens.get(0).applicableTypeName(), tokens.subList(1, tokens.size() - 1)));

            out.append("};\n\n");
        }
    }

    private static CharSequence generateChoiceNotPresentCondition(final int sinceVersion)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            "        if (m_actingVersion < %1$d)\n" +
            "        {\n" +
            "            return false;\n" +
            "        }\n\n",
            sinceVersion);
    }

    private CharSequence generateChoices(final String bitsetClassName, final List<Token> tokens)
    {
        final StringBuilder sb = new StringBuilder();

        tokens
            .stream()
            .filter((token) -> token.signal() == Signal.CHOICE)
            .forEach((token) ->
            {
                final String choiceName = formatPropertyName(token.name());
                final String typeName = cppTypeName(token.encoding().primitiveType());
                final String choiceBitPosition = token.encoding().constValue().toString();
                final String byteOrderStr = formatByteOrderEncoding(
                    token.encoding().byteOrder(), token.encoding().primitiveType());

                new Formatter(sb).format("\n" +
                    "    static bool %1$s(const %2$s bits)\n" +
                    "    {\n" +
                    "        return (bits & (1u << %3$su)) != 0;\n" +
                    "    }\n",
                    choiceName,
                    typeName,
                    choiceBitPosition);

                new Formatter(sb).format("\n" +
                    "    static %2$s %1$s(const %2$s bits, const bool value)\n" +
                    "    {\n" +
                    "        return value ?" +
                    " static_cast<%2$s>(bits | (1u << %3$su)) : static_cast<%2$s>(bits & ~(1u << %3$su));\n" +
                    "    }\n",
                    choiceName,
                    typeName,
                    choiceBitPosition);

                new Formatter(sb).format("\n" +
                    "    SBE_NODISCARD bool %1$s() const\n" +
                    "    {\n" +
                    "%2$s" +
                    "        %4$s val;\n" +
                    "        std::memcpy(&val, m_buffer + m_offset, sizeof(%4$s));\n" +
                    "        return (%3$s(val) & (1u << %5$su)) != 0;\n" +
                    "    }\n",
                    choiceName,
                    generateChoiceNotPresentCondition(token.version()),
                    byteOrderStr,
                    typeName,
                    choiceBitPosition);

                new Formatter(sb).format("\n" +
                    "    %1$s &%2$s(const bool value)\n" +
                    "    {\n" +
                    "        %3$s bits;\n" +
                    "        std::memcpy(&bits, m_buffer + m_offset, sizeof(%3$s));\n" +
                    "        bits = %4$s(value ?" +
                    " static_cast<%3$s>(%4$s(bits) | (1u << %5$su)) " +
                    ": static_cast<%3$s>(%4$s(bits) & ~(1u << %5$su)));\n" +
                    "        std::memcpy(m_buffer + m_offset, &bits, sizeof(%3$s));\n" +
                    "        return *this;\n" +
                    "    }\n",
                    bitsetClassName,
                    choiceName,
                    typeName,
                    byteOrderStr,
                    choiceBitPosition);
            });

        return sb;
    }

    private CharSequence generateEnumValues(final List<Token> tokens, final Token encodingToken)
    {
        final StringBuilder sb = new StringBuilder();
        final Encoding encoding = encodingToken.encoding();

        sb.append(
            "    enum Value\n" +
            "    {\n");

        for (final Token token : tokens)
        {
            final CharSequence constVal = generateLiteral(
                token.encoding().primitiveType(), token.encoding().constValue().toString());
            sb.append("        ").append(token.name()).append(" = ").append(constVal).append(",\n");
        }

        sb.append(String.format(
            "        NULL_VALUE = %1$s",
            generateLiteral(encoding.primitiveType(), encoding.applicableNullValue().toString())));

        sb.append("\n    };\n\n");

        return sb;
    }

    private static CharSequence generateEnumLookupMethod(final List<Token> tokens, final Token encodingToken)
    {
        final String enumName = formatClassName(encodingToken.applicableTypeName());
        final StringBuilder sb = new StringBuilder();

        new Formatter(sb).format(
            "    static %1$s::Value get(const %2$s value)\n" +
            "    {\n" +
            "        switch (value)\n" +
            "        {\n",
            enumName,
            cppTypeName(tokens.get(0).encoding().primitiveType()));

        for (final Token token : tokens)
        {
            new Formatter(sb).format(
                "            case %1$s: return %2$s;\n",
                token.encoding().constValue().toString(),
                token.name());
        }

        new Formatter(sb).format(
            "            case %1$s: return NULL_VALUE;\n" +
            "        }\n\n" +

            "        sbe_throw_errnum(E103, \"unknown value for enum %2$s [E103]\");\n" +
            "        return NULL_VALUE;\n" +
            "    }\n",
            encodingToken.encoding().applicableNullValue().toString(),
            enumName);

        return sb;
    }

    private CharSequence generateFieldNotPresentCondition(
        final int sinceVersion, final Encoding encoding, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return %2$s;\n" +
            indent + "        }\n\n",
            sinceVersion,
            generateLiteral(encoding.primitiveType(), encoding.applicableNullValue().toString()));
    }

    private static CharSequence generateArrayFieldNotPresentCondition(final int sinceVersion, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return 0;\n" +
            indent + "        }\n\n",
            sinceVersion);
    }

    private static CharSequence generateStringNotPresentCondition(final int sinceVersion, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return std::string(\"\");\n" +
            indent + "        }\n\n",
            sinceVersion);
    }

    private static CharSequence generateStringViewNotPresentCondition(final int sinceVersion, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return std::string_view(\"\");\n" +
            indent + "        }\n\n",
            sinceVersion);
    }

    private static CharSequence generateTypeFieldNotPresentCondition(final int sinceVersion, final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return nullptr;\n" +
            indent + "        }\n\n",
            sinceVersion);
    }

    private static CharSequence generateFileHeader(
        final CharSequence[] namespaces,
        final String className)
    {
        final StringBuilder sb = new StringBuilder();
        sb.append(generateSbecHeader());

        sb.append(String.format(
            "#ifndef _%1$s_%2$s_H_\n" +
            "#define _%1$s_%2$s_H_\n\n",
            String.join("_", namespaces).toUpperCase(),
            className.toUpperCase()));

        sb.append("\nnamespace ");
        sb.append(String.join(" {\nnamespace ", namespaces));
        sb.append(" {\n\n");

        return sb;
    }

    private static CharSequence generateClassDeclaration(final String className)
    {
        return
            "class " + className + "\n" +
            "{\n";
    }

    private static CharSequence generateEnumDeclaration(final String name)
    {
        return "class " + name + "\n{\npublic:\n";
    }

    private CharSequence generateCompositePropertyElements(
        final String containingClassName, final List<Token> tokens, final String indent)
    {
        final StringBuilder sb = new StringBuilder();

        for (int i = 0; i < tokens.size();)
        {
            final Token fieldToken = tokens.get(i);
            final String propertyName = formatPropertyName(fieldToken.name());

            generateFieldMetaAttributeMethod(sb, fieldToken, indent);
            generateFieldCommonMethods(indent, sb, fieldToken, fieldToken, propertyName);

            switch (fieldToken.signal())
            {
                case ENCODING:
                    generatePrimitiveProperty(sb, containingClassName, propertyName, fieldToken, fieldToken, indent);
                    break;

                case BEGIN_ENUM:
                    generateEnumProperty(sb, containingClassName, fieldToken, propertyName, fieldToken, indent);
                    break;

                case BEGIN_SET:
                    generateBitsetProperty(sb, propertyName, fieldToken, indent);
                    break;

                case BEGIN_COMPOSITE:
                    generateCompositeProperty(sb, propertyName, fieldToken, indent);
                    break;

                default:
                    break;
            }

            i += tokens.get(i).componentTokenCount();
        }

        return sb;
    }

    private void generatePrimitiveProperty(
        final StringBuilder sb,
        final String containingClassName,
        final String propertyName,
        final Token propertyToken,
        final Token encodingToken,
        final String indent)
    {
        generatePrimitiveFieldMetaData(sb, propertyName, encodingToken, indent);

        if (encodingToken.isConstantEncoding())
        {
            generateConstPropertyMethods(sb, propertyName, encodingToken, indent);
        }
        else
        {
            generatePrimitivePropertyMethods(
                sb, containingClassName, propertyName, propertyToken, encodingToken, indent);
        }
    }

    private void generatePrimitivePropertyMethods(
        final StringBuilder sb,
        final String containingClassName,
        final String propertyName,
        final Token propertyToken,
        final Token encodingToken,
        final String indent)
    {
        final int arrayLength = encodingToken.arrayLength();
        if (arrayLength == 1)
        {
            generateSingleValueProperty(sb, containingClassName, propertyName, propertyToken, encodingToken, indent);
        }
        else if (arrayLength > 1)
        {
            generateArrayProperty(sb, containingClassName, propertyName, propertyToken, encodingToken, indent);
        }
    }

    private void generatePrimitiveFieldMetaData(
        final StringBuilder sb, final String propertyName, final Token token, final String indent)
    {
        final Encoding encoding = token.encoding();
        final PrimitiveType primitiveType = encoding.primitiveType();
        final String cppTypeName = cppTypeName(primitiveType);
        final CharSequence nullValueString = generateNullValueLiteral(primitiveType, encoding);

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR %1$s %2$sNullValue() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %3$s;\n" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            nullValueString);

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR %1$s %2$sMinValue() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %3$s;\n" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            generateLiteral(primitiveType, token.encoding().applicableMinValue().toString()));

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR %1$s %2$sMaxValue() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %3$s;\n" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            generateLiteral(primitiveType, token.encoding().applicableMaxValue().toString()));

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t %1$sEncodedLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            token.encoding().primitiveType().size() * token.arrayLength());
    }

    private CharSequence generateLoadValue(
        final PrimitiveType primitiveType,
        final String offsetStr,
        final ByteOrder byteOrder,
        final String indent)
    {
        final String cppTypeName = cppTypeName(primitiveType);
        final String byteOrderStr = formatByteOrderEncoding(byteOrder, primitiveType);
        final StringBuilder sb = new StringBuilder();

        if (primitiveType == PrimitiveType.FLOAT || primitiveType == PrimitiveType.DOUBLE)
        {
            final String stackUnion =
                primitiveType == PrimitiveType.FLOAT ? "union sbe_float_as_uint" : "union sbe_double_as_uint";

            new Formatter(sb).format(
                indent + "        %1$s val;\n" +
                indent + "        std::memcpy(&val, m_buffer + m_offset + %2$s, sizeof(%3$s));\n" +
                indent + "        val.uint_value = %4$s(val.uint_value);\n" +
                indent + "        return val.fp_value;\n",
                stackUnion,
                offsetStr,
                cppTypeName,
                byteOrderStr);
        }
        else
        {
            new Formatter(sb).format(
                indent + "        %1$s val;\n" +
                indent + "        std::memcpy(&val, m_buffer + m_offset + %2$s, sizeof(%1$s));\n" +
                indent + "        return %3$s(val);\n",
                cppTypeName,
                offsetStr,
                byteOrderStr);
        }

        return sb;
    }

    private CharSequence generateStoreValue(
        final PrimitiveType primitiveType,
        final String valueSuffix,
        final String offsetStr,
        final ByteOrder byteOrder,
        final String indent)
    {
        final String cppTypeName = cppTypeName(primitiveType);
        final String byteOrderStr = formatByteOrderEncoding(byteOrder, primitiveType);
        final StringBuilder sb = new StringBuilder();

        if (primitiveType == PrimitiveType.FLOAT || primitiveType == PrimitiveType.DOUBLE)
        {
            final String stackUnion = primitiveType == PrimitiveType.FLOAT ?
                "union sbe_float_as_uint" : "union sbe_double_as_uint";

            new Formatter(sb).format(
                indent + "        %1$s val%2$s;\n" +
                indent + "        val%2$s.fp_value = value%2$s;\n" +
                indent + "        val%2$s.uint_value = %3$s(val%2$s.uint_value);\n" +
                indent + "        std::memcpy(m_buffer + m_offset + %4$s, &val%2$s, sizeof(%5$s));\n",
                stackUnion,
                valueSuffix,
                byteOrderStr,
                offsetStr,
                cppTypeName);
        }
        else
        {
            new Formatter(sb).format(
                indent + "        %1$s val%2$s = %3$s(value%2$s);\n" +
                indent + "        std::memcpy(m_buffer + m_offset + %4$s, &val%2$s, sizeof(%1$s));\n",
                cppTypeName,
                valueSuffix,
                byteOrderStr,
                offsetStr);
        }

        return sb;
    }

    private void generateSingleValueProperty(
        final StringBuilder sb,
        final String containingClassName,
        final String propertyName,
        final Token propertyToken,
        final Token encodingToken,
        final String indent)
    {
        final PrimitiveType primitiveType = encodingToken.encoding().primitiveType();
        final String cppTypeName = cppTypeName(primitiveType);
        final int offset = encodingToken.offset();

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD %1$s %2$s() const SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            "%3$s" +
            "%4$s" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            generateFieldNotPresentCondition(propertyToken.version(), encodingToken.encoding(), indent),
            generateLoadValue(primitiveType, Integer.toString(offset), encodingToken.encoding().byteOrder(), indent));

        final CharSequence storeValue = generateStoreValue(
            primitiveType, "", Integer.toString(offset), encodingToken.encoding().byteOrder(), indent);

        new Formatter(sb).format("\n" +
            indent + "    %1$s &%2$s(const %3$s value) SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            "%4$s" +
            indent + "        return *this;\n" +
            indent + "    }\n",
            containingClassName,
            propertyName,
            cppTypeName,
            storeValue);
    }

    private void generateArrayProperty(
        final StringBuilder sb,
        final String containingClassName,
        final String propertyName,
        final Token propertyToken,
        final Token encodingToken,
        final String indent)
    {
        final PrimitiveType primitiveType = encodingToken.encoding().primitiveType();
        final String cppTypeName = cppTypeName(primitiveType);
        final int offset = encodingToken.offset();

        final int arrayLength = encodingToken.arrayLength();
        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t %1$sLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            arrayLength);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD const char *%1$s() const SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            "%2$s" +
            indent + "        return m_buffer + m_offset + %3$d;\n" +
            indent + "    }\n",
            propertyName,
            generateTypeFieldNotPresentCondition(propertyToken.version(), indent),
            offset);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD char *%1$s() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            "%2$s" +
            indent + "        return m_buffer + m_offset + %3$d;\n" +
            indent + "    }\n",
            propertyName,
            generateTypeFieldNotPresentCondition(propertyToken.version(), indent),
            offset);

        final CharSequence loadValue = generateLoadValue(
            primitiveType,
            String.format("%d + (index * %d)", offset, primitiveType.size()),
            encodingToken.encoding().byteOrder(),
            indent);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD %1$s %2$s(const std::uint64_t index) const\n" +
            indent + "    {\n" +
            indent + "        if (index >= %3$d)\n" +
            indent + "        {\n" +
            indent + "            sbe_throw_errnum(E104, \"index out of range for %2$s [E104]\");\n" +
            indent + "            return %1$s();\n" +
            indent + "        }\n\n" +
            "%4$s" +
            "%5$s" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            arrayLength,
            generateFieldNotPresentCondition(propertyToken.version(), encodingToken.encoding(), indent),
            loadValue);

        final CharSequence storeValue = generateStoreValue(
            primitiveType,
            "",
            String.format("%d + (index * %d)", offset, primitiveType.size()),
            encodingToken.encoding().byteOrder(),
            indent);

        new Formatter(sb).format("\n" +
            indent + "    %1$s &%2$s(const std::uint64_t index, const %3$s value)\n" +
            indent + "    {\n" +
            indent + "        if (index >= %4$d)\n" +
            indent + "        {\n" +
            indent + "            sbe_throw_errnum(E105, \"index out of range for %2$s [E105]\");\n" +
            indent + "            return *this;\n" +
            indent + "        }\n\n" +

            "%5$s" +
            indent + "        return *this;\n" +
            indent + "    }\n",
            containingClassName,
            propertyName,
            cppTypeName,
            arrayLength,
            storeValue);

        new Formatter(sb).format("\n" +
            indent + "    std::uint64_t get%1$s(char *const dst, const std::uint64_t length) const\n" +
            indent + "    {\n" +
            indent + "        if (length > %2$d)\n" +
            indent + "        {\n" +
            indent + "            sbe_throw_errnum(E106, \"length too large for get%1$s [E106]\");\n" +
            indent + "            return UINT64_MAX;\n" +
            indent + "        }\n\n" +

            "%3$s" +
            indent + "        std::memcpy(dst, m_buffer + m_offset + %4$d, " +
            "sizeof(%5$s) * static_cast<std::size_t>(length));\n" +
            indent + "        return length;\n" +
            indent + "    }\n",
            toUpperFirstChar(propertyName),
            arrayLength,
            generateArrayFieldNotPresentCondition(propertyToken.version(), indent),
            offset,
            cppTypeName);

        new Formatter(sb).format("\n" +
            indent + "    %1$s &put%2$s(const char *const src) SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        std::memcpy(m_buffer + m_offset + %3$d, src, sizeof(%4$s) * %5$d);\n" +
            indent + "        return *this;\n" +
            indent + "    }\n",
            containingClassName,
            toUpperFirstChar(propertyName),
            offset,
            cppTypeName,
            arrayLength);

        if (arrayLength > 1 && arrayLength <= 4)
        {
            sb.append("\n").append(indent).append("    ")
                .append(containingClassName).append(" &put").append(toUpperFirstChar(propertyName))
                .append("(\n");

            for (int i = 0; i < arrayLength; i++)
            {
                sb.append(indent).append("        ")
                    .append("const ").append(cppTypeName).append(" value").append(i);

                if (i < (arrayLength - 1))
                {
                    sb.append(",\n");
                }
            }

            sb.append(") SBE_NOEXCEPT\n");
            sb.append(indent).append("    {\n");

            for (int i = 0; i < arrayLength; i++)
            {
                sb.append(generateStoreValue(
                    primitiveType,
                    Integer.toString(i),
                    Integer.toString(offset + (i * primitiveType.size())),
                    encodingToken.encoding().byteOrder(),
                    indent));
            }

            sb.append("\n");
            sb.append(indent).append("        return *this;\n");
            sb.append(indent).append("    }\n");
        }

        if (encodingToken.encoding().primitiveType() == PrimitiveType.CHAR)
        {
            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD std::string get%1$sAsString() const\n" +
                indent + "    {\n" +
                indent + "        const char *buffer = m_buffer + m_offset + %2$d;\n" +
                indent + "        size_t length = 0;\n\n" +

                indent + "        for (; length < %3$d && *(buffer + length) != '\\0'; ++length);\n" +
                indent + "        std::string result(buffer, length);\n\n" +

                indent + "        return result;\n" +
                indent + "    }\n",
                toUpperFirstChar(propertyName),
                offset,
                arrayLength);

            generateJsonEscapedStringGetter(sb, encodingToken, indent, propertyName);

            new Formatter(sb).format("\n" +
                indent + "    #if __cplusplus >= 201703L\n" +
                indent + "    SBE_NODISCARD std::string_view get%1$sAsStringView() const SBE_NOEXCEPT\n" +
                indent + "    {\n" +
                indent + "        const char *buffer = m_buffer + m_offset + %2$d;\n" +
                indent + "        size_t length = 0;\n\n" +

                indent + "        for (; length < %3$d && *(buffer + length) != '\\0'; ++length);\n" +
                indent + "        std::string_view result(buffer, length);\n\n" +

                indent + "        return result;\n" +
                indent + "    }\n" +
                indent + "    #endif\n",
                toUpperFirstChar(propertyName),
                offset,
                arrayLength);

            new Formatter(sb).format("\n" +
                indent + "    #if __cplusplus >= 201703L\n" +
                indent + "    %1$s &put%2$s(const std::string_view str)\n" +
                indent + "    {\n" +
                indent + "        const size_t srcLength = str.length();\n" +
                indent + "        if (srcLength > %4$d)\n" +
                indent + "        {\n" +
                indent + "            sbe_throw_errnum(E106, \"string too large for put%2$s [E106]\");\n" +
                indent + "            return *this;\n" +
                indent + "        }\n\n" +

                indent + "        std::memcpy(m_buffer + m_offset + %3$d, str.data(), srcLength);\n" +
                indent + "        for (size_t start = srcLength; start < %4$d; ++start)\n" +
                indent + "        {\n" +
                indent + "            m_buffer[m_offset + %3$d + start] = 0;\n" +
                indent + "        }\n\n" +

                indent + "        return *this;\n" +
                indent + "    }\n" +
                indent + "    #else\n" +
                indent + "    %1$s &put%2$s(const std::string& str)\n" +
                indent + "    {\n" +
                indent + "        const size_t srcLength = str.length();\n" +
                indent + "        if (srcLength > %4$d)\n" +
                indent + "        {\n" +
                indent + "            sbe_throw_errnum(E106, \"string too large for put%2$s [E106]\");\n" +
                indent + "            return *this;\n" +
                indent + "        }\n\n" +

                indent + "        std::memcpy(m_buffer + m_offset + %3$d, str.c_str(), srcLength);\n" +
                indent + "        for (size_t start = srcLength; start < %4$d; ++start)\n" +
                indent + "        {\n" +
                indent + "            m_buffer[m_offset + %3$d + start] = 0;\n" +
                indent + "        }\n\n" +

                indent + "        return *this;\n" +
                indent + "    }\n" +
                indent + "    #endif\n",
                containingClassName,
                toUpperFirstChar(propertyName),
                offset,
                arrayLength);
        }
    }

    private void generateJsonEscapedStringGetter(
        final StringBuilder sb, final Token token, final String indent, final String propertyName)
    {
        new Formatter(sb).format("\n" +
            indent + "    std::string get%1$sAsJsonEscapedString()\n" +
            indent + "    {\n" +
            "%2$s" +
            indent + "        std::ostringstream oss;\n" +
            indent + "        std::string s = get%1$sAsString();\n\n" +
            indent + "        for (std::string::iterator ci= s.begin(); ci != s.end(); ci += 1)\n" +
            indent + "        {\n" +
            indent + "            char c = *ci;\n" +
            indent + "            switch (c)\n" +
            indent + "            {\n" +
            indent + "                case '\"': oss << \"\\\\\\\"\"; break;\n" +
            indent + "                case '\\\\': oss << \"\\\\\\\\\"; break;\n" +
            indent + "                case '\\b': oss << \"\\\\b\"; break;\n" +
            indent + "                case '\\f': oss << \"\\\\f\"; break;\n" +
            indent + "                case '\\n': oss << \"\\\\n\"; break;\n" +
            indent + "                case '\\r': oss << \"\\\\r\"; break;\n" +
            indent + "                case '\\t': oss << \"\\\\t\"; break;\n\n" +
            indent + "                default:\n" +
            indent + "                    if ('\\x00' <= c && c <= '\\x1f')\n" +
            indent + "                    {\n" +
            indent + "                        oss << \"\\\\u\"" + " << std::hex << std::setw(4)\n" +
            indent + "                            << std::setfill('0') << (int)(c);\n" +
            indent + "                    }\n" +
            indent + "                    else\n" +
            indent + "                    {\n" +
            indent + "                        oss << c;\n" +
            indent + "                    }\n" +
            indent + "            }\n" +
            indent + "        }\n\n" +
            indent + "        return oss.str();\n" +
            indent + "    }\n",
            toUpperFirstChar(propertyName),
            generateStringNotPresentCondition(token.version(), indent));
    }

    private void generateConstPropertyMethods(
        final StringBuilder sb, final String propertyName, final Token token, final String indent)
    {
        final String cppTypeName = cppTypeName(token.encoding().primitiveType());

        if (token.encoding().primitiveType() != PrimitiveType.CHAR)
        {
            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD static SBE_CONSTEXPR %1$s %2$s() SBE_NOEXCEPT\n" +
                indent + "    {\n" +
                indent + "        return %3$s;\n" +
                indent + "    }\n",
                cppTypeName,
                propertyName,
                generateLiteral(token.encoding().primitiveType(), token.encoding().constValue().toString()));

            return;
        }

        final byte[] constantValue = token.encoding().constValue().byteArrayValue(token.encoding().primitiveType());
        final StringBuilder values = new StringBuilder();
        for (final byte b : constantValue)
        {
            values.append(b).append(", ");
        }

        if (values.length() > 0)
        {
            values.setLength(values.length() - 2);
        }

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t %1$sLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            constantValue.length);

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD const char *%1$s() const\n" +
            indent + "    {\n" +
            indent + "        static const std::uint8_t %1$sValues[] = { %2$s };\n\n" +

            indent + "        return (const char *)%1$sValues;\n" +
            indent + "    }\n",
            propertyName,
            values);

        sb.append(String.format("\n" +
            indent + "    SBE_NODISCARD %1$s %2$s(const std::uint64_t index) const\n" +
            indent + "    {\n" +
            indent + "        static const std::uint8_t %2$sValues[] = { %3$s };\n\n" +

            indent + "        return (char)%2$sValues[index];\n" +
            indent + "    }\n",
            cppTypeName,
            propertyName,
            values));

        new Formatter(sb).format("\n" +
            indent + "    std::uint64_t get%1$s(char *dst, const std::uint64_t length) const\n" +
            indent + "    {\n" +
            indent + "        static std::uint8_t %2$sValues[] = { %3$s };\n" +
            indent + "        std::uint64_t bytesToCopy = " +
            "length < sizeof(%2$sValues) ? length : sizeof(%2$sValues);\n\n" +

            indent + "        std::memcpy(dst, %2$sValues, static_cast<std::size_t>(bytesToCopy));\n" +
            indent + "        return bytesToCopy;\n" +
            indent + "    }\n",
            toUpperFirstChar(propertyName),
            propertyName,
            values);

        new Formatter(sb).format("\n" +
            indent + "    std::string get%1$sAsString() const\n" +
            indent + "    {\n" +
            indent + "        static const std::uint8_t %1$sValues[] = { %2$s };\n\n" +
            indent + "        return std::string((const char *)%1$sValues, %3$s);\n" +
            indent + "    }\n",
            toUpperFirstChar(propertyName),
            values,
            constantValue.length);

        generateJsonEscapedStringGetter(sb, token, indent, propertyName);
    }

    private CharSequence generateFixedFlyweightCode(final String className, final int size)
    {
        final String schemaIdType = cppTypeName(ir.headerStructure().schemaIdType());
        final String schemaVersionType = cppTypeName(ir.headerStructure().schemaVersionType());

        return String.format(
            "private:\n" +
            "    char *m_buffer;\n" +
            "    std::uint64_t m_bufferLength;\n" +
            "    std::uint64_t m_offset;\n" +
            "    std::uint64_t m_actingVersion;\n\n" +

            "public:\n" +
            "    %1$s()\n" +
            "    {\n" +
            "        m_buffer = nullptr;\n" +
            "        m_bufferLength = 0;\n" +
            "        m_offset = 0;\n" +
            "        m_actingVersion = 0;\n" +
            "    }\n\n" +

            "    %1$s(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t offset,\n" +
            "        const std::uint64_t bufferLength,\n" +
            "        const std::uint64_t actingVersion)\n" +
            "    {\n" +
            "        init(buffer, offset, bufferLength, actingVersion);\n" +
            "    }\n\n" +

            "    void init(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t offset,\n" +
            "        const std::uint64_t bufferLength,\n" +
            "        const std::uint64_t actingVersion)\n" +
            "    {\n" +
            "        m_buffer = buffer;\n" +
            "        m_bufferLength = bufferLength;\n" +
            "        m_offset = offset;\n" +
            "        m_actingVersion = actingVersion;\n" +
            "        if (SBE_BOUNDS_CHECK_EXPECT(((m_offset + %2$s) > m_bufferLength), false))\n" +
            "        {\n" +
            "            sbe_throw_errnum(E107, \"buffer too short for flyweight [E107]\");\n" +
            "            return ;\n" +
            "        }\n" +
            "    }\n\n" +

            "    %1$s(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t bufferLength,\n" +
            "        const std::uint64_t actingVersion)\n" +
            "    {\n" +
            "        init(buffer, 0, bufferLength, actingVersion);\n" +
            "    }\n\n" +

            "    %1$s(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t bufferLength)\n" +
            "    {\n" +
            "        init(buffer, 0, bufferLength, sbeSchemaVersion());\n" +
            "    }\n\n" +

            "    %1$s &wrap(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t offset,\n" +
            "        const std::uint64_t actingVersion,\n" +
            "        const std::uint64_t bufferLength)\n" +
            "    {\n" +
            "        return *this = %1$s(buffer, offset, bufferLength, actingVersion);\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t encodedLength() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %2$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t offset() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_offset;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD const char * buffer() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_buffer;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD char * buffer() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_buffer;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t bufferLength() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_bufferLength;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t actingVersion() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_actingVersion;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR %3$s sbeSchemaId() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %4$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR %5$s sbeSchemaVersion() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %6$s;\n" +
            "    }\n",
            className,
            size,
            schemaIdType,
            generateLiteral(ir.headerStructure().schemaIdType(), Integer.toString(ir.id())),
            schemaVersionType,
            generateLiteral(ir.headerStructure().schemaVersionType(), Integer.toString(ir.version())));
    }

    private static CharSequence generateConstructorsAndOperators(final String className)
    {
        return String.format(
            "    %1$s()\n" +
            "    {\n" +
            "        init(nullptr, 0, 0, 0, 0);\n" +
            "    }\n\n" +

            "    void init(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t offset,\n" +
            "        const std::uint64_t bufferLength,\n" +
            "        const std::uint64_t actingBlockLength,\n" +
            "        const std::uint64_t actingVersion)\n" +
            "    {\n" +
            "        m_buffer = buffer;\n" +
            "        m_bufferLength = bufferLength;\n" +
            "        m_offset = offset;\n" +
            "        m_position = sbeCheckPosition(offset + actingBlockLength);\n" +
            "        m_actingVersion = actingVersion;\n" +
            "    }\n\n" +


            "    %1$s(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t offset,\n" +
            "        const std::uint64_t bufferLength,\n" +
            "        const std::uint64_t actingBlockLength,\n" +
            "        const std::uint64_t actingVersion)\n" +
            "    {\n" +
            "        init(buffer, offset, bufferLength, actingBlockLength, actingVersion);\n" +
            "    }\n\n" +

            "    %1$s(char *buffer, const std::uint64_t bufferLength)\n" +
            "    {\n" +
            "        init(buffer, 0, bufferLength, sbeBlockLength(), sbeSchemaVersion());\n" +
            "    }\n\n" +

            "    %1$s(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t bufferLength,\n" +
            "        const std::uint64_t actingBlockLength,\n" +
            "        const std::uint64_t actingVersion)\n" +
            "    {\n" +
            "        init(buffer, 0, bufferLength, actingBlockLength, actingVersion);\n" +
            "    }\n\n",
            className);
    }

    private CharSequence generateMessageFlyweightCode(final String className, final Token token)
    {
        final String blockLengthType = cppTypeName(ir.headerStructure().blockLengthType());
        final String templateIdType = cppTypeName(ir.headerStructure().templateIdType());
        final String schemaIdType = cppTypeName(ir.headerStructure().schemaIdType());
        final String schemaVersionType = cppTypeName(ir.headerStructure().schemaVersionType());
        final String semanticType = token.encoding().semanticType() == null ? "" : token.encoding().semanticType();
        final String headerType = ir.headerStructure().tokens().get(0).name();

        return String.format(
            "private:\n" +
            "    char *m_buffer;\n" +
            "    std::uint64_t m_bufferLength;\n" +
            "    std::uint64_t m_offset;\n" +
            "    std::uint64_t m_position;\n" +
            "    std::uint64_t m_actingVersion;\n\n" +

            "    inline std::uint64_t *sbePositionPtr() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return &m_position;\n" +
            "    }\n\n" +

            "public:\n" +
            "    typedef %12$s messageHeader;\n\n" +

            "%11$s" +
            "    SBE_NODISCARD static SBE_CONSTEXPR %1$s sbeBlockLength() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %2$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t sbeBlockAndHeaderLength() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return messageHeader::encodedLength() + sbeBlockLength();\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR %3$s sbeTemplateId() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %4$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR %5$s sbeSchemaId() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %6$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR %7$s sbeSchemaVersion() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return %8$s;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD static SBE_CONSTEXPR const char * sbeSemanticType() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return \"%9$s\";\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t offset() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_offset;\n" +
            "    }\n\n" +

            "    %10$s &wrapForEncode(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)\n" +
            "    {\n" +
            "        return *this = %10$s(buffer, offset, bufferLength, sbeBlockLength(), sbeSchemaVersion());\n" +
            "    }\n\n" +

            "    %10$s &wrapAndApplyHeader(" +
            "char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)\n" +
            "    {\n" +
            "        messageHeader hdr(buffer, offset, bufferLength, sbeSchemaVersion());\n\n" +

            "        hdr\n" +
            "            .blockLength(sbeBlockLength())\n" +
            "            .templateId(sbeTemplateId())\n" +
            "            .schemaId(sbeSchemaId())\n" +
            "            .version(sbeSchemaVersion());\n\n" +

            "        return *this = %10$s(\n" +
            "            buffer,\n" +
            "            offset + messageHeader::encodedLength(),\n" +
            "            bufferLength,\n" +
            "            sbeBlockLength(),\n" +
            "            sbeSchemaVersion());\n" +
            "    }\n\n" +

            "    %10$s &wrapForDecode(\n" +
            "        char *buffer,\n" +
            "        const std::uint64_t offset,\n" +
            "        const std::uint64_t actingBlockLength,\n" +
            "        const std::uint64_t actingVersion,\n" +
            "        const std::uint64_t bufferLength)\n" +
            "    {\n" +
            "        return *this = %10$s(buffer, offset, bufferLength, actingBlockLength, actingVersion);\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t sbePosition() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_position;\n" +
            "    }\n\n" +

            "    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)\n" +
            "    std::uint64_t sbeCheckPosition(const std::uint64_t position)\n" +
            "    {\n" +
            "        if (SBE_BOUNDS_CHECK_EXPECT((position > m_bufferLength), false))\n" +
            "        {\n" +
            "            sbe_throw_errnum(E100, \"buffer too short [E100]\");\n" +
            "            return UINT64_MAX;\n" +
            "        }\n" +
            "        return position;\n" +
            "    }\n\n" +

            "    void sbePosition(const std::uint64_t position)\n" +
            "    {\n" +
            "        m_position = sbeCheckPosition(position);\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t encodedLength() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return sbePosition() - m_offset;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t decodeLength() const\n" +
            "    {\n" +
            "        %10$s skipper(m_buffer, m_offset,\n" +
            "            m_bufferLength, sbeBlockLength(), m_actingVersion);\n" +
            "        skipper.skip();\n" +
            "        return skipper.encodedLength();\n" +
            "    }\n\n" +

            "    SBE_NODISCARD const char * buffer() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_buffer;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD char * buffer() SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_buffer;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t bufferLength() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_bufferLength;\n" +
            "    }\n\n" +

            "    SBE_NODISCARD std::uint64_t actingVersion() const SBE_NOEXCEPT\n" +
            "    {\n" +
            "        return m_actingVersion;\n" +
            "    }\n",
            blockLengthType,
            generateLiteral(ir.headerStructure().blockLengthType(), Integer.toString(token.encodedLength())),
            templateIdType,
            generateLiteral(ir.headerStructure().templateIdType(), Integer.toString(token.id())),
            schemaIdType,
            generateLiteral(ir.headerStructure().schemaIdType(), Integer.toString(ir.id())),
            schemaVersionType,
            generateLiteral(ir.headerStructure().schemaVersionType(), Integer.toString(ir.version())),
            semanticType,
            className,
            generateConstructorsAndOperators(className),
            formatClassName(headerType));
    }

    private void generateFields(
        final StringBuilder sb,
        final String containingClassName,
        final List<Token> tokens,
        final String indent)
    {
        for (int i = 0, size = tokens.size(); i < size; i++)
        {
            final Token signalToken = tokens.get(i);
            if (signalToken.signal() == Signal.BEGIN_FIELD)
            {
                final Token encodingToken = tokens.get(i + 1);
                final String propertyName = formatPropertyName(signalToken.name());

                generateFieldMetaAttributeMethod(sb, signalToken, indent);
                generateFieldCommonMethods(indent, sb, signalToken, encodingToken, propertyName);

                switch (encodingToken.signal())
                {
                    case ENCODING:
                        generatePrimitiveProperty(
                            sb, containingClassName, propertyName, signalToken, encodingToken, indent);
                        break;

                    case BEGIN_ENUM:
                        generateEnumProperty(sb, containingClassName, signalToken, propertyName, encodingToken, indent);
                        break;

                    case BEGIN_SET:
                        generateBitsetProperty(sb, propertyName, encodingToken, indent);
                        break;

                    case BEGIN_COMPOSITE:
                        generateCompositeProperty(sb, propertyName, encodingToken, indent);
                        break;

                    default:
                        break;
                }
            }
        }
    }

    private void generateFieldCommonMethods(
        final String indent,
        final StringBuilder sb,
        final Token fieldToken,
        final Token encodingToken,
        final String propertyName)
    {
        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint16_t %1$sId() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            fieldToken.id());

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t %1$sSinceVersion() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n\n" +

            indent + "    SBE_NODISCARD bool %1$sInActingVersion() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return m_actingVersion >= %1$sSinceVersion();\n" +
            indent + "    }\n",
            propertyName,
            fieldToken.version());

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t %1$sEncodedOffset() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            encodingToken.offset());
    }

    private static void generateFieldMetaAttributeMethod(
        final StringBuilder sb, final Token token, final String indent)
    {
        final Encoding encoding = token.encoding();
        final String epoch = encoding.epoch() == null ? "" : encoding.epoch();
        final String timeUnit = encoding.timeUnit() == null ? "" : encoding.timeUnit();
        final String semanticType = encoding.semanticType() == null ? "" : encoding.semanticType();

        sb.append("\n")
            .append(indent).append("    SBE_NODISCARD static const char * ")
            .append(token.name()).append("MetaAttribute(const sbe_meta_attribute metaAttribute) SBE_NOEXCEPT\n")
            .append(indent).append("    {\n")
            .append(indent).append("        switch (metaAttribute)\n")
            .append(indent).append("        {\n");

        if (!Strings.isEmpty(epoch))
        {
            sb.append(indent)
                .append("            case sbe_meta_attribute_EPOCH: return \"").append(epoch).append("\";\n");
        }

        if (!Strings.isEmpty(timeUnit))
        {
            sb.append(indent)
                .append("            case sbe_meta_attribute_TIME_UNIT: return \"").append(timeUnit).append("\";\n");
        }

        if (!Strings.isEmpty(semanticType))
        {
            sb.append(indent)
                .append("            case sbe_meta_attribute_SEMANTIC_TYPE: return \"").append(semanticType)
                .append("\";\n");
        }

        sb
            .append(indent).append("            case sbe_meta_attribute_PRESENCE: return \"")
            .append(encoding.presence().toString().toLowerCase()).append("\";\n")
            .append(indent).append("            default: return \"\";\n")
            .append(indent).append("        }\n")
            .append(indent).append("    }\n");
    }

    private static CharSequence generateEnumFieldNotPresentCondition(
        final int sinceVersion,
        final String enumName,
        final String indent)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            indent + "        if (m_actingVersion < %1$d)\n" +
            indent + "        {\n" +
            indent + "            return %2$s::NULL_VALUE;\n" +
            indent + "        }\n\n",
            sinceVersion,
            enumName);
    }

    private void generateEnumProperty(
        final StringBuilder sb,
        final String containingClassName,
        final Token fieldToken,
        final String propertyName,
        final Token token,
        final String indent)
    {
        final String enumName = formatClassName(token.applicableTypeName());
        final String typeName = cppTypeName(token.encoding().primitiveType());
        final int offset = token.offset();

        new Formatter(sb).format("\n" +
            indent + "    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t %1$sEncodedLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            fieldToken.encodedLength());

        if (fieldToken.isConstantEncoding())
        {
            final String constValue = fieldToken.encoding().constValue().toString();

            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD static SBE_CONSTEXPR %1$s::Value %2$sConstValue() SBE_NOEXCEPT\n" +
                indent + "    {\n" +
                indent + "        return %1$s::%3$s;\n" +
                indent + "    }\n",
                enumName,
                propertyName,
                constValue.substring(constValue.indexOf(".") + 1));

            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD %1$s::Value %2$s() const SBE_NOEXCEPT\n" +
                indent + "    {\n" +
                "%3$s" +
                indent + "        return %1$s::%4$s;\n" +
                indent + "    }\n",
                enumName,
                propertyName,
                generateEnumFieldNotPresentCondition(fieldToken.version(), enumName, indent),
                constValue.substring(constValue.indexOf(".") + 1));
        }
        else
        {
            new Formatter(sb).format("\n" +
                indent + "    SBE_NODISCARD %1$s::Value %2$s() const SBE_NOEXCEPT\n" +
                indent + "    {\n" +
                "%3$s" +
                indent + "        %5$s val;\n" +
                indent + "        std::memcpy(&val, m_buffer + m_offset + %6$d, sizeof(%5$s));\n" +
                indent + "        return %1$s::get(%4$s(val));\n" +
                indent + "    }\n",
                enumName,
                propertyName,
                generateEnumFieldNotPresentCondition(fieldToken.version(), enumName, indent),
                formatByteOrderEncoding(token.encoding().byteOrder(), token.encoding().primitiveType()),
                typeName,
                offset);

            new Formatter(sb).format("\n" +
                indent + "    %1$s &%2$s(const %3$s::Value value) SBE_NOEXCEPT\n" +
                indent + "    {\n" +
                indent + "        %4$s val = %6$s(value);\n" +
                indent + "        std::memcpy(m_buffer + m_offset + %5$d, &val, sizeof(%4$s));\n" +
                indent + "        return *this;\n" +
                indent + "    }\n",
                formatClassName(containingClassName),
                propertyName,
                enumName,
                typeName,
                offset,
                formatByteOrderEncoding(token.encoding().byteOrder(), token.encoding().primitiveType()));
        }
    }

    private static void generateBitsetProperty(
        final StringBuilder sb, final String propertyName, final Token token, final String indent)
    {
        final String bitsetName = formatClassName(token.applicableTypeName());
        final int offset = token.offset();

        new Formatter(sb).format("\n" +
            indent + "private:\n" +
            indent + "    %1$s m_%2$s;\n\n" +

            indent + "public:\n",
            bitsetName,
            propertyName);

        new Formatter(sb).format(
            indent + "    SBE_NODISCARD %1$s &%2$s()\n" +
            indent + "    {\n" +
            indent + "        m_%2$s.wrap(m_buffer, m_offset + %3$d, m_actingVersion, m_bufferLength);\n" +
            indent + "        return m_%2$s;\n" +
            indent + "    }\n",
            bitsetName,
            propertyName,
            offset);

        new Formatter(sb).format("\n" +
            indent + "    static SBE_CONSTEXPR std::uint64_t %1$sEncodedLength() SBE_NOEXCEPT\n" +
            indent + "    {\n" +
            indent + "        return %2$d;\n" +
            indent + "    }\n",
            propertyName,
            token.encoding().primitiveType().size());
    }

    private static void generateCompositeProperty(
        final StringBuilder sb, final String propertyName, final Token token, final String indent)
    {
        final String compositeName = formatClassName(token.applicableTypeName());

        new Formatter(sb).format("\n" +
            "private:\n" +
            indent + "    %1$s m_%2$s;\n\n" +

            "public:\n",
            compositeName,
            propertyName);

        new Formatter(sb).format(
            indent + "    SBE_NODISCARD %1$s &%2$s()\n" +
            indent + "    {\n" +
            indent + "        m_%2$s.wrap(m_buffer, m_offset + %3$d, m_actingVersion, m_bufferLength);\n" +
            indent + "        return m_%2$s;\n" +
            indent + "    }\n",
            compositeName,
            propertyName,
            token.offset());
    }

    private CharSequence generateNullValueLiteral(final PrimitiveType primitiveType, final Encoding encoding)
    {
        // Visual C++ does not handle minimum integer values properly
        // See: http://msdn.microsoft.com/en-us/library/4kh09110.aspx
        // So some of the null values get special handling
        if (null == encoding.nullValue())
        {
            switch (primitiveType)
            {
                case CHAR:
                case FLOAT:
                case DOUBLE:
                    break; // no special handling
                case INT8:
                    return "SBE_NULLVALUE_INT8";
                case INT16:
                    return "SBE_NULLVALUE_INT16";
                case INT32:
                    return "SBE_NULLVALUE_INT32";
                case INT64:
                    return "SBE_NULLVALUE_INT64";
                case UINT8:
                    return "SBE_NULLVALUE_UINT8";
                case UINT16:
                    return "SBE_NULLVALUE_UINT16";
                case UINT32:
                    return "SBE_NULLVALUE_UINT32";
                case UINT64:
                    return "SBE_NULLVALUE_UINT64";
            }
        }

        return generateLiteral(primitiveType, encoding.applicableNullValue().toString());
    }

    private CharSequence generateLiteral(final PrimitiveType type, final String value)
    {
        String literal = "";

        final String castType = cppTypeName(type);
        switch (type)
        {
            case CHAR:
            case UINT8:
            case UINT16:
            case INT8:
            case INT16:
                literal = "(" + castType + ")" + value;
                break;

            case UINT32:
            {
                final long intValue = Long.parseLong(value);
                literal = "UINT32_C(0x" + Integer.toHexString((int)intValue) + ")";
                break;
            }
            case INT32:
                final long intValue = Long.parseLong(value);
                if (intValue == Integer.MIN_VALUE)
                {
                    literal = "INT32_MIN";
                }
                else
                {
                    literal = "INT32_C(" + Long.toString(intValue) + ")";
                }
                break;

            case FLOAT:
                literal = value.endsWith("NaN") ? "SBE_FLOAT_NAN" : value + "f";
                break;

            case INT64:
                final long longValue = Long.parseLong(value);
                if (longValue == Long.MIN_VALUE)
                {
                    literal = "INT64_MIN";
                }
                else
                {
                    literal = "INT64_C(" + String.valueOf(longValue) + ")";
                }
                break;

            case UINT64:
                literal = "UINT64_C(0x" + Long.toHexString(Long.parseLong(value)) + ")";
                break;

            case DOUBLE:
                literal = value.endsWith("NaN") ? "SBE_DOUBLE_NAN" : value;
                break;
        }

        return literal;
    }

    public static String jsonString(final String token, final String suffix)
    {
        /* "\"color\"" */
        /* "\"color\": " */
        /* "\"color\", " */
        return String.format("\"\\\"%1$s\\\"%2$s\"", token, suffix);
    }

    private void generateDisplay(
        final StringBuilder sb,
        final MessageItem messageItem)
    {
        new Formatter(sb).format("\n" +
            INDENT + "friend std::ostream& operator << (std::ostream& builder, %1$s _writer)\n" +
            INDENT + "{\n" +
            INDENT + "    %1$s writer(_writer.m_buffer, _writer.m_offset,\n" +
            INDENT + "        _writer.m_bufferLength, _writer.sbeBlockLength(), _writer.m_actingVersion);\n" +
            INDENT + "    builder << '{';\n" +
            INDENT + "    builder << " + jsonString("Name", ": ") + " << " + jsonString("%2$s", ", ") + ";\n" +
            INDENT + "    builder << " + jsonString("sbeTemplateId", ": ") + ";\n" +
            INDENT + "    builder << writer.sbeTemplateId();\n" +
            INDENT + "    builder << \", \";\n\n" +
            "%3$s" +
            INDENT + "    builder << '}';\n\n" +
            INDENT + "    return builder;\n" +
            INDENT + "}\n",
            messageItemFullClassName(messageItem),
            messageItemClassName(messageItem),
            appendDisplay(messageItem, INDENT + INDENT));
    }

    private void generateGroupDisplay(
        final StringBuilder sb,
        final MessageItem messageItem,
        final String indent)
    {
        formatter(sb).format("\n" +
            indent + "friend std::ostream& operator << (std::ostream& builder, %1$s writer)\n" +
            indent + "{\n" +
            indent + "    builder << '{';\n" +
            "%2$s" +
            indent + "    builder << '}';\n\n" +
            indent + "    return builder;\n" +
            indent + "}\n",
            messageItemFullClassName(messageItem),
            appendDisplay(messageItem, indent + INDENT));
    }

    private CharSequence generateCompositeDisplay(final String name, final List<Token> tokens)
    {
        return String.format("\n" +
            "friend std::ostream& operator << (std::ostream& builder, %1$s writer)\n" +
            "{\n" +
            "    builder << '{';\n" +
            "%2$s" +
            "    builder << '}';\n\n" +
            "    return builder;\n" +
            "}\n\n",
            formatClassName(name),
            appendDisplay(new MessageItem(tokens, new ArrayList<Token>()), INDENT)
        );
    }

    private CharSequence appendDisplay(
        final MessageItem messageItem, final String indent)
    {
        final StringBuilder sb = new StringBuilder();
        final boolean[] atLeastOne = { false };

        for (int i = 0, size = messageItem.fields.size(); i < size;)
        {
            final Token fieldToken = messageItem.fields.get(i);
            final Token encodingToken = messageItem.fields.get(fieldToken.signal() == Signal.BEGIN_FIELD ? i + 1 : i);

            writeTokenDisplay(sb, fieldToken.name(), encodingToken, atLeastOne, indent);
            i += fieldToken.componentTokenCount();
        }

        for (int i = 0, size = messageItem.children.size(); i < size; i++)
        {
            final MessageItem child = messageItem.children.get(i);
            final Token groupToken = child.rootToken;
            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }

            if (atLeastOne[0])
            {
                sb.append(indent).append("builder << \", \";\n");
            }
            atLeastOne[0] = true;

            new Formatter(sb).format(
                indent + "{\n" +
                indent + "    bool atLeastOne = false;\n" +
                indent + "    builder << " + jsonString("%3$s", ": [") + ";\n" +
                indent + "    %1$s& %2$s = writer.%2$s();\n" +
                indent + "    while(%2$s.hasNext())\n" +
                indent + "    {\n" +
                indent + "        %2$s.next();\n" +
                indent + "        if (atLeastOne)\n" +
                indent + "        {\n" +
                indent + "            builder << \", \";\n" +
                indent + "        }\n" +
                indent + "        atLeastOne = true;\n" +
                indent + "        builder << %2$s;\n" +
                indent + "    };\n" +
                indent + "    builder << ']';\n" +
                indent + "}\n\n",
                messageItemFullClassName(child),
                formatPropertyName(groupToken.name()),
                groupToken.name());
        }

        for (int i = 0, size = messageItem.varData.size(); i < size;)
        {
            final Token varDataToken = messageItem.varData.get(i);
            if (varDataToken.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + varDataToken);
            }

            if (atLeastOne[0])
            {
                sb.append(indent).append("builder << \", \";\n");
            }
            atLeastOne[0] = true;

            final String characterEncoding = messageItem.varData.get(i + 3).encoding().characterEncoding();
            sb.append(indent).append("builder << " + jsonString(varDataToken.name(), ": ") + ";\n");

            if (null == characterEncoding)
            {
                final String skipFunction = "writer.skip" + toUpperFirstChar(varDataToken.name()) + "()";

                sb.append(indent).append("builder << '\"' <<\n").append(indent).append(INDENT).append(skipFunction)
                    .append(" << \" bytes of raw data\\\"\";\n");
            }
            else
            {
                final String getAsStringFunction =
                    "writer.get" + toUpperFirstChar(varDataToken.name()) + "AsJsonEscapedString().c_str()";

                sb.append(indent).append("builder << '\"' <<\n").append(indent).append(INDENT)
                    .append(getAsStringFunction).append(" << '\"';\n\n");
            }

            i += varDataToken.componentTokenCount();
        }

        return sb;
    }

    private void writeTokenDisplay(
        final StringBuilder sb,
        final String fieldTokenName,
        final Token typeToken,
        final boolean[] atLeastOne,
        final String indent)
    {
        if (typeToken.encodedLength() <= 0 || typeToken.isConstantEncoding())
        {
            return;
        }

        if (atLeastOne[0])
        {
            sb.append(indent).append("builder << \", \";\n");
        }
        else
        {
            atLeastOne[0] = true;
        }

        sb.append(indent).append("builder << ").append(jsonString(fieldTokenName, ": ")).append(";\n");
        final String fieldName = "writer." + formatPropertyName(fieldTokenName);

        switch (typeToken.signal())
        {
            case ENCODING:
                if (typeToken.arrayLength() > 1)
                {
                    if (typeToken.encoding().primitiveType() == PrimitiveType.CHAR)
                    {
                        final String getAsStringFunction =
                            "writer.get" + toUpperFirstChar(fieldTokenName) + "AsJsonEscapedString().c_str()";

                        sb.append(indent).append("builder << '\"' <<\n").append(indent).append(INDENT)
                            .append(getAsStringFunction).append(" << '\"';\n");
                    }
                    else
                    {
                        sb.append(
                            indent + "builder << '[';\n" +
                            indent + "if (" + fieldName + "Length() > 0)\n" +
                            indent + "{\n" +
                            indent + "    for (size_t i = 0, length = static_cast<std::size_t>(" +
                            fieldName + "Length()); i < length; i++)\n" +
                            indent + "    {\n" +
                            indent + "        if (i)\n" +
                            indent + "        {\n" +
                            indent + "            builder << ',';\n" +
                            indent + "        }\n" +
                            indent + "        builder << +" + fieldName + "(i);\n" +
                            indent + "    }\n" +
                            indent + "}\n" +
                            indent + "builder << ']';\n");
                    }
                }
                else
                {
                    if (typeToken.encoding().primitiveType() == PrimitiveType.CHAR)
                    {
                        sb.append(
                            indent + "if (std::isprint(" + fieldName + "()))\n" +
                            indent + "{\n" +
                            indent + "    builder << '\"' << (char)" + fieldName + "() << '\"';\n" +
                            indent + "}\n" +
                            indent + "else\n" +
                            indent + "{\n" +
                            indent + "    builder << (int)" + fieldName + "();\n" +
                            indent + "}\n");
                    }
                    else
                    {
                        sb.append(indent).append("builder << +").append(fieldName).append("();\n");
                    }
                }
                break;

            case BEGIN_ENUM:
                sb.append(indent).append("builder << '\"' << ").append(fieldName).append("() << '\"';\n");
                break;

            case BEGIN_SET:
            case BEGIN_COMPOSITE:
                sb.append(indent).append("builder << ").append(fieldName).append("();\n");
                break;

            default:
                break;
        }

        sb.append('\n');
    }

    private CharSequence generateChoicesDisplay(final String name, final List<Token> tokens)
    {
        final String indent = INDENT;
        final StringBuilder sb = new StringBuilder();
        final List<Token> choiceTokens = new ArrayList<>();

        collect(Signal.CHOICE, tokens, 0, choiceTokens);

        new Formatter(sb).format("\n" +
            indent + "friend std::ostream& operator << (std::ostream& builder, %1$s writer)\n" +
            indent + "{\n" +
            indent + "    builder << '[';\n",
            name);

        if (choiceTokens.size() > 1)
        {
            sb.append(indent + "    bool atLeastOne = false;\n");
        }

        for (int i = 0, size = choiceTokens.size(); i < size; i++)
        {
            final Token token = choiceTokens.get(i);
            final String choiceName = "writer." + formatPropertyName(token.name());

            sb.append(indent + "    if (").append(choiceName).append("())\n")
                .append(indent).append("    {\n");

            if (i > 0)
            {
                sb.append(
                    indent + "        if (atLeastOne)\n" +
                    indent + "        {\n" +
                    indent + "            builder << \",\";\n" +
                    indent + "        }\n");
            }
            sb.append(indent + "        builder << " + jsonString(token.name(), "") + ";\n");

            if (i < (size - 1))
            {
                sb.append(indent + "        atLeastOne = true;\n");
            }

            sb.append(indent + "    }\n");
        }

        sb.append(
            indent + "    builder << ']';\n" +
            indent + "    return builder;\n" +
            indent + "}\n");

        return sb;
    }

    private CharSequence generateEnumDisplay(final List<Token> tokens, final Token encodingToken)
    {
        final String enumName = formatClassName(encodingToken.applicableTypeName());
        final StringBuilder sb = new StringBuilder();

        new Formatter(sb).format("\n" +
            "    static const char* c_str(const %1$s::Value value)\n" +
            "    {\n" +
            "        switch (value)\n" +
            "        {\n",
            enumName);

        for (final Token token : tokens)
        {
            new Formatter(sb).format(
                "            case %1$s: return \"%1$s\";\n",
                token.name());
        }

        new Formatter(sb).format(
            "            case NULL_VALUE: return \"NULL_VALUE\";\n" +
            "        }\n\n" +
            "        return sbe_throw_errnum(E103, \"unknown value for enum %1$s [E103]:\");\n" +
            "    }\n\n" +

            "    friend std::ostream& operator << (std::ostream& os, %1$s::Value m)\n" +
            "    {\n" +
            "        return os << %1$s::c_str(m);\n" +
            "    }\n",
            enumName);

        return sb;
    }

    private void generateProperties(
        final StringBuilder sbLengthType,
        final StringBuilder sb,
        final MessageItem messageItem,
        final String parentIndent)
    {
        final StringBuilder sbGroupProperties = new StringBuilder();
        final StringBuilder sbEncode = new StringBuilder();
        final StringBuilder sbSkip = new StringBuilder();
        final String indent = parentIndent + INDENT;

        for (int i = 0, size = messageItem.children.size(); i < size; i++)
        {
            final MessageItem child = messageItem.children.get(i);
            final Token groupToken = child.rootToken;
            final String propertyName = formatPropertyName(groupToken.name());
            final String memberClassName = messageItemFullClassName(child);

            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }

            final Token numInGroupToken = Generators.findFirst("numInGroup", child.tokens, 0);
            final long minCount = numInGroupToken.encoding().applicableMinValue().longValue();
            final long maxCount = numInGroupToken.encoding().applicableMaxValue().longValue();

            final String cppTypeForNumInGroup = cppTypeName(numInGroupToken.encoding().primitiveType());
            generateGroupProperty(
                sbGroupProperties, memberClassName, propertyName,
                child.rootToken, cppTypeForNumInGroup, parentIndent);

            final boolean childIsConstant = child.isConst();
            final String countMember = "lengthInfo." + propertyName;
            final String countName = childIsConstant ? countMember : countMember + ".size()";

            final String minCheck = minCount > 0 ? countName + " < " + minCount + " || " : "";
            final String maxCheck = countName + " > " + maxCount;

            new Formatter(sbEncode).format("\n" +
                indent + "    length += static_cast<std::size_t>(%1$s::sbeHeaderSize());\n",
                memberClassName);

            if (childIsConstant)
            {
                new Formatter(sbEncode).format(
                    indent + "    if (%3$s%4$s)\n" +
                    indent + "    {\n" +
                    indent + "        sbe_throw_errnum(E110, \"%1$s outside of allowed range [E110]\");\n" +
                    indent + "        return SIZE_MAX ;\n" +
                    indent + "    }\n" +
                    indent + "    length += static_cast<std::size_t>(%1$s * %2$s::sbeBlockLength());\n",
                    countName,
                    memberClassName,
                    minCheck,
                    maxCheck);
            }
            else
            {
                new Formatter(sbEncode).format(
                    indent + "    if (%3$s%4$s)\n" +
                    indent + "    {\n" +
                    indent + "        sbe_throw_errnum(E110, \"%5$s outside of allowed range [E110]\");\n" +
                    indent + "        return SIZE_MAX ;\n" +
                    indent + "    }\n" +
                    indent + "    for (const %2$sLengthParam *ei = %1$s.begin(); ei != %1$s.end(); ei+= 1)\n" +
                    indent + "    {\n" +
                    indent + "        length += %2$s::computeLength(*ei);\n" +
                    indent + "    }\n",
                    countMember,
                    memberClassName,
                    minCheck,
                    maxCheck,
                    countName);
            }
            if (childIsConstant)
            {
                formatter(sbLengthType).format(
                    indent + "size_t %1$s;\n", propertyName);
            }
            else
            {
                formatter(sbLengthType).format(
                    indent + "sbe_vector_view<const %1$sLengthParam> %2$s;\n", memberClassName, propertyName);
            }

            new Formatter(sbSkip).format(
                indent + "    %1$s& %2$s = this->%2$s();\n" +
                indent + "    while(%2$s.hasNext())\n" +
                indent + "    {\n" +
                indent + "        %2$s.next();\n" +
                indent + "        %2$s.skip();\n" +
                indent + "    };\n",
                memberClassName,
                formatPropertyName(groupToken.name()),
                groupToken.name());
        }

        final String className = messageItemFullClassName(messageItem);
        /* First, fixed size fields */
        generateFields(sb, className, messageItem.fields, parentIndent);
        /* Second, var size groups */
        sb.append(sbGroupProperties);
        /* Third, var size data */
        generateVarData(sb, className, messageItem.varData, parentIndent);

        for (int i = 0, size = messageItem.varData.size(); i < size;)
        {
            final Token varDataToken = messageItem.varData.get(i);

            if (varDataToken.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + varDataToken);
            }

            final String propertyName = toUpperFirstChar(varDataToken.name());
            final Token lengthToken = Generators.findFirst("length", messageItem.varData, i);
            formatter(sbLengthType).format(
                indent + "size_t %1$s;\n", varDataToken.name());

            new Formatter(sbEncode).format("\n" +
                indent + "    length += static_cast<std::size_t>(%1$sHeaderLength());\n" +
                indent + "    if (lengthInfo.%1$s > %2$d)\n" +
                indent + "    {\n" +
                indent + "        sbe_throw_errnum(E109, \"lengthInfo.%1$s too long for length type [E109]\");\n" +
                indent + "        return SIZE_MAX ;\n" +
                indent + "    }\n" +
                indent + "    length += lengthInfo.%1$s;\n",
                varDataToken.name(),
                lengthToken.encoding().applicableMaxValue().longValue());

            new Formatter(sbSkip).format(
                indent + "    skip%1$s();\n",
                propertyName);

            i += varDataToken.componentTokenCount();
        }

        if (messageItem.rootToken.signal() == Signal.BEGIN_GROUP)
        {
            generateGroupDisplay(sb, messageItem, indent);
        }
        else
        {
            generateDisplay(sb, messageItem);
        }

        new Formatter(sb).format("\n" +
            indent + "void skip()\n" +
            indent + "{\n" +
            "%1$s" +
            indent + "}\n\n" +

            indent + "SBE_NODISCARD static SBE_CONSTEXPR bool isConstLength() SBE_NOEXCEPT\n" +
            indent + "{\n" +
            indent + "    return " + (messageItem.isConst() ? "true" : "false") + ";\n" +
            indent + "}\n\n" +

            indent + "SBE_NODISCARD static SBE_CONSTEXPR_14 size_t computeLength(%2$s)\n" +
            indent + "{\n" +
            indent + "    size_t length = static_cast<std::size_t>(sbeBlockLength());\n\n" +
            "%3$s" +
            indent + "    return length;\n" +
            indent + "}\n",
            sbSkip,
            messageItem.isConst() ? "" : "const " + messageItemFullClassName(messageItem) + "LengthParam &lengthInfo",
            sbEncode);
    }
}
