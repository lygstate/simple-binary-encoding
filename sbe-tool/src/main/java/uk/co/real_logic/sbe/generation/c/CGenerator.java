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
package uk.co.real_logic.sbe.generation.c;

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
import static uk.co.real_logic.sbe.generation.c.CUtil.*;
import static uk.co.real_logic.sbe.ir.GenerationUtil.*;

/**
 * Codec generator for the C11 programming language.
 */
@SuppressWarnings("MethodLength")
public class CGenerator implements CodeGenerator
{
    private final Ir ir;
    private final OutputManager outputManager;

    public CGenerator(final Ir ir, final OutputManager outputManager)
    {
        Verify.notNull(ir, "ir");
        Verify.notNull(outputManager, "outputManager");

        this.ir = ir;
        this.outputManager = outputManager;
    }

    public void generateTypeStubs(final Writer out, final CharSequence[] scope) throws IOException
    {
        for (final List<Token> tokens : GenerationUtil.sortTypes(ir))
        {
            switch (tokens.get(0).signal())
            {
                case BEGIN_ENUM:
                    generateEnum(out, scope, tokens);
                    break;

                case BEGIN_SET:
                    generateChoiceSet(out, scope, tokens);
                    break;

                case BEGIN_COMPOSITE:
                    generateComposite(out, scope, tokens);
                    break;

                default:
                    break;
            }
        }

    }

    public void generate() throws IOException
    {
        final String filename = formatScope(ir.namespaces());
        try (Writer out = outputManager.createOutput(filename))
        {
            out.append(generateFileHeader(filename, null));
            generateTypeStubs(out, ir.namespaces());
            for (final List<Token> tokens : ir.messages())
            {
                final Token msgToken = tokens.get(0);

                final String structName = formatScopedName(ir.namespaces(), msgToken.name());

                final List<Token> messageBody = tokens.subList(1, tokens.size() - 1);
                int i = 0;

                final List<Token> fields = new ArrayList<>();
                i = collectFields(messageBody, i, fields);

                final List<Token> groups = new ArrayList<>();
                i = collectGroups(messageBody, i, groups);

                final List<Token> varData = new ArrayList<>();
                collectVarData(messageBody, i, varData);
                out.append(generateMessageFlyweightStruct(structName));
                out.append(generateMessageFlyweightFunctions(structName, msgToken, ir.namespaces()));

                out.append(generateFieldFunctions(ir.namespaces(), structName, structName, fields));

                final StringBuilder sb = new StringBuilder();
                generateGroups(sb, ir.namespaces(), groups, structName, structName);
                out.append(sb);
                out.append(generateVarData(structName, structName, varData));
            }
            out.append("\n#endif\n");
        }
    }

    private void generateGroups(
        final StringBuilder sb,
        final CharSequence[] scope,
        final List<Token> tokens,
        final String outerStruct,
        final String outermostStruct)
    {
        for (int i = 0, size = tokens.size(); i < size; ++i)
        {
            final Token groupToken = tokens.get(i);
            if (groupToken.signal() != Signal.BEGIN_GROUP)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_GROUP: token=" + groupToken);
            }

            final String groupName = outerStruct + '_' + formatName(groupToken.name());
            final Token numInGroupToken = Generators.findFirst("numInGroup", tokens, i);
            final String cTypeForNumInGroup = cTypeName(numInGroupToken.encoding().primitiveType());

            generateGroupStruct(sb, groupName);
            generateGroupHeaderFunctions(sb, scope, groupName, tokens, i);

            ++i;
            final int groupHeaderTokenCount = tokens.get(i).componentTokenCount();
            i += groupHeaderTokenCount;

            final List<Token> fields = new ArrayList<>();
            i = collectFields(tokens, i, fields);
            sb.append(generateFieldFunctions(scope, groupName, outermostStruct, fields));

            final List<Token> groups = new ArrayList<>();
            i = collectGroups(tokens, i, groups);
            generateGroups(sb, scope, groups, groupName, outermostStruct);

            final List<Token> varData = new ArrayList<>();
            i = collectVarData(tokens, i, varData);
            sb.append(generateVarData(groupName, outermostStruct, varData));

            sb.append(generateGroupPropertyFunctions(outerStruct, groupName, groupToken, cTypeForNumInGroup));
        }
    }

    private static void generateGroupStruct(final StringBuilder sb, final String groupName)
    {
        sb.append(String.format("\n" +
            "typedef struct sbe_group %1$s;\n",
            groupName));
    }

    private static void generateGroupHeaderFunctions(
        final StringBuilder sb,
        final CharSequence[] scope,
        final String groupName,
        final List<Token> tokens,
        final int index)
    {
        final String dimensionsStructName = formatScopedName(scope, tokens.get(index + 1).name());
        final int dimensionHeaderLength = tokens.get(index + 1).encodedLength();
        final int blockLength = tokens.get(index).encodedLength();
        final Token blockLengthToken = Generators.findFirst("blockLength", tokens, index);
        final Token numInGroupToken = Generators.findFirst("numInGroup", tokens, index);
        final String cTypeForBlockLength = cTypeName(blockLengthToken.encoding().primitiveType());
        final String cTypeForNumInGroup = cTypeName(numInGroupToken.encoding().primitiveType());

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t *%1$s_sbe_position_ptr(\n" +
            "    %1$s *const codec)\n" +
            "{\n" +
            "    return codec->position_ptr;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %1$s *%1$s_wrap_for_decode(\n" +
            "    %1$s *const codec,\n" +
            "    char *const buffer,\n" +
            "    uint64_t *const pos,\n" +
            "    const uint64_t acting_version,\n" +
            "    const uint64_t buffer_length)\n" +
            "{\n" +
            "    codec->buffer = buffer;\n" +
            "    codec->buffer_length = buffer_length;\n" +
            "    %2$s dimensions;\n" +
            "    if (!%2$s_wrap(&dimensions, codec->buffer, *pos, acting_version, buffer_length))\n" +
            "    {\n" +
            "        return NULL;\n" +
            "    }\n\n" +
            "    codec->block_length = %2$s_blockLength(&dimensions);\n" +
            "    codec->count = %2$s_numInGroup(&dimensions);\n" +
            "    codec->index = -1;\n" +
            "    codec->acting_version = acting_version;\n" +
            "    codec->position_ptr = pos;\n" +
            "    *codec->position_ptr = *codec->position_ptr + %3$d;\n\n" +
            "    return codec;\n" +
            "}\n",
            groupName, dimensionsStructName, dimensionHeaderLength));

        final long minCount = numInGroupToken.encoding().applicableMinValue().longValue();
        final String minCheck = minCount > 0 ? "count < " + minCount + " || " : "";

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s *%1$s_wrap_for_encode(\n" +
            "    %1$s *const codec,\n" +
            "    char *const buffer,\n" +
            "    const %4$s count,\n" +
            "    uint64_t *const pos,\n" +
            "    const uint64_t acting_version,\n" +
            "    const uint64_t buffer_length)\n" +
            "{\n" +
            "    if (%7$scount > %8$d)\n" +
            "    {\n" +
            "        errno = E110;\n" +
            "        return NULL;\n" +
            "    }\n" +
            "    codec->buffer = buffer;\n" +
            "    codec->buffer_length = buffer_length;\n" +
            "    %5$s dimensions;\n" +
            "    if (!%5$s_wrap(&dimensions, codec->buffer, *pos, acting_version, buffer_length))\n" +
            "    {\n" +
            "        return NULL;\n" +
            "    }\n\n" +
            "    %5$s_set_blockLength(&dimensions, (%2$s)%3$d);\n" +
            "    %5$s_set_numInGroup(&dimensions, (%4$s)count);\n" +
            "    codec->index = -1;\n" +
            "    codec->count = count;\n" +
            "    codec->block_length = %3$d;\n" +
            "    codec->acting_version = acting_version;\n" +
            "    codec->position_ptr = pos;\n" +
            "    *codec->position_ptr = *codec->position_ptr + %6$d;\n\n" +
            "    return codec;\n" +
            "}\n",
            groupName, cTypeForBlockLength, blockLength, cTypeForNumInGroup,
            dimensionsStructName, dimensionHeaderLength,
            minCheck,
            numInGroupToken.encoding().applicableMaxValue().longValue()));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %3$s_sbe_header_size(void)\n" +
            "{\n" +
            "    return %1$d;\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %3$s_sbe_block_length(void)\n" +
            "{\n" +
            "    return %2$d;\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %3$s_sbe_position(\n" +
            "    const %3$s *const codec)\n" +
            "{\n" +
            "    return *codec->position_ptr;\n" +
            "}\n\n" +

            "SBE_ONE_DEF bool %3$s_set_sbe_position(\n" +
            "    %3$s *const codec,\n" +
            "    const uint64_t position)\n" +
            "{\n" +
            "    if (SBE_BOUNDS_CHECK_EXPECT((position > codec->buffer_length), false))\n" +
            "    {\n" +
            "       errno = E100;\n" +
            "       return false;\n" +
            "    }\n" +
            "    *codec->position_ptr = position;\n\n" +
            "    return true;\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %3$s_count(\n" +
            "    const %3$s *const codec)\n" +
            "{\n" +
            "    return codec->count;\n" +
            "}\n\n" +

            "SBE_ONE_DEF bool %3$s_has_next(\n" +
            "    const %3$s *const codec)\n" +
            "{\n" +
            "    return codec->index + 1 < codec->count;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %3$s *%3$s_next(\n" +
            "    %3$s *const codec)\n" +
            "{\n" +
            "    codec->offset = *codec->position_ptr;\n" +
            "    if (SBE_BOUNDS_CHECK_EXPECT(((codec->offset + codec->block_length) " +
            "> codec->buffer_length), false))\n" +
            "    {\n" +
            "        errno = E108;\n" +
            "        return NULL;\n" +
            "    }\n" +
            "    *codec->position_ptr = codec->offset + codec->block_length;\n" +
            "    ++codec->index;\n\n" +

            "    return codec;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %3$s *%3$s_for_each(\n" +
            "    %3$s *const codec,\n" +
            "    void (*func)(%3$s *, void *),\n" +
            "    void *const context)\n" +
            "{\n" +
            "    while (%3$s_has_next(codec))\n" +
            "    {\n" +
            "        if (!%3$s_next(codec))\n" +
            "        {\n" +
            "            return NULL;\n" +
            "        }\n" +
            "        func(codec, context);\n" +
            "    }\n\n" +
            "    return codec;\n" +
            "}\n",
            dimensionHeaderLength, blockLength, groupName));
    }

    private static CharSequence generateGroupPropertyFunctions(
        final String outerStruct, final String groupName, final Token token, final String cTypeForNumInGroup)
    {
        final StringBuilder sb = new StringBuilder();
        final String propertyName = formatPropertyName(token.name());

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint16_t %1$s_id(void)\n" +
            "{\n" +
            "    return %2$d;\n" +
            "}\n",
            groupName,
            token.id()));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %2$s *%1$s_get_%3$s(\n" +
            "    %1$s *const codec,\n" +
            "    %2$s *const property)\n" +
            "{\n" +
            "    return %2$s_wrap_for_decode(\n" +
            "        property,\n" +
            "        codec->buffer,\n" +
            "        %1$s_sbe_position_ptr(codec),\n" +
            "        codec->acting_version,\n" +
            "        codec->buffer_length);\n" +
            "}\n",
            outerStruct,
            groupName,
            propertyName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %2$s *%2$s_set_count(\n" +
            "    %1$s *const codec,\n" +
            "    %2$s *const property,\n" +
            "    const %3$s count)\n" +
            "{\n" +
            "    return %2$s_wrap_for_encode(\n" +
            "        property,\n" +
            "        codec->buffer,\n" +
            "        count,\n" +
            "        %1$s_sbe_position_ptr(codec),\n" +
            "        codec->acting_version,\n" +
            "        codec->buffer_length);\n" +
            "}\n",
            outerStruct,
            groupName,
            cTypeForNumInGroup));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %2$s_since_version(void)\n" +
            "{\n" +
            "    return %3$d;\n" +
            "}\n\n" +

            "SBE_ONE_DEF bool %2$s_in_acting_version(\n" +
            "    const %1$s *const codec)\n" +
            "{\n" +
            "    return codec->acting_version >= %2$s_since_version();\n" +
            "}\n",
            outerStruct,
            groupName,
            token.version()));

        return sb;
    }

    private CharSequence generateVarData(
        final String structName, final String outermostStruct, final List<Token> tokens)
    {
        final StringBuilder sb = new StringBuilder();

        for (int i = 0, size = tokens.size(); i < size;)
        {
            final Token token = tokens.get(i);
            if (token.signal() != Signal.BEGIN_VAR_DATA)
            {
                throw new IllegalStateException("tokens must begin with BEGIN_VAR_DATA: token=" + token);
            }

            final String propertyName = formatPropertyName(token.name());
            final Token lengthToken = Generators.findFirst("length", tokens, i);
            final Token varDataToken = Generators.findFirst("varData", tokens, i);
            final String characterEncoding = varDataToken.encoding().characterEncoding();
            final int lengthOfLengthField = lengthToken.encodedLength();
            final String lengthCType = cTypeName(lengthToken.encoding().primitiveType());
            final String lengthByteOrderStr = formatByteOrderEncoding(
                lengthToken.encoding().byteOrder(), lengthToken.encoding().primitiveType());

            generateFieldMetaAttributeFunction(sb, token, structName, outermostStruct);

            generateVarDataDescriptors(
                sb, token, propertyName, characterEncoding, lengthToken,
                lengthOfLengthField, lengthCType, structName);

            sb.append(String.format("\n" +
                "SBE_ONE_DEF const char *%6$s_%1$s(\n" +
                "    %6$s *const codec)\n" +
                "{\n" +
                "%2$s" +
                "    %4$s length_field_value;\n" +
                "    memcpy(&length_field_value, codec->buffer + %6$s_sbe_position(codec), sizeof(%4$s));\n" +
                "    const char *field_ptr = (codec->buffer + %6$s_sbe_position(codec) + %3$d);\n\n" +
                "    if (!%6$s_set_sbe_position(\n" +
                "        codec, %6$s_sbe_position(codec) + %3$d + %5$s(length_field_value)))\n" +
                "    {\n" +
                "        return NULL;\n" +
                "    }\n\n" +
                "    return field_ptr;\n" +
                "}\n",
                propertyName,
                generateFieldNotPresentCondition(token.version(), "NULL"),
                lengthOfLengthField,
                lengthCType,
                lengthByteOrderStr,
                structName));

            sb.append(String.format("\n" +
                "SBE_ONE_DEF uint64_t %6$s_get_%1$s(\n" +
                "    %6$s *const codec,\n" +
                "    char *dst,\n" +
                "    const uint64_t length)\n" +
                "{\n" +
                "%2$s" +
                "    uint64_t length_of_length_field = %3$d;\n" +
                "    uint64_t length_position = %6$s_sbe_position(codec);\n" +
                "    if (!%6$s_set_sbe_position(codec, length_position + length_of_length_field))\n" +
                "    {\n" +
                "        return 0;\n" +
                "    }\n\n" +
                "    %5$s length_field_value;\n" +
                "    memcpy(&length_field_value, codec->buffer + length_position, sizeof(%5$s));\n" +
                "    uint64_t data_length = %4$s(length_field_value);\n" +
                "    uint64_t bytes_to_copy = length < data_length ? length : data_length;\n" +
                "    uint64_t pos = %6$s_sbe_position(codec);\n\n" +
                "    if (!%6$s_set_sbe_position(codec, pos + data_length))\n" +
                "    {\n" +
                "        return 0;\n" +
                "    }\n\n" +
                "    memcpy(dst, codec->buffer + pos, (size_t)bytes_to_copy);\n\n" +
                "    return bytes_to_copy;\n" +
                "}\n",
                propertyName,
                generateFieldNotPresentCondition(token.version(), "0"),
                lengthOfLengthField,
                lengthByteOrderStr,
                lengthCType,
                structName));

            sb.append(String.format("\n" +
                "SBE_ONE_DEF sbe_string_view %5$s_get_%1$s_as_string_view(\n" +
                "    %5$s *const codec)\n" +
                "{\n" +
                "    sbe_string_view ret = {NULL, 0};\n" +
                "%2$s" +
                "    %4$s length_field_value = %5$s_%1$s_length(codec);\n" +
                "    char *field_ptr = codec->buffer + %5$s_sbe_position(codec) + %3$d;\n" +
                "    if (%5$s_set_sbe_position(\n" +
                "        codec, %5$s_sbe_position(codec) + %3$d + length_field_value))\n" +
                "    {\n" +
                "        ret.data = field_ptr;\n" +
                "        ret.length = length_field_value;\n" +
                "    }\n" +
                "    return ret;\n" +
                "}\n",
                propertyName,
                generateFieldNotPresentCondition(token.version(), "ret"),
                lengthOfLengthField,
                lengthCType,
                structName,
                outermostStruct));

            sb.append(String.format("\n" +
                "SBE_ONE_DEF %5$s *%5$s_put_%1$s(\n" +
                "    %5$s *const codec,\n" +
                "    const char *src,\n" +
                "    const uint64_t length)\n" +
                "{\n" +
                "    uint64_t length_of_length_field = %2$d;\n" +
                "    uint64_t length_position = %5$s_sbe_position(codec);\n" +
                "    %3$s length_field_value = %4$s(length);\n" +
                "    if (!%5$s_set_sbe_position(codec, length_position + length_of_length_field))\n" +
                "    {\n" +
                "        return NULL;\n" +
                "    }\n\n" +
                "    memcpy(codec->buffer + length_position, &length_field_value, sizeof(%3$s));\n" +
                "    uint64_t pos = %5$s_sbe_position(codec);\n\n" +
                "    if (!%5$s_set_sbe_position(codec, pos + length))\n" +
                "    {\n" +
                "        return NULL;\n" +
                "    }\n\n" +
                "    memcpy(codec->buffer + pos, src, (size_t)length);\n\n" +
                "    return codec;\n" +
                "}\n",
                propertyName,
                lengthOfLengthField,
                lengthCType,
                lengthByteOrderStr,
                structName));

            i += token.componentTokenCount();
        }

        return sb;
    }

    private void generateVarDataDescriptors(
        final StringBuilder sb,
        final Token token,
        final String propertyName,
        final String characterEncoding,
        final Token lengthToken,
        final Integer sizeOfLengthField,
        final String lengthCType,
        final String structName)
    {
        final String fullyQualifiedPropertyName = structName + "_" + propertyName;

        sb.append(String.format("\n" +
            "SBE_ONE_DEF const char *%s_character_encoding(void)\n" +
            "{\n" +
            "    return \"%s\";\n" +
            "}\n",
            fullyQualifiedPropertyName,
            characterEncoding));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %1$s_since_version(void)\n" +
            "{\n" +
            "    return %2$d;\n" +
            "}\n\n" +

            "SBE_ONE_DEF bool %1$s_in_acting_version(\n" +
            "    const %4$s *const codec)\n" +
            "{\n" +
            "    return codec->acting_version >= %1$s_since_version();\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint16_t %1$s_id(void)\n" +
            "{\n" +
            "    return %3$d;\n" +
            "}\n",
            fullyQualifiedPropertyName,
            token.version(),
            token.id(),
            structName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %s_header_length(void)\n" +
            "{\n" +
            "    return %d;\n" +
            "}\n",
            fullyQualifiedPropertyName,
            sizeOfLengthField));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %1$s_length(\n" +
            "    const %5$s *const codec)\n" +
            "{\n" +
            "%2$s" +
            "    %4$s length;\n" +
            "    memcpy(&length, codec->buffer + %5$s_sbe_position(codec), sizeof(%4$s));\n\n" +
            "    return %3$s(length);\n" +
            "}\n",
            fullyQualifiedPropertyName,
            generateFieldNotPresentCondition(token.version(), "0"),
            formatByteOrderEncoding(lengthToken.encoding().byteOrder(), lengthToken.encoding().primitiveType()),
            lengthCType,
            structName));
    }

    private void generateChoiceSet(
        final Writer out,
        final CharSequence[] scope,
        final List<Token> tokens)
        throws IOException
    {
        final Token bitsetToken = tokens.get(0);
        {
            final String bitSetName = formatScopedName(scope, bitsetToken.applicableTypeName());
            out.append(generateFixedFlyweightStruct(bitSetName));
            out.append(generateFixedFlyweightCodeFunctions(bitSetName, bitsetToken.encodedLength()));

            out.append(String.format("\n" +
                "SBE_ONE_DEF %1$s *%1$s_clear(\n" +
                "    %1$s *const codec)\n" +
                "{\n" +
                "    %2$s zero = 0;\n" +
                "    memcpy(codec->buffer + codec->offset, &zero, sizeof(%2$s));\n\n" +
                "    return codec;\n" +
                "}\n",
                bitSetName,
                cTypeName(bitsetToken.encoding().primitiveType())));

            out.append(String.format("\n" +
                "SBE_ONE_DEF bool %1$s_is_empty(\n" +
                "    const %1$s *const codec)\n" +
                "{\n" +
                "    %2$s val;\n" +
                "    memcpy(&val, codec->buffer + codec->offset, sizeof(%2$s));\n\n" +
                "    return 0 == val;\n" +
                "}\n",
                bitSetName,
                cTypeName(bitsetToken.encoding().primitiveType())));

            out.append(generateChoices(bitSetName, bitsetToken, tokens.subList(1, tokens.size() - 1)));
        }
    }

    private void generateEnum(
        final Writer out,
        final CharSequence[] scope, final List<Token> tokens) throws IOException
    {
        final Token enumToken = tokens.get(0);

        {
            out.append(generateEnumValues(scope, tokens.subList(1, tokens.size() - 1), enumToken));

            out.append(generateEnumLookupFunction(scope, tokens.subList(1, tokens.size() - 1), enumToken));
        }
    }

    private void generateComposite(
        final Writer out,
        final CharSequence[] scope, final List<Token> tokens) throws IOException
    {
        final Token compositeToken = tokens.get(0);
        {
            final String compositeName = formatScopedName(scope, compositeToken.applicableTypeName());

            out.append(generateFixedFlyweightStruct(compositeName));
            out.append(generateFixedFlyweightCodeFunctions(compositeName, compositeToken.encodedLength()));
            out.append(generateCompositePropertyFunctions(
                scope, compositeName, tokens.subList(1, tokens.size() - 1)));
        }
    }

    private CharSequence generateChoices(
        final String bitsetStructName, final Token bitsetToken, final List<Token> tokens)
    {
        final StringBuilder sb = new StringBuilder();

        tokens
            .stream()
            .filter((token) -> token.signal() == Signal.CHOICE)
            .forEach((token) ->
            {
                final String choiceName = formatPropertyName(token.name());
                final String bitsetTypeName = cTypeName(bitsetToken.encoding().primitiveType());

                final Encoding encoding = token.encoding();

                final long lsb = encoding.isChoice() ?
                    encoding.constValue().longValue() : encoding.lsbValue().longValue();
                final long msb = encoding.isChoice() ?
                    encoding.constValue().longValue() : encoding.msbValue().longValue();
                final String byteOrderStr = formatByteOrderEncoding(
                    bitsetToken.encoding().byteOrder(), bitsetToken.encoding().primitiveType());
                final PrimitiveType bitsetPrimitiveType = bitsetToken.encoding().primitiveType();

                final GenerateLiteralFunction genBitsLiteral = (PrimitiveType a, String b) -> generateLiteral(a, b);
                CharSequence bitsGetNormal = generateGetBits(
                    lsb, msb, bitsetPrimitiveType, "bitset", "", genBitsLiteral);
                CharSequence bitsGetByteOrder = generateGetBits(
                    lsb, msb, bitsetPrimitiveType, "bitset", byteOrderStr, genBitsLiteral);
                final CharSequence bitsSetNormal = generateSetBits(
                    lsb, msb, bitsetPrimitiveType, "bits", "bitset", "", genBitsLiteral);
                final CharSequence bitsSetByteOrder = generateSetBits(
                    lsb, msb, bitsetPrimitiveType, "bits", "bitset", byteOrderStr, genBitsLiteral);

                final CharSequence nullReturn;
                final String choiceTypeName;
                if (encoding.isChoice())
                {
                    nullReturn = "false";
                    choiceTypeName = "bool";
                }
                else
                {
                    if (token.referencedName() != null)
                    {
                        final String enumName = formatScopedName(ir.namespaces(), token.applicableTypeName());
                        nullReturn = enumName + "_NULL_VALUE";
                        choiceTypeName = enumName;
                    }
                    else
                    {
                        nullReturn = generateLiteral(encoding.primitiveType(),
                            encoding.applicableNullValue().toString());
                        choiceTypeName = cTypeName(encoding.primitiveType());
                    }
                }
                bitsGetNormal = String.format("(%1$s)(%2$s)", choiceTypeName, bitsGetNormal);
                bitsGetByteOrder = String.format("(%1$s)(%2$s)", choiceTypeName, bitsGetByteOrder);
                final CharSequence notPresent = generateFieldNotPresentCondition(token.version(), nullReturn);
                sb.append(String.format("\n" +
                    "SBE_ONE_DEF %5$s %1$s_%2$s_get(\n" +
                    "    const %3$s bitset)\n" +
                    "{\n" +
                    "    return %6$s;\n" +
                    "}\n\n" +

                    "SBE_ONE_DEF %3$s %1$s_%2$s_apply(\n" +
                    "    const %3$s bitset,\n" +
                    "    const %5$s bits)\n" +
                    "{\n" +
                    "    return %7$s;\n" +
                    "}\n\n" +

                    "SBE_ONE_DEF %5$s %1$s_%2$s(\n" +
                    "    const %1$s *const codec)\n" +
                    "{\n" +
                    "    %3$s bitset;\n" +
                    "%4$s" +
                    "    memcpy(&bitset, codec->buffer + codec->offset, sizeof(%3$s));\n\n" +

                    "    return %8$s;\n" +
                    "}\n\n" +

                    "SBE_ONE_DEF %1$s *%1$s_%2$s_set(\n" +
                    "    %1$s *const codec,\n" +
                    "    const %5$s bits)\n" +
                    "{\n" +
                    "    %3$s bitset;\n" +
                    "    memcpy(&bitset, codec->buffer + codec->offset, sizeof(%3$s));\n" +
                    "    bitset = %9$s;\n" +
                    "    memcpy(codec->buffer + codec->offset, &bitset, sizeof(%3$s));\n\n" +

                    "    return codec;\n" +
                    "}\n",
                    bitsetStructName,
                    choiceName,
                    bitsetTypeName,
                    notPresent,
                    choiceTypeName,
                    bitsGetNormal,
                    bitsSetNormal,
                    bitsGetByteOrder,
                    bitsSetByteOrder
                ));
            });

        return sb;
    }

    private CharSequence generateEnumValues(
        final CharSequence[] scope, final List<Token> tokens, final Token encodingToken)
    {
        final StringBuilder sb = new StringBuilder();
        final Encoding encoding = encodingToken.encoding();
        final String enumName = formatScopedName(scope, encodingToken.applicableTypeName());

        sb.append(String.format("\n" +
            "enum %1$s\n" +
            "{\n",
            enumName));

        for (final Token token : tokens)
        {
            sb.append(String.format(
                "    %s_%s = %s,\n",
                enumName,
                token.name(),
                generateLiteral(token.encoding())
            ));
        }

        sb.append(String.format(
            "    %s_NULL_VALUE = %s\n",
            enumName,
            generateLiteral(encoding.primitiveType(), encoding.applicableNullValue().toString())));

        sb.append("};\n\n");

        return sb;
    }

    private static CharSequence generateEnumLookupFunction(
        final CharSequence[] scope, final List<Token> tokens, final Token encodingToken)
    {
        final String enumName = formatScopedName(scope, encodingToken.applicableTypeName());
        final StringBuilder sb = new StringBuilder();

        for (final Token token : tokens)
        {
            sb.append(String.format(
                "        case %s_%s:\n" +
                "             return true;\n",
                enumName,
                token.name()
            ));
        }


        return String.format(
            "SBE_ONE_DEF bool %1$s_is_normal(\n" +
            "    const %2$s value)\n" +
            "{\n" +
            "    switch (value)\n" +
            "    {\n" +
            "%3$s" +
            "        default:\n" +
            "             return false;\n" +
            "    }\n" +
            "}\n\n" +

            "SBE_ONE_DEF bool %1$s_is_null(\n" +
            "    const %2$s value)\n" +
            "{\n" +

            "    return value == %1$s_NULL_VALUE;\n" +
            "}\n\n" +

            "SBE_ONE_DEF bool %1$s_get(\n" +
            "    const %2$s value,\n" +
            "    enum %1$s *const out)\n" +
            "{\n" +
            "    if (%1$s_is_normal(value) || %1$s_is_null(value))\n" +
            "    {\n" +
            "        *out = (enum %1$s)value;\n" +
            "        return true;\n" +
            "    }\n" +
            "    *out =  %1$s_NULL_VALUE;\n" +
            "    errno = E103;\n" +
            "    return false;\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %1$s_encoded_length(void)\n" +
            "{\n" +
            "    return %4$s;\n" +
            "}\n",
            enumName,
            cTypeName(encodingToken.encoding().primitiveType()),
            sb,
            encodingToken.encoding().primitiveType().size()
        );
    }

    private CharSequence generateFieldNotPresentCondition(final int sinceVersion, final CharSequence nullReturn)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            "    if (codec->acting_version < %1$d)\n" +
            "    {\n" +
            "        return %2$s;\n" +
            "    }\n\n",
            sinceVersion, nullReturn);
    }

    private static CharSequence generateFileHeader(final String structName, final List<String> typesToInclude)
    {
        final StringBuilder sb = new StringBuilder();
        sb.append(generateSbecHeader());

        sb.append("/* Generated SBE (Simple Binary Encoding) message codec */\n");

        sb.append(String.format("\n" +
            "#ifndef _%1$s_H_\n" +
            "#define _%1$s_H_\n\n",
            structName.toUpperCase()));

        if (typesToInclude != null && typesToInclude.size() != 0)
        {
            sb.append("\n");
            for (final String incName : typesToInclude)
            {
                sb.append(String.format("#include \"%1$s.h\"\n", toLowerFirstChar(incName)));
            }
        }

        return sb;
    }

    private void generatePropertyFunctions(
        final StringBuilder sb,
        final CharSequence[] scope,
        final String containingStructName,
        final String outermostStruct,
        final Token signalToken,
        final String propertyName,
        final Token encodingToken)
    {
        switch (encodingToken.signal())
        {
            case ENCODING:
                generatePrimitiveProperty(
                    sb,
                    containingStructName,
                    outermostStruct,
                    propertyName,
                    encodingToken);
                break;

            case BEGIN_ENUM:
                generateEnumProperty(
                    sb,
                    scope,
                    containingStructName,
                    signalToken,
                    propertyName,
                    encodingToken);
                break;

            case BEGIN_SET:
                generateBitsetPropertyFunctions(
                    sb,
                    scope,
                    propertyName,
                    encodingToken,
                    containingStructName);
                break;

            case BEGIN_COMPOSITE:
                generateCompositePropertyFunction(
                    sb,
                    scope,
                    propertyName,
                    encodingToken,
                    containingStructName);
                break;

            default:
                break;
        }
    }

    private CharSequence generateCompositePropertyFunctions(
        final CharSequence[] scope, final String containingStructName, final List<Token> tokens)
    {
        final StringBuilder sb = new StringBuilder();

        for (int i = 0; i < tokens.size();)
        {
            final Token fieldToken = tokens.get(i);
            final String propertyName = formatPropertyName(fieldToken.name());

            generateFieldMetaAttributeFunction(
                sb,
                fieldToken,
                containingStructName,
                containingStructName);

            generateFieldCommonFunctions(
                sb,
                fieldToken,
                fieldToken,
                propertyName,
                containingStructName);

            generatePropertyFunctions(
                sb,
                scope,
                containingStructName,
                containingStructName,
                fieldToken,
                propertyName,
                fieldToken);

            i += tokens.get(i).componentTokenCount();
        }

        return sb;
    }

    private void generatePrimitiveProperty(
        final StringBuilder sb,
        final String containingStructName,
        final String outermostStruct,
        final String propertyName,
        final Token token)
    {
        sb.append(generatePrimitiveFieldMetaData(propertyName, token, containingStructName));

        if (token.isConstantEncoding())
        {
            sb.append(generateConstPropertyFunctions(propertyName, token, containingStructName));
        }
        else
        {
            sb.append(generatePrimitivePropertyFunctions(containingStructName, outermostStruct, propertyName, token));
        }
    }

    private CharSequence generatePrimitivePropertyFunctions(
        final String containingStructName, final String outermostStruct, final String propertyName, final Token token)
    {
        final int arrayLength = token.arrayLength();

        if (arrayLength == 1)
        {
            return generateSingleValueProperty(containingStructName, outermostStruct, propertyName, token);
        }
        else if (arrayLength > 1)
        {
            return generateArrayProperty(containingStructName, outermostStruct, propertyName, token);
        }

        return "";
    }

    private CharSequence generatePrimitiveFieldMetaData(
        final String propertyName, final Token token, final String containingStructName)
    {
        final StringBuilder sb = new StringBuilder();

        final Encoding encoding = token.encoding();
        final PrimitiveType primitiveType = encoding.primitiveType();
        final String cTypeName = cTypeName(primitiveType);
        final CharSequence nullValueString = generateNullValueLiteral(primitiveType, encoding);

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s %4$s_%2$s_null_value(void)\n" +
            "{\n" +
            "    return %3$s;\n" +
            "}\n",
            cTypeName,
            propertyName,
            nullValueString,
            containingStructName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s %4$s_%2$s_min_value(void)\n" +
            "{\n" +
            "    return %3$s;\n" +
            "}\n",
            cTypeName,
            propertyName,
            generateLiteral(primitiveType, token.encoding().applicableMinValue().toString()),
            containingStructName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s %4$s_%2$s_max_value(void)\n" +
            "{\n" +
            "    return %3$s;\n" +
            "}\n",
            cTypeName,
            propertyName,
            generateLiteral(primitiveType, token.encoding().applicableMaxValue().toString()),
            containingStructName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %3$s_%1$s_encoded_length(void)\n" +
            "{\n" +
            "    return %2$d;\n" +
            "}\n",
            propertyName,
            token.encoding().primitiveType().size() * token.arrayLength(),
            containingStructName));

        return sb;
    }

    private CharSequence generateLoadValue(
        final String outermostStruct,
        final PrimitiveType primitiveType,
        final String offsetStr,
        final ByteOrder byteOrder,
        final String outputHandler)
    {
        final String cTypeName = cTypeName(primitiveType);
        final String byteOrderStr = formatByteOrderEncoding(byteOrder, primitiveType);
        final StringBuilder sb = new StringBuilder();

        if (primitiveType == PrimitiveType.FLOAT || primitiveType == PrimitiveType.DOUBLE)
        {
            final String stackUnion = primitiveType == PrimitiveType.FLOAT ? "float" : "double";

            sb.append(String.format(
                "    union sbe_%2$s_as_uint val;\n" +
                "    memcpy(&val, codec->buffer + codec->offset + %3$s, sizeof(%4$s));\n" +
                "    val.uint_value = %5$s(val.uint_value);\n" +
                "    %6$s val.fp_value;",
                outermostStruct,
                stackUnion,
                offsetStr,
                cTypeName,
                byteOrderStr,
                outputHandler));
        }
        else
        {
            sb.append(String.format(
                "    %1$s val;\n" +
                "    memcpy(&val, codec->buffer + codec->offset + %2$s, sizeof(%1$s));\n" +
                "    %4$s %3$s(val);",
                cTypeName,
                offsetStr,
                byteOrderStr,
                outputHandler));
        }

        return sb;
    }

    private CharSequence generateLoadValue(
        final String outermostStruct,
        final PrimitiveType primitiveType,
        final String offsetStr,
        final ByteOrder byteOrder)
    {
        return generateLoadValue(outermostStruct, primitiveType, offsetStr, byteOrder, "return");
    }

    private CharSequence generateLoadValueUnsafe(
        final String outermostStruct,
        final PrimitiveType primitiveType,
        final String offsetStr,
        final ByteOrder byteOrder)
    {
        return generateLoadValue(outermostStruct, primitiveType, offsetStr, byteOrder, "*out =");
    }

    private CharSequence generateStoreValue(
        final String outermostStruct,
        final PrimitiveType primitiveType,
        final String offsetStr,
        final ByteOrder byteOrder)
    {
        final String cTypeName = cTypeName(primitiveType);
        final String byteOrderStr = formatByteOrderEncoding(byteOrder, primitiveType);
        final StringBuilder sb = new StringBuilder();

        if (primitiveType == PrimitiveType.FLOAT || primitiveType == PrimitiveType.DOUBLE)
        {
            final String stackUnion = primitiveType == PrimitiveType.FLOAT ? "float" : "double";

            sb.append(String.format(
                "    union sbe_%2$s_as_uint val;\n" +
                "    val.fp_value = value;\n" +
                "    val.uint_value = %3$s(val.uint_value);\n" +
                "    memcpy(codec->buffer + codec->offset + %4$s, &val, sizeof(%5$s));",
                outermostStruct,
                stackUnion,
                byteOrderStr,
                offsetStr,
                cTypeName));
        }
        else
        {
            sb.append(String.format(
                "    %1$s val = %2$s(value);\n" +
                "    memcpy(codec->buffer + codec->offset + %3$s, &val, sizeof(%1$s));\n",
                cTypeName,
                byteOrderStr,
                offsetStr));
        }

        return sb;
    }

    private CharSequence generateSingleValueProperty(
        final String containingStructName, final String outermostStruct, final String propertyName, final Token token)
    {
        final PrimitiveType primitiveType = token.encoding().primitiveType();
        final String cTypeName = cTypeName(primitiveType);
        final int offset = token.offset();
        final StringBuilder sb = new StringBuilder();

        final CharSequence loadValue = generateLoadValue(
            outermostStruct,
            primitiveType,
            Integer.toString(offset),
            token.encoding().byteOrder());

        final CharSequence nullReturn = generateLiteral(primitiveType,
            token.encoding().applicableNullValue().toString());
        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s %5$s_%2$s(\n" +
            "    const %5$s *const codec)\n" +
            "{\n" +
            "%3$s" +
            "%4$s\n" +
            "}\n",
            cTypeName,
            propertyName,
            generateFieldNotPresentCondition(token.version(), nullReturn),
            loadValue,
            containingStructName));

        final CharSequence storeValue = generateStoreValue(
            outermostStruct, primitiveType, Integer.toString(offset), token.encoding().byteOrder());

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s *%1$s_set_%2$s(\n" +
            "    %1$s *const codec,\n" +
            "    const %3$s value)\n" +
            "{\n" +
            "%4$s\n" +
            "    return codec;\n" +
            "}\n",
            containingStructName,
            propertyName,
            cTypeName,
            storeValue));

        return sb;
    }

    private CharSequence generateArrayProperty(
        final String containingStructName, final String outermostStruct, final String propertyName, final Token token)
    {
        final PrimitiveType primitiveType = token.encoding().primitiveType();
        final String cTypeName = cTypeName(primitiveType);
        final int offset = token.offset();

        final StringBuilder sb = new StringBuilder();

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %1$s_%2$s_length(void)\n" +
            "{\n" +
            "    return %3$d;\n" +
            "}\n",
            containingStructName,
            propertyName,
            token.arrayLength()));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF const char *%1$s_%2$s_buffer(\n" +
            "    const %1$s *const codec)\n" +
            "{\n" +
            "%3$s" +
            "    return codec->buffer + codec->offset + %4$d;\n" +
            "}\n",
            containingStructName,
            propertyName,
            generateFieldNotPresentCondition(token.version(), "NULL"),
            offset));

        final CharSequence loadValue = generateLoadValue(
            outermostStruct,
            primitiveType,
            String.format("%d + (index * %d)", offset, primitiveType.size()),
            token.encoding().byteOrder());

        final CharSequence nullReturn = generateLiteral(primitiveType,
            token.encoding().applicableNullValue().toString());
        sb.append(String.format("\n" +
            "SBE_ONE_DEF %2$s %1$s_%3$s_unsafe(\n" +
            "    const %1$s *const codec,\n" +
            "    const uint64_t index)\n" +
            "{\n" +
            "%4$s" +
            "%5$s\n" +
            "}\n",
            containingStructName,
            cTypeName,
            propertyName,
            generateFieldNotPresentCondition(token.version(), nullReturn),
            loadValue));

        final CharSequence loadValueUnsafe = generateLoadValueUnsafe(
            outermostStruct,
            primitiveType,
            String.format("%d + (index * %d)", offset, primitiveType.size()),
            token.encoding().byteOrder());

        sb.append(String.format("\n" +
            "SBE_ONE_DEF bool %1$s_%3$s(\n" +
            "    const %1$s *const codec,\n" +
            "    const uint64_t index,\n" +
            "    %2$s *const out)\n" +
            "{\n" +
            "    if (index >= %4$d)\n" +
            "    {\n" +
            "        errno = E104;\n" +
            "        return false;\n" +
            "    }\n\n" +

            "%5$s" +
            "%6$s\n" +
            "    return true;\n" +
            "}\n",
            containingStructName,
            cTypeName,
            propertyName,
            token.arrayLength(),
            generateFieldNotPresentCondition(token.version(), "false"),
            loadValueUnsafe));

        final CharSequence storeValue = generateStoreValue(
            outermostStruct,
            primitiveType,
            String.format("%d + (index * %d)", offset, primitiveType.size()),
            token.encoding().byteOrder());

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s *%1$s_set_%2$s_unsafe(\n" +
            "    %1$s *const codec,\n" +
            "    const uint64_t index,\n" +
            "    const %3$s value)\n" +
            "{\n" +
            "%4$s\n" +
            "    return codec;\n" +
            "}\n",
            containingStructName,
            propertyName,
            cTypeName,
            storeValue));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s *%1$s_set_%2$s(\n" +
            "    %1$s *const codec,\n" +
            "    const uint64_t index,\n" +
            "    const %3$s value)\n" +
            "{\n" +
            "    if (index >= %4$d)\n" +
            "    {\n" +
            "        errno = E105;\n" +
            "        return NULL;\n" +
            "    }\n\n" +

            "%5$s\n" +
            "    return codec;\n" +
            "}\n",
            containingStructName,
            propertyName,
            cTypeName,
            token.arrayLength(),
            storeValue));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF char *%1$s_get_%2$s(\n" +
            "    const %1$s *const codec,\n" +
            "    char *dst,\n" +
            "    const uint64_t length)\n" +
            "{\n" +
            "    if (length > %3$d)\n" +
            "    {\n" +
            "        errno = E106;\n" +
            "        return NULL;\n" +
            "    }\n\n" +

            "%4$s" +
            "    memcpy(dst, codec->buffer + codec->offset + %5$d, sizeof(%6$s) * (size_t)length);\n\n" +
            "    return dst;\n" +
            "}\n",
            containingStructName,
            propertyName,
            token.arrayLength(),
            generateFieldNotPresentCondition(token.version(), "NULL"),
            offset,
            cTypeName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s *%1$s_put_%2$s(\n" +
            "    %1$s *const codec,\n" +
            "    const char *src)\n" +
            "{\n" +
            "    memcpy(codec->buffer + codec->offset + %3$d, src, sizeof(%4$s) * %5$d);\n\n" +
            "    return codec;\n" +
            "}\n",
            containingStructName,
            propertyName,
            offset,
            cTypeName,
            token.arrayLength()));

        return sb;
    }

    private CharSequence generateConstPropertyFunctions(
        final String propertyName, final Token token, final String containingStructName)
    {
        final String cTypeName = cTypeName(token.encoding().primitiveType());

        if (token.encoding().primitiveType() != PrimitiveType.CHAR)
        {
            return String.format("\n" +
                "SBE_ONE_DEF %1$s %4$s_%2$s(void)\n" +
                "{\n" +
                "    return %3$s;\n" +
                "}\n",
                cTypeName,
                propertyName,
                generateLiteral(token.encoding()),
                containingStructName);
        }

        final StringBuilder sb = new StringBuilder();

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

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %3$s_%1$s_length(void)\n" +
            "{\n" +
            "    return %2$d;\n" +
            "}\n",
            propertyName,
            constantValue.length,
            containingStructName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF const char *%3$s_%1$s(void)\n" +
            "{\n" +
            "    static uint8_t %1$s_values[] = {%2$s};\n\n" +

            "    return (const char *)%1$s_values;\n" +
            "}\n",
            propertyName,
            values,
            containingStructName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s %4$s_%2$s_index(const uint64_t index)\n" +
            "{\n" +
            "    static uint8_t %2$s_values[] = {%3$s};\n\n" +

            "    return %2$s_values[index];\n" +
            "}\n",
            cTypeName,
            propertyName,
            values,
            containingStructName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %4$s_get_%1$s(\n" +
            "    const %4$s *const codec,\n" +
            "    char *dst,\n" +
            "    const uint64_t length)\n" +
            "{\n" +
            "    static uint8_t %2$s_values[] = {%3$s};\n" +
            "    uint64_t bytes_to_copy = length < sizeof(%2$s_values) ? length : sizeof(%2$s_values);\n\n" +

            "    memcpy(dst, %2$s_values, (size_t)bytes_to_copy);\n\n" +
            "    return bytes_to_copy;\n" +
            "}\n",
            toUpperFirstChar(propertyName),
            propertyName,
            values,
            containingStructName));

        return sb;
    }

    private CharSequence generateFixedFlyweightStruct(final String structName)
    {
        return String.format("\n" +
            "typedef struct sbe_composite %1$s;\n",
            structName);
    }

    private CharSequence generateFixedFlyweightCodeFunctions(final String structName, final int size)
    {
        final String schemaIdType = cTypeName(ir.headerStructure().schemaIdType());
        final String schemaVersionType = cTypeName(ir.headerStructure().schemaVersionType());

        return String.format("\n" +
            "SBE_ONE_DEF %1$s *%1$s_reset(\n" +
            "    %1$s *const codec,\n" +
            "    char *buffer,\n" +
            "    const uint64_t offset,\n" +
            "    const uint64_t buffer_length,\n" +
            "    const uint64_t acting_version)\n" +
            "{\n" +
            "    if (SBE_BOUNDS_CHECK_EXPECT(((offset + %2$s) > buffer_length), false))\n" +
            "    {\n" +
            "        errno = E107;\n" +
            "        return NULL;\n" +
            "    }\n" +
            "    codec->buffer = buffer;\n" +
            "    codec->buffer_length = buffer_length;\n" +
            "    codec->offset = offset;\n" +
            "    codec->acting_version = acting_version;\n\n" +
            "    return codec;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %1$s *%1$s_wrap(\n" +
            "    %1$s *const codec,\n" +
            "    char *buffer,\n" +
            "    const uint64_t offset,\n" +
            "    const uint64_t acting_version,\n" +
            "    const uint64_t buffer_length)\n" +
            "{\n" +
            "    return %1$s_reset(codec, buffer, offset, buffer_length, acting_version);\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %1$s_encoded_length(void)\n" +
            "{\n" +
            "    return %2$s;\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %1$s_offset(\n" +
            "    const %1$s *const codec)\n" +
            "{\n" +
            "    return codec->offset;\n" +
            "}\n\n" +

            "SBE_ONE_DEF const char *%1$s_buffer(\n" +
            "    const %1$s *const codec)\n" +
            "{\n" +
            "    return codec->buffer;\n" +
            "}\n\n" +

            "SBE_ONE_DEF char *%1$s_mut_buffer(\n" +
            "    %1$s *const codec)\n" +
            "{\n" +
            "    return codec->buffer;\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %1$s_buffer_length(\n" +
            "    const %1$s *const codec)\n" +
            "{\n" +
            "    return codec->buffer_length;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %3$s %1$s_sbe_schema_id(void)\n" +
            "{\n" +
            "    return %4$s;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %5$s %1$s_sbe_schema_version(void)\n" +
            "{\n" +
            "    return %6$s;\n" +
            "}\n",
            structName,
            size,
            schemaIdType,
            generateLiteral(ir.headerStructure().schemaIdType(), Integer.toString(ir.id())),
            schemaVersionType,
            generateLiteral(ir.headerStructure().schemaVersionType(), Integer.toString(ir.version())));
    }

    private CharSequence generateMessageFlyweightStruct(final String structName)
    {
        return String.format("\n" +
            "typedef struct sbe_message %1$s;\n",
            structName);
    }

    private CharSequence generateMessageFlyweightFunctions(
        final String structName, final Token token, final CharSequence[] scope)
    {
        final String blockLengthType = cTypeName(ir.headerStructure().blockLengthType());
        final String templateIdType = cTypeName(ir.headerStructure().templateIdType());
        final String schemaIdType = cTypeName(ir.headerStructure().schemaIdType());
        final String schemaVersionType = cTypeName(ir.headerStructure().schemaVersionType());
        final String semanticType = token.encoding().semanticType() == null ? "" : token.encoding().semanticType();
        final String messageHeaderStruct = formatScopedName(scope, "messageHeader");

        return String.format("\n" +
            "SBE_ONE_DEF uint64_t %10$s_sbe_position(\n" +
            "    const %10$s *const codec)\n" +
            "{\n" +
            "    return codec->position;\n" +
            "}\n\n" +

            "SBE_ONE_DEF bool %10$s_set_sbe_position(\n" +
            "    %10$s *const codec,\n" +
            "    const uint64_t position)\n" +
            "{\n" +
            "    if (SBE_BOUNDS_CHECK_EXPECT((position > codec->buffer_length), false))\n" +
            "    {\n" +
            "        errno = E100;\n" +
            "        return false;\n" +
            "    }\n" +
            "    codec->position = position;\n\n" +
            "    return true;\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t *%10$s_sbe_position_ptr(\n" +
            "    %10$s *const codec)\n" +
            "{\n" +
            "    return &codec->position;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %10$s *%10$s_reset(\n" +
            "    %10$s *const codec,\n" +
            "    char *buffer,\n" +
            "    const uint64_t offset,\n" +
            "    const uint64_t buffer_length,\n" +
            "    const uint64_t acting_block_length,\n" +
            "    const uint64_t acting_version)\n" +
            "{\n" +
            "    codec->buffer = buffer;\n" +
            "    codec->offset = offset;\n" +
            "    codec->buffer_length = buffer_length;\n" +
            "    codec->acting_version = acting_version;\n" +
            "    if (!%10$s_set_sbe_position(codec, offset + acting_block_length))\n" +
            "    {\n" +
            "        return NULL;\n" +
            "    }\n\n" +
            "    return codec;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %10$s *%10$s_copy(\n" +
            "    %10$s *const codec,\n" +
            "    const %10$s *const other)\n" +
            "{\n" +
            "     codec->buffer = other->buffer;\n" +
            "     codec->offset = other->offset;\n" +
            "     codec->buffer_length = other->buffer_length;\n" +
            "     codec->acting_version = other->acting_version;\n" +
            "     codec->position = other->position;\n\n" +
            "     return codec;\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %10$s_sbe_block_length(void)\n" +
            "{\n" +
            "    return %2$s;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %3$s %10$s_sbe_template_id(void)\n" +
            "{\n" +
            "    return %4$s;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %5$s %10$s_sbe_schema_id(void)\n" +
            "{\n" +
            "    return %6$s;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %7$s %10$s_sbe_schema_version(void)\n" +
            "{\n" +
            "    return %8$s;\n" +
            "}\n\n" +

            "SBE_ONE_DEF const char *%10$s_sbe_semantic_type(void)\n" +
            "{\n" +
            "    return \"%9$s\";\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %10$s_offset(\n" +
            "    const %10$s *const codec)\n" +
            "{\n" +
            "    return codec->offset;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %10$s *%10$s_wrap_and_apply_header(\n" +
            "    %10$s *const codec,\n" +
            "    char *buffer,\n" +
            "    const uint64_t offset,\n" +
            "    const uint64_t buffer_length,\n" +
            "    %11$s *const hdr)\n" +
            "{\n" +
            "    %11$s_wrap(\n" +
            "        hdr, buffer + offset, 0, buffer_length, %11$s_sbe_schema_version());\n\n" +

            "    %11$s_set_blockLength(hdr, %10$s_sbe_block_length());\n" +
            "    %11$s_set_templateId(hdr, %10$s_sbe_template_id());\n" +
            "    %11$s_set_schemaId(hdr, %10$s_sbe_schema_id());\n" +
            "    %11$s_set_version(hdr, %10$s_sbe_schema_version());\n\n" +

            "    %10$s_reset(\n" +
            "        codec,\n" +
            "        buffer + offset + %11$s_encoded_length(),\n" +
            "        0,\n" +
            "        buffer_length - %11$s_encoded_length(),\n" +
            "        %10$s_sbe_block_length(),\n" +
            "        %10$s_sbe_schema_version());\n\n" +
            "    return codec;\n" +
            "}\n\n" +

            "SBE_ONE_DEF %10$s *%10$s_wrap_for_encode(\n" +
            "    %10$s *const codec,\n" +
            "    char *buffer,\n" +
            "    const uint64_t offset,\n" +
            "    const uint64_t buffer_length)\n" +
            "{\n" +
            "    return %10$s_reset(\n" +
            "        codec,\n" +
            "        buffer,\n" +
            "        offset,\n" +
            "        buffer_length,\n" +
            "        %10$s_sbe_block_length(),\n" +
            "        %10$s_sbe_schema_version());\n" +
            "}\n\n" +

            "SBE_ONE_DEF %10$s *%10$s_wrap_for_decode(\n" +
            "    %10$s *const codec,\n" +
            "    char *buffer,\n" +
            "    const uint64_t offset,\n" +
            "    const uint64_t acting_block_length,\n" +
            "    const uint64_t acting_version,\n" +
            "    const uint64_t buffer_length)\n" +
            "{\n" +
            "    return %10$s_reset(\n" +
            "        codec,\n" +
            "        buffer,\n" +
            "        offset,\n" +
            "        buffer_length,\n" +
            "        acting_block_length,\n" +
            "        acting_version);\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %10$s_encoded_length(\n" +
            "    const %10$s *const codec)\n" +
            "{\n" +
            "    return %10$s_sbe_position(codec) - codec->offset;\n" +
            "}\n\n" +

            "SBE_ONE_DEF const char *%10$s_buffer(\n" +
            "    const %10$s *const codec)\n" +
            "{\n" +
            "    return codec->buffer;\n" +
            "}\n\n" +

            "SBE_ONE_DEF char *%10$s_mut_buffer(\n" +
            "    %10$s *const codec)\n" +
            "{\n" +
            "    return codec->buffer;\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %10$s_buffer_length(\n" +
            "    const %10$s *const codec)\n" +
            "{\n" +
            "    return codec->buffer_length;\n" +
            "}\n\n" +

            "SBE_ONE_DEF uint64_t %10$s_acting_version(\n" +
            "    const %10$s *const codec)\n" +
            "{\n" +
            "    return codec->acting_version;\n" +
            "}\n",
            blockLengthType,
            generateLiteral(ir.headerStructure().blockLengthType(), Integer.toString(token.encodedLength())),
            templateIdType,
            generateLiteral(ir.headerStructure().templateIdType(), Integer.toString(token.id())),
            schemaIdType,
            generateLiteral(ir.headerStructure().schemaIdType(), Integer.toString(ir.id())),
            schemaVersionType,
            generateLiteral(ir.headerStructure().schemaVersionType(), Integer.toString(ir.version())),
            semanticType,
            structName,
            messageHeaderStruct);
    }

    private CharSequence generateFieldFunctions(
        final CharSequence[] scope,
        final String containingStructName,
        final String outermostStruct,
        final List<Token> tokens)
    {
        final StringBuilder sb = new StringBuilder();

        for (int i = 0, size = tokens.size(); i < size; i++)
        {
            final Token signalToken = tokens.get(i);
            if (signalToken.signal() == Signal.BEGIN_FIELD)
            {
                final Token encodingToken = tokens.get(i + 1);
                final String propertyName = formatPropertyName(signalToken.name());

                generateFieldMetaAttributeFunction(
                    sb,
                    signalToken,
                    containingStructName,
                    outermostStruct);
                generateFieldCommonFunctions(
                    sb,
                    signalToken,
                    encodingToken,
                    propertyName,
                    containingStructName);
                generatePropertyFunctions(
                    sb,
                    scope,
                    containingStructName,
                    outermostStruct,
                    signalToken,
                    propertyName,
                    encodingToken);
            }
        }

        return sb;
    }

    private void generateFieldCommonFunctions(
        final StringBuilder sb,
        final Token fieldToken,
        final Token encodingToken,
        final String propertyName,
        final String containingStructName)
    {
        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint16_t %3$s_%1$s_id(void)\n" +
            "{\n" +
            "    return %2$d;\n" +
            "}\n",
            propertyName,
            fieldToken.id(),
            containingStructName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %3$s_%1$s_since_version(void)\n" +
            "{\n" +
            "    return %2$d;\n" +
            "}\n\n" +

            "SBE_ONE_DEF bool %3$s_%1$s_in_acting_version(\n" +
            "    const %3$s *const codec)\n" +
            "{\n" +
            "    return codec->acting_version >= %3$s_%1$s_since_version();\n" +
            "}\n",
            propertyName,
            fieldToken.version(),
            containingStructName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %2$s_%1$s_encoded_offset(void)\n" +
            "{\n" +
            "    return %3$d;\n" +
            "}\n",
            propertyName,
            containingStructName,
            encodingToken.offset()));
    }

    private static void generateFieldMetaAttributeFunction(
        final StringBuilder sb, final Token token, final String containingStructName, final String outermostStruct)
    {
        final Encoding encoding = token.encoding();
        final String epoch = encoding.epoch() == null ? "" : encoding.epoch();
        final String timeUnit = encoding.timeUnit() == null ? "" : encoding.timeUnit();
        final String semanticType = encoding.semanticType() == null ? "" : encoding.semanticType();

        sb.append(String.format("\n" +
            "SBE_ONE_DEF const char *%6$s_%s_meta_attribute(\n" +
            "    const enum sbe_meta_attribute attribute)\n" +
            "{\n" +
            "    switch (attribute)\n" +
            "    {\n" +
            "        case sbe_meta_attribute_EPOCH: return \"%s\";\n" +
            "        case sbe_meta_attribute_TIME_UNIT: return \"%s\";\n" +
            "        case sbe_meta_attribute_SEMANTIC_TYPE: return \"%s\";\n" +
            "        case sbe_meta_attribute_PRESENCE: return \"%s\";\n" +
            "    }\n\n" +

            "    return \"\";\n" +
            "}\n",
            token.name(),
            epoch,
            timeUnit,
            semanticType,
            encoding.presence().toString().toLowerCase(),
            containingStructName
        ));
    }

    private static CharSequence generateEnumFieldNotPresentCondition(final int sinceVersion)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            "    if (codec->acting_version < %1$d)\n" +
            "    {\n" +
            "        return false;\n" +
            "    }\n\n",
            sinceVersion);
    }

    private static CharSequence generateEnumFieldNotPresentCondition(final int sinceVersion, final String enumName)
    {
        if (0 == sinceVersion)
        {
            return "";
        }

        return String.format(
            "    if (codec->acting_version < %1$d)\n" +
            "    {\n" +
            "        return %2$s_NULL_VALUE;\n" +
            "    }\n\n",
            sinceVersion,
            enumName);
    }

    private void generateEnumProperty(
        final StringBuilder sb,
        final CharSequence[] scope,
        final String containingStructName,
        final Token signalToken,
        final String propertyName,
        final Token token)
    {
        final String enumName = formatScopedName(scope, token.applicableTypeName());
        final String typeName = cTypeName(token.encoding().primitiveType());
        final int offset = token.offset();

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %3$s_%1$s_encoded_length(void)\n" +
            "{\n" +
            "    return %2$d;\n" +
            "}\n",
            propertyName,
            signalToken.encodedLength(),
            containingStructName));

        if (signalToken.isConstantEncoding())
        {
            final String constValue = signalToken.encoding().constValue().toString();

            sb.append(String.format("\n" +
                "SBE_ONE_DEF enum %1$s %4$s_%2$s_const_value(void)\n" +
                "{\n" +
                "    return %1$s_%3$s;\n" +
                "}\n",
                enumName,
                propertyName,
                constValue.substring(constValue.indexOf(".") + 1),
                containingStructName));

            sb.append(String.format("\n" +
                "SBE_ONE_DEF enum %1$s %5$s_%2$s(\n" +
                "    const %5$s *const codec)\n" +
                "{\n" +
                "%3$s" +
                "    return %1$s_%4$s;\n" +
                "}\n",
                enumName,
                propertyName,
                generateEnumFieldNotPresentCondition(token.version(), enumName),
                constValue.substring(constValue.indexOf(".") + 1),
                containingStructName));
        }
        else
        {
            sb.append(String.format("\n" +
                "SBE_ONE_DEF bool %7$s_%2$s(\n" +
                "    const %7$s *const codec,\n" +
                "    enum %1$s *const out)\n" +
                "{\n" +
                "%3$s" +
                "    %5$s val;\n" +
                "    memcpy(&val, codec->buffer + codec->offset + %6$d, sizeof(%5$s));\n\n" +
                "    return %1$s_get(%4$s(val), out);\n" +
                "}\n",
                enumName,
                propertyName,
                generateEnumFieldNotPresentCondition(token.version()),
                formatByteOrderEncoding(token.encoding().byteOrder(), token.encoding().primitiveType()),
                typeName,
                offset,
                containingStructName));

            sb.append(String.format("\n" +
                "SBE_ONE_DEF %1$s *%1$s_set_%2$s(\n" +
                "    %1$s *const codec,\n" +
                "    const enum %3$s value)\n" +
                "{\n" +
                "    %4$s val = %6$s(value);\n" +
                "    memcpy(codec->buffer + codec->offset + %5$d, &val, sizeof(%4$s));\n\n" +
                "    return codec;\n" +
                "}\n",
                containingStructName,
                propertyName,
                enumName,
                typeName,
                offset,
                formatByteOrderEncoding(token.encoding().byteOrder(), token.encoding().primitiveType())));
        }
    }

    private static void generateBitsetPropertyFunctions(
        final StringBuilder sb,
        final CharSequence[] scope,
        final String propertyName,
        final Token token,
        final String containingStructName)
    {
        final String bitsetName = formatScopedName(scope, token.applicableTypeName());
        final int offset = token.offset();

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s *%4$s_%2$s(\n" +
            "    %4$s *const codec,\n" +
            "    %1$s *const bitset)\n" +
            "{\n" +
            "    return %1$s_wrap(\n" +
            "        bitset,\n" +
            "        codec->buffer,\n" +
            "        codec->offset + %3$d,\n" +
            "        codec->acting_version,\n" +
            "        codec->buffer_length);\n" +
            "}\n",
            bitsetName,
            propertyName,
            offset,
            containingStructName));

        sb.append(String.format("\n" +
            "SBE_ONE_DEF uint64_t %s_%s_encoded_length(void)\n" +
            "{\n" +
            "    return %d;\n" +
            "}\n",
            containingStructName,
            propertyName,
            token.encoding().primitiveType().size()));
    }

    private static void generateCompositePropertyFunction(
        final StringBuilder sb,
        final CharSequence[] scope,
        final String propertyName,
        final Token token,
        final String containingStructName)
    {
        final String compositeName = formatScopedName(scope, token.applicableTypeName());
        final int offset = token.offset();

        sb.append(String.format("\n" +
            "SBE_ONE_DEF %1$s *%4$s_%2$s(\n" +
            "    %4$s *const codec,\n" +
            "    %1$s *const composite)\n" +
            "{\n" +
            "    return %1$s_wrap(\n" +
            "        composite,\n" +
            "        codec->buffer,\n" +
            "        codec->offset + %3$d,\n" +
            "        codec->acting_version,\n" +
            "        codec->buffer_length);\n" +
            "}\n",
            compositeName,
            propertyName,
            offset,
            containingStructName));
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

    private static CharSequence generateLiteral(final Encoding encoding)
    {
        return generateLiteral(encoding.primitiveType(), encoding.constValue().toString());
    }

    private static CharSequence generateLiteral(final PrimitiveType type, final String value)
    {
        String literal = "";

        final String castType = cTypeName(type);
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
            {
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
            }

            case FLOAT:
                literal = value.endsWith("NaN") ? "SBE_FLOAT_NAN" : value + "f";
                break;

            case INT64:
            {
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
            }

            case UINT64:
                literal = "UINT64_C(0x" + Long.toHexString(Long.parseLong(value)) + ")";
                break;

            case DOUBLE:
                literal = value.endsWith("NaN") ? "SBE_DOUBLE_NAN" : value;
                break;
        }

        return literal;
    }
}
