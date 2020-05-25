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

import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.util.ValidationUtil;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.ByteOrder;
import java.util.EnumMap;
import java.util.Map;
import java.util.StringJoiner;

import static uk.co.real_logic.sbe.generation.Generators.toLowerFirstChar;
import static uk.co.real_logic.sbe.ir.GenerationUtil.*;

/**
 * Utilities for mapping between IR and the C language.
 */
public class CUtil
{
    private static final Map<PrimitiveType, String> PRIMITIVE_TYPE_STRING_ENUM_MAP = new EnumMap<>(PrimitiveType.class);

    static
    {
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.CHAR, "char");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT8, "int8_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT16, "int16_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT32, "int32_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT64, "int64_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT8, "uint8_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT16, "uint16_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT32, "uint32_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT64, "uint64_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.FLOAT, "float");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.DOUBLE, "double");
    }

    @FunctionalInterface
    public interface GenerateLiteralFunction
    {
        CharSequence apply(PrimitiveType a, String b);
    }

    /**
     * Map the name of a {@link uk.co.real_logic.sbe.PrimitiveType} to a C11 primitive type name.
     *
     * @param primitiveType to map.
     * @return the name of the Java primitive that most closely maps.
     */
    public static String cTypeName(final PrimitiveType primitiveType)
    {
        return PRIMITIVE_TYPE_STRING_ENUM_MAP.get(primitiveType);
    }

    /**
     * Format a String as a property name.
     *
     * @param value to be formatted.
     * @return the string formatted as a property name.
     */
    public static String formatPropertyName(final String value)
    {
        final String formattedValue = toLowerFirstChar(value);
        return ValidationUtil.tryFixInvalidName(formattedValue);
    }

    /**
     * Format a String as a struct name.
     *
     * @param value to be formatted.
     * @return the string formatted as a struct name.
     */
    public static String formatName(final String value)
    {
        return toLowerFirstChar(value);
    }

    /**
     * Format a String as a struct name prepended with a scope.
     *
     * @param scope to be prepended.
     * @param value to be formatted.
     * @return the string formatted as a struct name.
     */
    public static String formatScopedName(final CharSequence[] scope, final String value)
    {
        return formatScope(scope) + "_" + formatName(value);
    }

    /**
     * Return the C99 formatted byte order encoding string to use for a given byte order and primitiveType
     *
     * @param byteOrder     of the {@link uk.co.real_logic.sbe.ir.Token}
     * @param primitiveType of the {@link uk.co.real_logic.sbe.ir.Token}
     * @return the string formatted as the byte ordering encoding
     */
    public static String formatByteOrderEncoding(final ByteOrder byteOrder, final PrimitiveType primitiveType)
    {
        switch (primitiveType.size())
        {
            case 2:
                return "SBE_" + byteOrder + "_ENCODE_16";

            case 4:
                return "SBE_" + byteOrder + "_ENCODE_32";

            case 8:
                return "SBE_" + byteOrder + "_ENCODE_64";

            default:
                return "";
        }
    }

    public static String generateGetBits(
        final long lsb,
        final long msb,
        final PrimitiveType bitsetPrimitiveType,
        final String bitsetName,
        final String byteOrderStr,
        final GenerateLiteralFunction generateLiteral)
    {
        final String bitsetWithByteOrder = String.format("%1$s(%2$s)", byteOrderStr, bitsetName);
        final long len = Math.abs(msb - lsb) + 1;
        final CharSequence maskLiteral = generateLiteral.apply(bitsetPrimitiveType, Long.toString((1 << len) - 1));
        if (msb >= lsb)
        {
            return String.format("%1$s & (%2$s >> %3$d)", maskLiteral, bitsetWithByteOrder, lsb);
        }
        else
        {
            final long reversedLsb = bitsetPrimitiveType.size() * 8 - 1 - lsb;
            return String.format("%1$s & (sbe_reverse_bits_%2$s(%3$s) >> %4$d)",
                maskLiteral,
                cTypeName(bitsetPrimitiveType),
                bitsetWithByteOrder,
                reversedLsb);
        }
    }

    public static String generateSetBits(
        final long lsb,
        final long msb,
        final PrimitiveType bitsetPrimitiveType,
        final String bitsName,
        final String bitsetName,
        final String byteOrderStr,
        final GenerateLiteralFunction generateLiteral)
    {
        final String bitsetWithByteOrder = String.format("%1$s(%2$s)", byteOrderStr, bitsetName);
        final long len = Math.abs(msb - lsb) + 1;
        final long fullMask = (1L << bitsetPrimitiveType.size() * 8) - 1;
        long mask = (1 << len) - 1;
        final String setBitsExpression;
        if (msb >= lsb)
        {
            mask = fullMask - (mask << lsb); /* bits keep unchanged */
            final CharSequence maskLiteral = generateLiteral.apply(bitsetPrimitiveType, Long.toString(mask));

            setBitsExpression = String.format("(%1$s & %2$s) | ((%3$s)%4$s << %5$d)",
                maskLiteral,
                bitsetWithByteOrder,
                cTypeName(bitsetPrimitiveType),
                bitsName,
                lsb);
        }
        else
        {
            mask = fullMask - (mask << msb); /* bits keep unchanged */
            final CharSequence maskLiteral = generateLiteral.apply(bitsetPrimitiveType, Long.toString(mask));
            final long reversedLsb = bitsetPrimitiveType.size() * 8 - 1 - lsb;

            setBitsExpression = String.format("(%1$s & %2$s) | sbe_reverse_bits_%3$s((%3$s)%4$s << %5$d)",
                maskLiteral,
                bitsetWithByteOrder,
                cTypeName(bitsetPrimitiveType),
                bitsName,
                reversedLsb);
        }
        if (byteOrderStr.compareTo("") == 0)
        {
            return setBitsExpression;
        }
        else
        {
            return String.format("%1$s(%2$s)", byteOrderStr, setBitsExpression);
        }
    }

    public static String generateViewName(final PrimitiveType primitiveType, final boolean isBigEndian)
    {
        final CharSequence byteOrderSuffix = isBigEndian ? "_be" : "_le";
        final String viewTypeName = String.format("sbe_%s_view%s",
            primitiveType.primitiveName().equals("char") ? "string" : primitiveType.primitiveName(),
            primitiveType.size() == 1 ? "" : byteOrderSuffix
        );
        return viewTypeName;
    }

    public static String generateAllViewFunctions()
    {
        final StringBuilder sb = new StringBuilder();

        for (final PrimitiveType val : PrimitiveType.values())
        {
            if (!val.primitiveName().equals("char"))
            {
                final int count = val.size() == 1 ? 1 : 2;
                for (int i = 0; i < count; i += 1)
                {
                    sb.append(generateSingleViewFunctions(val, i == 0));
                }
            }
        }
        return sb.toString();
    }

    public static String generateNullLiteral(final PrimitiveType primitiveType)
    {
        switch (primitiveType)
        {
            case CHAR:
                return "";
            case FLOAT:
                return "SBE_FLOAT_NAN";
            case DOUBLE:
                return "SBE_DOUBLE_NAN";
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
            default:
                return "";
        }
    }

    public static String identBlockWith(
        final String input, final String delimiter, final String prefix, final String suffix)
    {
        final String[] lines = input.split("\\r?\\n");
        final StringJoiner sj = new StringJoiner(delimiter, prefix, suffix);
        for (int i = 0; i < lines.length; i += 1)
        {
            if (lines[i].length() > 0)
            {
                sj.add("    " + lines[i]);
            }
            else
            {
                sj.add("");
            }
        }
        final String identString = sj.toString();
        return identString;
    }

    public static String identBlock(final String input)
    {
        return identBlockWith(input, "\n", "    {\n", "\n    }\n");
    }

    @SuppressWarnings("checkstyle:methodlength")
    private static String generateSingleViewFunctions(final PrimitiveType type, final boolean isBigEndian)
    {
        final String macroGetSetPrefix = type.size() > 1 ? "_SBE_BYTE_ORDER_BUFFER" : "_SBE_BUFFER_VIEW";
        final String byteOrderFunction = formatByteOrderEncoding(
            isBigEndian ? ByteOrder.BIG_ENDIAN : ByteOrder.LITTLE_ENDIAN,
            type
        );

        final String dateTypeForEncode = type.size() == 2 ? "uint16_t" :
            type.size() == 4 ? "uint32_t" :
            type.size() == 8 ? "uint64_t" : "";
        final String byteOrderParameter = type.size() == 1 ? "" :
            String.format(" ,%1$s, %2$s", byteOrderFunction, dateTypeForEncode);

        return String.format("\n" +
            "struct %1$s\n" +
            "{\n" +
            "    void* data;\n" +
            "    size_t length;\n" +
            "#if defined(__cplusplus)\n" +
            identBlockWith(
            "bool set(const %5$s *ptr, size_t len)\n" +
            "{\n" +
            "    %2$s_SAFE_SET_RANGE((*this), ptr, 0, len%3$s);\n" +
            "}\n" +

            "void set_unsafe(const %5$s *ptr)\n" +
            "{\n" +
            "    %2$s_UNSAFE_SET_RANGE((*this), ptr, 0, length%3$s);\n" +
            "}\n" +

            "bool set(size_t index, %5$s val)\n" +
            "{\n" +
            "    %2$s_SAFE_SET_AT((*this), index, val%3$s);\n" +
            "}\n" +

            "void set_unsafe(size_t index, %5$s val)\n" +
            "{\n" +
            "    %2$s_UNSAFE_SET_AT((*this), index, val%3$s);\n" +
            "}\n" +

            "bool set(const %5$s *ptr, size_t offset, size_t len)\n" +
            "{\n" +
            "    %2$s_SAFE_SET_RANGE((*this), ptr, offset, len%3$s);\n" +
            "}\n" +

            "void set_unsafe(const %5$s *ptr, size_t offset, size_t len)\n" +
            "{\n" +
            "    %2$s_UNSAFE_SET_RANGE((*this), ptr, offset, len%3$s);\n" +
            "}\n" +

            "size_t get(%5$s *ptr, size_t len)\n" +
            "{\n" +
            "    %2$s_SAFE_GET_RANGE((*this), ptr, 0, len%3$s);\n" +
            "}\n" +

            "size_t get_unsafe(%5$s *ptr, size_t len)\n" +
            "{\n" +
            "    %2$s_UNSAFE_GET_RANGE((*this), ptr, 0, len%3$s);\n" +
            "}\n" +

            "%5$s get(size_t index)\n" +
            "{\n" +
            "    %2$s_SAFE_GET_AT((*this), index, %4$s, %5$s%3$s);\n" +
            "}\n" +

            "%5$s get_unsafe(size_t index)\n" +
            "{\n" +
            "    %2$s_UNSAFE_GET_AT((*this), index, %5$s%3$s);\n" +
            "}\n" +

            "size_t get(%5$s *ptr, size_t offset, size_t len)\n" +
            "{\n" +
            "    %2$s_SAFE_GET_RANGE((*this), ptr, offset, len%3$s);\n" +
            "}\n" +

            "size_t get_unsafe(%5$s *ptr, size_t offset, size_t len)\n" +
            "{\n" +
            "    %2$s_UNSAFE_GET_RANGE((*this), ptr, offset, len%3$s);\n" +
            "}\n",
            "\n", "", "\n") + /* end identBlockWith */
            "#endif\n" +

            "};\n" +
            "typedef struct %1$s %1$s;\n\n" +

            "SBE_ONE_DEF bool %1$s_set(%1$s view, const %5$s *data, size_t len)\n" +
            "{\n" +
            "    %2$s_SAFE_SET_RANGE(view, data, 0, len%3$s);\n" +
            "}\n" +

            "SBE_ONE_DEF void %1$s_set_buffer(%1$s view, const %5$s *data)\n" +
            "{\n" +
            "    %2$s_UNSAFE_SET_RANGE(view, data, 0, view.length%3$s);\n" +
            "}\n" +

            "SBE_ONE_DEF bool %1$s_set_at(%1$s view, size_t index, %5$s val)\n" +
            "{\n" +
            "    %2$s_SAFE_SET_AT(view, index, val%3$s);\n" +
            "}\n" +

            "SBE_ONE_DEF void %1$s_set_at_unsafe(%1$s view, size_t index, %5$s val)\n" +
            "{\n" +
            "    %2$s_UNSAFE_SET_AT(view, index, val%3$s);\n" +
            "}\n" +

            "SBE_ONE_DEF bool %1$s_set_range(%1$s view, const %5$s *data, size_t offset, size_t len)\n" +
            "{\n" +
            "    %2$s_SAFE_SET_RANGE(view, data, offset, len%3$s);\n" +
            "}\n" +

            "SBE_ONE_DEF void %1$s_set_range_unsafe(%1$s view, const %5$s *data, size_t offset, size_t len)\n" +
            "{\n" +
            "    %2$s_UNSAFE_SET_RANGE(view, data, offset, len%3$s);\n" +
            "}\n" +

            "SBE_ONE_DEF size_t %1$s_get(%1$s view, %5$s *data, size_t len)\n" +
            "{\n" +
            "    %2$s_SAFE_GET_RANGE(view, data, 0, len%3$s);\n" +
            "}\n" +

            "SBE_ONE_DEF size_t %1$s_get_unsafe(%1$s view, %5$s *data, size_t len)\n" +
            "{\n" +
            "    %2$s_UNSAFE_GET_RANGE(view, data, 0, len%3$s);\n" +
            "}\n" +

            "SBE_ONE_DEF %5$s %1$s_get_at(%1$s view, size_t index)\n" +
            "{\n" +
            "    %2$s_SAFE_GET_AT(view, index, %4$s, %5$s%3$s);\n" +
            "}\n" +

            "SBE_ONE_DEF %5$s %1$s_get_at_unsafe(%1$s view, size_t index)\n" +
            "{\n" +
            "    %2$s_UNSAFE_GET_AT(view, index, %5$s%3$s);\n" +
            "}\n" +

            "SBE_ONE_DEF size_t %1$s_get_range(%1$s view, %5$s *data, size_t offset, size_t len)\n" +
            "{\n" +
            "    %2$s_SAFE_GET_RANGE(view, data, offset, len%3$s);\n" +
            "}\n" +

            "SBE_ONE_DEF size_t %1$s_get_range_unsafe(%1$s view, %5$s *data, size_t offset, size_t len)\n" +
            "{\n" +
            "    %2$s_UNSAFE_GET_RANGE(view, data, offset, len%3$s);\n" +
            "}\n" +
            "",
            generateViewName(type, isBigEndian),
            macroGetSetPrefix,
            byteOrderParameter,
            generateNullLiteral(type),
            cTypeName(type)
        );
    }

    public static CharSequence generateSbecHeader()
    {
        try
        {
            final StringBuilder out = new StringBuilder();
            final InputStream is = CUtil.class.getResourceAsStream("/c/templates/sbec.h");
            final BufferedReader reader = new BufferedReader(new InputStreamReader(is));
            String line;
            while ((line = reader.readLine()) != null)
            {
                out.append(line);
                out.append('\n');
                if (line.equals("/*@EXPANDING-BY-GENERATOR@*/"))
                {
                    out.append(generateAllViewFunctions());
                    out.append("\n#endif /* _SBE_POLYFILL_HEADER_H */\n\n");
                    break;
                }
            }
            reader.close();
            is.close();
            return out;
        }
        catch (final IOException ex)
        {
            throw new RuntimeException(ex);
        }
    }
}
