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
package uk.co.real_logic.sbe.xml;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.PrimitiveValue;

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.Map;

import static uk.co.real_logic.sbe.xml.XmlSchemaParser.*;

/**
 * SBE set tpe representing a bitset of options.
 */
public class SetType extends Type
{
    /**
     * SBE schema set type.
     */
    public static final String SET_TYPE = "set";

    private final int arrayCapacity;
    private final PrimitiveType encodingType;
    private final Map<PrimitiveValue, Subfield> subfieldByPrimitiveValueMap = new LinkedHashMap<>();
    private final Map<String, Subfield> subfieldByNameMap = new LinkedHashMap<>();

    public SetType(final Node node)
        throws XPathExpressionException, IllegalArgumentException
    {
        this(node, null, null, -1);
    }

    /**
     * Construct a new SetType from XML Schema.
     *
     * @param node           from the XML Schema Parsing
     * @param givenName      for the node.
     * @param referencedName of the type when created from a ref in a composite.
     * @param arrayCapacity  for this EnumType.
     * @throws XPathExpressionException on invalid XPath.
     * @throws IllegalArgumentException on illegal encoding type.
     */
    public SetType(final Node node, final String givenName, final String referencedName, final int arrayCapacity)
        throws XPathExpressionException, IllegalArgumentException
    {
        super(node, givenName, referencedName);
        this.arrayCapacity = arrayCapacity;

        final String encodingTypeStr = getAttributeValueOrNull(node, "encodingType");
        encodingType = getEncodingType(node, encodingTypeStr).encodingType;
        if (encodingType == null)
        {
            handleError(node, "Illegal encodingType " + encodingTypeStr);
        }
        else if (!PrimitiveType.isUnsigned(encodingType))
        {
            handleError(node, "encodingType should be unsigned PrimitiveType for bitset, but we got " +
                encodingTypeStr);
        }

        final XPath xPath = XPathFactory.newInstance().newXPath();
        final NodeList bitsList = (NodeList)xPath.compile("bits").evaluate(node, XPathConstants.NODESET);

        for (int i = 0, size = bitsList.getLength(); i < size; i++)
        {
            final Bits c = new Bits(bitsList.item(i), encodingType);
            final PrimitiveValue[] values = c.primitiveValues();
            for (int j = 0; j < values.length; j += 1)
            {
                final PrimitiveValue value = values[j];
                if (subfieldByPrimitiveValueMap.get(value) != null)
                {
                    handleError(node, "Bits value already defined: " + value);
                }
                subfieldByPrimitiveValueMap.put(value, c);
            }

            if (subfieldByNameMap.get(c.name()) != null)
            {
                handleError(node, "Bits already exists for name: " + c.name());
            }

            subfieldByNameMap.put(c.name(), c);
        }
        final NodeList choiceList = (NodeList)xPath.compile("choice").evaluate(node, XPathConstants.NODESET);

        for (int i = 0, size = choiceList.getLength(); i < size; i++)
        {
            final Choice c = new Choice(choiceList.item(i), encodingType);

            if (subfieldByPrimitiveValueMap.get(c.primitiveValue()) != null)
            {
                handleWarning(node, "Choice value already defined: " + c.primitiveValue());
            }

            if (subfieldByNameMap.get(c.name()) != null)
            {
                handleWarning(node, "Choice already exists for name: " + c.name());
            }

            subfieldByPrimitiveValueMap.put(c.primitiveValue(), c);
            subfieldByNameMap.put(c.name(), c);
        }
    }

    /**
     * The encoding type of the bitset to be used on the wire.
     *
     * @return encoding type of the bitset to be used on the wire.
     */
    public PrimitiveType encodingType()
    {
        return encodingType;
    }

    /**
     * The arrayCapacity of the SetType when this SetType
     * referenced as an array element in compositeType
     * @return arrayCapacity of the SetType
     */
    public int arrayCapacity()
    {
        return arrayCapacity;
    }

    /**
     * The encodedLength (in octets) of the encodingType
     *
     * @return encodedLength of the encodingType
     */
    public int encodedLength()
    {
        if (this.arrayCapacity < 0)
        {
            return encodingType.size();
        }
        else
        {
            return encodingType.size() * this.arrayCapacity;
        }
    }

    /**
     * Get the {@link Choice} represented by a {@link PrimitiveValue}.
     *
     * @param value to get
     * @return the {@link Choice} represented by a {@link PrimitiveValue} or null if not found.
     */
    public Subfield getSubfield(final PrimitiveValue value)
    {
        return subfieldByPrimitiveValueMap.get(value);
    }

    /**
     * Get the {@link Choice} represented by a String name.
     *
     * @param name to get
     * @return the {@link Choice} represented by a String name or null if not found.
     */
    public Subfield getSubfield(final String name)
    {
        return subfieldByNameMap.get(name);
    }

    /**
     * The collection of possible {@link Choice} values for a bitset.
     *
     * @return the collection of possible {@link Choice} values for a bitset.
     */
    public Collection<Subfield> subfields()
    {
        return subfieldByNameMap.values();
    }

    /**
     * Get the {@link Choice} represented by a String name.
     *
     * @param name to get
     * @return the {@link Choice} represented by a String name or null if not found.
     */
    public Choice getChoice(final String name)
    {
        return (Choice)subfieldByNameMap.get(name);
    }

    /**
     * Get the {@link Bits} represented by a String name.
     *
     * @param name to get
     * @return the {@link Bits} represented by a String name or null if not found.
     */
    public Bits getBits(final String name)
    {
        return (Bits)subfieldByNameMap.get(name);
    }

    /**
     * Always false.
     *
     * {@inheritDoc}
     */
    public boolean isVariableLength()
    {
        return false;
    }

    public String toString()
    {
        return "SetType{" +
            "encodingType=" + encodingType +
            ", subfieldByPrimitiveValueMap=" + subfieldByPrimitiveValueMap +
            ", subfieldByNameMap=" + subfieldByNameMap +
            '}';
    }

    public static class Subfield
    {
        protected final String name;
        public Subfield(final String name)
        {
            this.name = name;
        }

        /**
         * The String name representation of the bitset choice.
         *
         * @return the String name representation of the bitset choice.
         */
        public String name()
        {
            return name;
        }
    }

    /**
     * Holder for valid values for SBE schema choice type.
     */
    public static class Choice extends Subfield
    {
        private final String description;
        private final PrimitiveValue value;
        private final int sinceVersion;
        private final int deprecated;

        /**
         * Construct a Choice given the XML node and the encodingType
         *
         * @param node         that contains the validValue
         * @param bitsetEncodingType for the bitset
         */
        public Choice(final Node node, final PrimitiveType bitsetEncodingType)
        {
            super(getAttributeValue(node, "name"));
            description = getAttributeValueOrNull(node, "description");
            value = PrimitiveValue.parse(node.getFirstChild().getNodeValue(), PrimitiveType.UINT8);
            sinceVersion = Integer.parseInt(getAttributeValue(node, "sinceVersion", "0"));
            deprecated = Integer.parseInt(getAttributeValue(node, "deprecated", "0"));

            // choice values are bit positions (0, 1, 2, 3, 4, etc.) from LSB to MSB
            if (value.longValue() >= (bitsetEncodingType.size() * 8))
            {
                throw new IllegalArgumentException("Choice value out of bounds: " + value.longValue());
            }

            checkForValidName(node, name);
        }

        /**
         * The {@link PrimitiveValue} representation of the bitset choice.
         *
         * @return the {@link PrimitiveValue} representation of the bitset choice.
         */
        public PrimitiveValue primitiveValue()
        {
            return value;
        }


        /**
         * The description of the bitset choice.
         *
         * @return the description of the bitset choice.
         */
        public String description()
        {
            return description;
        }

        /**
         * The sinceVersion value of the {@link Choice}
         *
         * @return the sinceVersion value of the {@link Choice}
         */
        public int sinceVersion()
        {
            return sinceVersion;
        }

        /**
         * Version in which {@link Choice} was deprecated. Only valid if greater than zero.
         *
         * @return version in which the {@link Choice} was deprecated.
         */
        public int deprecated()
        {
            return deprecated;
        }

        public String toString()
        {
            return "Choice{" +
                "name='" + name + '\'' +
                ", description='" + description + '\'' +
                ", value=" + value +
                ", sinceVersion=" + sinceVersion +
                ", deprecated=" + deprecated +
                '}';
        }
    }

    public static class EncodingTypeInfo
    {
        public final PrimitiveType encodingType;
        public final String referencedName;

        public EncodingTypeInfo(final PrimitiveType encodingType, final String referencedName)
        {
            this.encodingType = encodingType;
            this.referencedName = referencedName;
        }
    }

    public static EncodingTypeInfo getEncodingType(final Node node, final String encodingTypeStr)
        throws XPathExpressionException
    {
        if (node == null || encodingTypeStr == null)
        {
            return new EncodingTypeInfo(null, null);
        }
        final PrimitiveType foundType = PrimitiveType.getOrNull(encodingTypeStr);
        if (foundType != null)
        {
            return new EncodingTypeInfo(foundType, null);
        }
        final XPath xPath = XPathFactory.newInstance().newXPath();
        final String expression = TYPE_XPATH_EXPR + "[@name='" + encodingTypeStr + "']";
        final Node encodingTypeNode = (Node)xPath
            .compile(expression)
            .evaluate(node.getOwnerDocument(), XPathConstants.NODE);
        if (null == encodingTypeNode)
        {
            return new EncodingTypeInfo(null, null);
        }
        if (Integer.parseInt(getAttributeValue(encodingTypeNode, "length", "1")) != 1)
        {
            return new EncodingTypeInfo(null, encodingTypeStr);
        }
        final PrimitiveType foundIndirectType = PrimitiveType.getOrNull(
            getAttributeValue(encodingTypeNode, "primitiveType", ""));
        return new EncodingTypeInfo(foundIndirectType, encodingTypeStr);
    }

    /**
     * Holder for valid values for SBE schema bits type.
     */
    public static class Bits extends Subfield
    {
        private final String description;
        private final PrimitiveValue[] values;
        private final int sinceVersion;
        private final int deprecated;
        private final PrimitiveValue msb;
        private final PrimitiveValue lsb;
        private final EncodingTypeInfo encodingTypeInfo;

        /**
         * Construct a Bits given the XML node and the encodingType
         *
         * @param node         that contains the validValue
         * @param bitsetEncodingType for the bitset
         * @throws XPathExpressionException on invalid XPath.
         */
        public Bits(final Node node, final PrimitiveType bitsetEncodingType)
            throws XPathExpressionException
        {
            super(getAttributeValue(node, "name"));
            description = getAttributeValueOrNull(node, "description");
            sinceVersion = Integer.parseInt(getAttributeValue(node, "sinceVersion", "0"));
            deprecated = Integer.parseInt(getAttributeValue(node, "deprecated", "0"));

            final String encodingTypeStr = getAttributeValueOrNull(node, "type");

            if (encodingTypeStr == null)
            {
                encodingTypeInfo = new EncodingTypeInfo(bitsetEncodingType, null);
            }
            else
            {
                final EncodingTypeInfo encodingTypeInfoDirect = getEncodingType(node, encodingTypeStr);
                if (encodingTypeInfoDirect.encodingType != null || encodingTypeInfoDirect.referencedName != null)
                {
                    encodingTypeInfo = encodingTypeInfoDirect;
                }
                else
                {
                    final XPath xPath = XPathFactory.newInstance().newXPath();
                    final Node enumNode = (Node)xPath.compile(ENUM_XPATH_EXPR + "[@name='" + encodingTypeStr + "']")
                        .evaluate(node.getOwnerDocument(), XPathConstants.NODE);
                    final String encodingTypeStrEnum = getAttributeValueOrNull(enumNode, "encodingType");
                    final PrimitiveType enumEncodingType = getEncodingType(enumNode, encodingTypeStrEnum).encodingType;
                    encodingTypeInfo = new EncodingTypeInfo(enumEncodingType, encodingTypeStr);
                }
            }

            if (encodingTypeInfo.encodingType == null)
            {
                handleError(node, "Illegal encodingType " + encodingTypeStr);
            }
            else if (encodingTypeInfo.encodingType.size() > bitsetEncodingType.size())
            {
                handleError(node, String.format("Bits encodingType %s out of bitset encodingType: %s",
                    encodingTypeStr, bitsetEncodingType));
            }

            msb = PrimitiveValue.parse(getAttributeValue(node, "msb"), PrimitiveType.UINT8);
            lsb = PrimitiveValue.parse(getAttributeValue(node, "lsb"), PrimitiveType.UINT8);

            // bitset composite by a fixed size `8|16|32|64` (| stands for or) number of bits numbering as following,
            //   0[LSB], 1, 2, 3, 4, ..., `8|16|32|64-1` [MSB] bits)
            // bits are set of consecutive bit positions in bitset
            // the bit count of bits are in the range 1 to `8|16|32|64` inclusive.
            // the bits have two non negtive integer attribute lsb and msb all in the
            // range 0 to `8|16|32|64 - 1` inclusive
            // if lsb <= msb, the bits are composed by bits lsb, lsb + 1, lsb + 2, lsb + 3, ... msb in bitset
            // if lsb > msb, the bits are composed by bits lsb, lsb - 1, lsb - 2, lsb - 3, ... msb in bitset

            if (msb.longValue() >= (bitsetEncodingType.size() * 8))
            {
                handleError(node, "Bits msb out of bounds: " + msb.longValue());
            }

            if (lsb.longValue() >= (bitsetEncodingType.size() * 8))
            {
                handleError(node, "Bits lsb out of bounds: " + lsb.longValue());
            }

            final long diff = msb.longValue() - lsb.longValue();
            final long sign = Long.signum(diff);
            values = new PrimitiveValue[(int)(Math.abs(diff) + 1)];
            for (int i = 0; i < values.length; i += 1)
            {
                values[i] = new PrimitiveValue(lsb.longValue() + sign * i, 1);
            }

            checkForValidName(node, name);
        }

        /**
         * The {@link PrimitiveValue} representation of the bitset bits.
         *
         * @return the {@link PrimitiveValue} representation of the bitset bits.
         */
        public EncodingTypeInfo encodingTypeInfo()
        {
            return encodingTypeInfo;
        }

        /**
         * The {@link PrimitiveValue} representation of the bitset bits.
         *
         * @return the {@link PrimitiveValue} representation of the bitset bits.
         */
        public PrimitiveValue[] primitiveValues()
        {
            return values;
        }

        /**
         * The {@link PrimitiveValue} representation of the lsb of bitset bits.
         *
         * @return the {@link PrimitiveValue} representation of the lsb of bitset bits.
         */
        public PrimitiveValue lsbValue()
        {
            return lsb;
        }

        /**
         * The {@link PrimitiveValue} representation of the lsb of bitset bits.
         *
         * @return the {@link PrimitiveValue} representation of the lsb of bitset bits.
         */
        public PrimitiveValue msbValue()
        {
            return msb;
        }

        /**
         * The description of the bitset bits.
         *
         * @return the description of the bitset bits.
         */
        public String description()
        {
            return description;
        }

        /**
         * The sinceVersion value of the {@link Bits}
         *
         * @return the sinceVersion value of the {@link Bits}
         */
        public int sinceVersion()
        {
            return sinceVersion;
        }

        /**
         * Version in which {@link Bits} was deprecated. Only valid if greater than zero.
         *
         * @return version in which the {@link Bits} was deprecated.
         */
        public int deprecated()
        {
            return deprecated;
        }

        public String toString()
        {
            return "Bits{" +
                "name='" + name + '\'' +
                ", description='" + description + '\'' +
                ", values=" + values +
                ", sinceVersion=" + sinceVersion +
                ", deprecated=" + deprecated +
                '}';
        }
    }
}
