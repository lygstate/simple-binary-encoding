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
import uk.co.real_logic.sbe.ir.Token;

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import static javax.xml.xpath.XPathConstants.NODESET;
import static uk.co.real_logic.sbe.PrimitiveType.*;
import static uk.co.real_logic.sbe.SbeTool.JAVA_GENERATE_INTERFACES;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.getAttributeValue;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.getAttributeValueOrNull;

/**
 * SBE compositeType which is a composite of other composites, sets, enums, or simple types.
 */
public class CompositeType extends Type
{
    /**
     * SBE schema composite type.
     */
    public static final String COMPOSITE_TYPE = "composite";
    public static final String SUB_TYPES_EXP = "type|enum|set|composite|array|ref";

    private final int arrayCapacity;
    private final List<String> compositesPath = new ArrayList<>();
    private final Map<String, Type> containedTypeByNameMap = new LinkedHashMap<>();

    public CompositeType(final Node node) throws XPathExpressionException
    {
        this(node, null, null, new ArrayList<>(), -1);
    }

    /**
     * Construct a new compositeType from XML Schema.
     *
     * @param node           from the XML Schema Parsing
     * @param givenName      for this node.
     * @param referencedName of the type when created from a ref in a composite.
     * @param compositesPath with the path of composites that represents the levels of composition.
     * @param arrayCapacity  for this CompositeType.
     * @throws XPathExpressionException if the XPath is invalid.
     */
    public CompositeType(
        final Node node,
        final String givenName,
        final String referencedName,
        final List<String> compositesPath,
        final int arrayCapacity)
        throws XPathExpressionException
    {
        super(node, givenName, referencedName);

        this.compositesPath.addAll(compositesPath);
        this.compositesPath.add(getAttributeValue(node, "name"));
        this.arrayCapacity = arrayCapacity;

        final XPath xPath = XPathFactory.newInstance().newXPath();
        final NodeList list = (NodeList)xPath.compile(SUB_TYPES_EXP).evaluate(node, NODESET);

        for (int i = 0, size = list.getLength(); i < size; i++)
        {
            final Node subTypeNode = list.item(i);
            final String subTypeName = XmlSchemaParser.getAttributeValue(subTypeNode, "name");

            processType(subTypeNode, subTypeName, null, null, -1);
        }

        checkForValidOffsets(node);
    }

    /**
     * Return the EncodedDataType within this composite with the given name
     *
     * @param name of the EncodedDataType to return
     * @return type requested
     */
    public Type getType(final String name)
    {
        return containedTypeByNameMap.get(name);
    }

    /**
     * The arrayCapacity of the compositeType when this compositeType
     * referenced as an array element in compositeType
     * @return arrayCapacity of the compositeType
     */
    public int arrayCapacity()
    {
        return arrayCapacity;
    }

    /**
     * The encodedLength (in octets) of the list of EncodedDataTypes
     *
     * @return encodedLength of the compositeType
     */
    public int encodedLength()
    {
        int length = 0;

        for (final Type t : containedTypeByNameMap.values())
        {
            if (t.isVariableLength())
            {
                return Token.VARIABLE_LENGTH;
            }

            if (t.offsetAttribute() != -1)
            {
                length = t.offsetAttribute();
            }

            if (t.presence() != Presence.CONSTANT)
            {
                length += t.encodedLength();
            }
        }

        if (this.arrayCapacity < 0)
        {
            return length;
        }
        else
        {
            return length * this.arrayCapacity;
        }
    }

    /**
     * Return list of the {@link Type}s that compose this composite
     *
     * @return {@link List} that holds the {@link Type}s in this composite
     */
    public List<Type> getTypeList()
    {

        return new ArrayList<>(containedTypeByNameMap.values());
    }

    /**
     * Make this composite type, if it has a varData member, variable length
     * by making the EncodedDataType with the name "varData" be variable length.
     */
    public void makeDataFieldCompositeType()
    {
        final EncodedDataType edt = (EncodedDataType)containedTypeByNameMap.get("varData");
        if (edt != null)
        {
            edt.variableLength(true);
        }
    }

    /**
     * Check the composite for being a well formed group encodedLength encoding. This means
     * that there are the fields "blockLength" and "numInGroup" present.
     *
     * @param node of the XML for this composite
     */
    public void checkForWellFormedGroupSizeEncoding(final Node node)
    {
        final EncodedDataType blockLengthType = (EncodedDataType)containedTypeByNameMap.get("blockLength");
        final EncodedDataType numInGroupType = (EncodedDataType)containedTypeByNameMap.get("numInGroup");

        if (blockLengthType == null)
        {
            XmlSchemaParser.handleError(node, "composite for group encodedLength encoding must have \"blockLength\"");
        }
        else if (!isUnsigned(blockLengthType.primitiveType()))
        {
            XmlSchemaParser.handleError(node, "\"blockLength\" must be unsigned type");
        }
        else
        {
            if (blockLengthType.primitiveType() != UINT8 &&
                blockLengthType.primitiveType() != UINT16 &&
                blockLengthType.primitiveType() != UINT32)
            {
                XmlSchemaParser.handleWarning(node, "\"blockLength\" should be UINT8, UINT16 or UINT32");
            }

            final PrimitiveValue blockLengthTypeMaxValue = blockLengthType.maxValue();
            validateGroupMaxValue(node, blockLengthType.primitiveType(), blockLengthTypeMaxValue);
        }

        if (numInGroupType == null)
        {
            XmlSchemaParser.handleError(node, "composite for group encodedLength encoding must have \"numInGroup\"");
        }
        else if (!isUnsigned(numInGroupType.primitiveType()))
        {
            XmlSchemaParser.handleWarning(node, "\"numInGroup\" should be unsigned type");
            final PrimitiveValue numInGroupMinValue = numInGroupType.minValue();
            if (null == numInGroupMinValue)
            {
                XmlSchemaParser.handleError(node, "\"numInGroup\" minValue must be set for signed types");
            }
            else if (numInGroupMinValue.longValue() < 0)
            {
                XmlSchemaParser.handleError(node, String.format(
                    "\"numInGroup\" minValue=%s must be greater than zero " +
                    "for signed \"numInGroup\" types", numInGroupMinValue));
            }
        }
        else
        {
            if (numInGroupType.primitiveType() != UINT8 &&
                numInGroupType.primitiveType() != UINT16 &&
                numInGroupType.primitiveType() != UINT32)
            {
                XmlSchemaParser.handleWarning(node, "\"numInGroup\" should be UINT8, UINT16 or UINT32");
            }

            final PrimitiveValue numInGroupMaxValue = numInGroupType.maxValue();
            validateGroupMaxValue(node, numInGroupType.primitiveType(), numInGroupMaxValue);

            final PrimitiveValue numInGroupMinValue = numInGroupType.minValue();
            if (null != numInGroupMinValue)
            {
                final long max = numInGroupMaxValue != null ?
                    numInGroupMaxValue.longValue() : numInGroupType.primitiveType().maxValue().longValue();

                if (numInGroupMinValue.longValue() > max)
                {
                    XmlSchemaParser.handleError(node, String.format(
                        "\"numInGroup\" minValue=%s greater than maxValue=%d", numInGroupMinValue, max));
                }
            }
        }
    }

    /**
     * Check the composite for being a well formed variable length data encoding. This means
     * that there are the fields "length" and "varData" present.
     *
     * @param node of the XML for this composite
     */
    public void checkForWellFormedVariableLengthDataEncoding(final Node node)
    {
        final EncodedDataType lengthType = (EncodedDataType)containedTypeByNameMap.get("length");

        if (lengthType == null)
        {
            XmlSchemaParser.handleError(node, "composite for variable length data encoding must have \"length\"");
        }
        else
        {
            final PrimitiveType primitiveType = lengthType.primitiveType();
            if (!isUnsigned(primitiveType))
            {
                XmlSchemaParser.handleError(node, "\"length\" must be unsigned type");
            }
            else if (primitiveType != UINT8 && primitiveType != UINT16 && primitiveType != UINT32)
            {
                XmlSchemaParser.handleWarning(node, "\"length\" should be UINT8, UINT16, or UINT32");
            }

            validateGroupMaxValue(node, primitiveType, lengthType.maxValue());
        }

        if ("optional".equals(getAttributeValueOrNull(node, "presence")))
        {
            XmlSchemaParser.handleError(
                node, "composite for variable length data encoding cannot have presence=\"optional\"");
        }

        if (containedTypeByNameMap.get("varData") == null)
        {
            XmlSchemaParser.handleError(node, "composite for variable length data encoding must have \"varData\"");
        }
    }

    private static final BigInteger UNSIGNED_LONG_MASK = BigInteger.ONE.shiftLeft(Long.SIZE).subtract(BigInteger.ONE);

    private static void validateGroupMaxValue(
        final Node node, final PrimitiveType primitiveType, final PrimitiveValue value)
    {
        if (null != value)
        {
            final BigInteger longValue = BigInteger.valueOf(value.longValue());
            BigInteger allowedValue = BigInteger.valueOf(primitiveType.maxValue().longValue());
            if (primitiveType == UINT64)
            {
                allowedValue = allowedValue.and(UNSIGNED_LONG_MASK);
            }
            if (longValue.compareTo(allowedValue) > 0)
            {
                XmlSchemaParser.handleError(node, String.format(
                    "maxValue greater than allowed for type: maxValue=%d allowed=%d", longValue, allowedValue));
            }
            BigInteger maxInt = null;
            if (primitiveType == UINT32)
            {
                maxInt = BigInteger.valueOf(INT32.maxValue().longValue());
            }
            else if (primitiveType == UINT64)
            {
                maxInt = BigInteger.valueOf(INT64.maxValue().longValue());
            }
            if (maxInt != null && longValue.compareTo(maxInt) > 0)
            {
                XmlSchemaParser.handleError(node, String.format(
                    "maxValue greater than allowed for type: maxValue=%d allowed=%d", longValue, maxInt));
            }
        }
        else if (primitiveType == UINT32)
        {
            final long maxInt = INT32.maxValue().longValue();
            XmlSchemaParser.handleError(node, String.format(
                "maxValue must be set for varData UINT32 type: max value allowed=%d", maxInt));
        }
        else if (primitiveType == UINT64)
        {
            final long maxInt = INT64.maxValue().longValue();
            XmlSchemaParser.handleError(node, String.format(
                "maxValue must be set for varData UINT64 type: max value allowed=%d", maxInt));
        }
    }

    /**
     * Check the composite for being a well formed message headerStructure encoding. This means
     * that there are the fields "blockLength", "templateId" and "version" present.
     *
     * @param node of the XML for this composite
     */
    public void checkForWellFormedMessageHeader(final Node node)
    {
        final boolean shouldGenerateInterfaces = Boolean.getBoolean(JAVA_GENERATE_INTERFACES);

        final EncodedDataType blockLengthType = (EncodedDataType)containedTypeByNameMap.get("blockLength");
        final EncodedDataType templateIdType = (EncodedDataType)containedTypeByNameMap.get("templateId");
        final EncodedDataType schemaIdType = (EncodedDataType)containedTypeByNameMap.get("schemaId");
        final EncodedDataType versionType = (EncodedDataType)containedTypeByNameMap.get("version");

        if (blockLengthType == null)
        {
            XmlSchemaParser.handleError(node, "composite for message header must have \"blockLength\"");
        }
        else if (!isUnsigned(blockLengthType.primitiveType()))
        {
            XmlSchemaParser.handleError(node, "\"blockLength\" must be unsigned");
        }

        validateHeaderField(node, "templateId", templateIdType, UINT16, shouldGenerateInterfaces);
        validateHeaderField(node, "schemaId", schemaIdType, UINT16, shouldGenerateInterfaces);
        validateHeaderField(node, "version", versionType, UINT16, shouldGenerateInterfaces);
    }

    private void validateHeaderField(
        final Node node,
        final String fieldName,
        final EncodedDataType actualType,
        final PrimitiveType expectedType,
        final boolean shouldGenerateInterfaces)
    {
        if (actualType == null)
        {
            XmlSchemaParser.handleError(
                node,
                String.format("composite for message header must have \"%s\"", fieldName));
        }
        else if (actualType.primitiveType() != expectedType)
        {
            XmlSchemaParser.handleWarning(node, String.format("\"%s\" should be %s", fieldName, expectedType.name()));

            if (shouldGenerateInterfaces)
            {
                if (actualType.primitiveType().size() > expectedType.size())
                {
                    XmlSchemaParser.handleError(
                        node,
                        String.format("\"%s\" must be less than %s bytes to use %s",
                        fieldName,
                        expectedType.size(),
                        JAVA_GENERATE_INTERFACES));
                }
                else
                {
                    XmlSchemaParser.handleWarning(
                        node,
                        String.format("\"%s\" will be cast to %s to use %s",
                        fieldName,
                        expectedType.name(),
                        JAVA_GENERATE_INTERFACES));
                }
            }
        }
    }

    /**
     * Check the composite for any specified offsets and validate they are correctly specified.
     *
     * @param node of the XML for this composite
     */
    public void checkForValidOffsets(final Node node)
    {
        int offset = 0;

        for (final Type edt : containedTypeByNameMap.values())
        {
            final int offsetAttribute = edt.offsetAttribute();

            if (-1 != offsetAttribute)
            {
                if (offsetAttribute < offset)
                {
                    XmlSchemaParser.handleError(
                        node,
                        String.format("composite element \"%s\" has incorrect offset specified", edt.name()));
                }

                offset = offsetAttribute;
            }

            offset += edt.encodedLength();
        }
    }

    public boolean isVariableLength()
    {
        return false;
    }

    private Type processType(
        final Node subTypeNode,
        final String subTypeName, final String givenName, final String referencedName,
        final int arrayCapacity)
        throws XPathExpressionException
    {
        final String nodeName = subTypeNode.getNodeName();
        Type type = null;

        final String typeName = XmlSchemaParser.getAttributeValueOrNull(subTypeNode, "type");

        switch (nodeName)
        {
            case "type":
                type = addType(subTypeNode, subTypeName, new EncodedDataType(subTypeNode, givenName, referencedName));
                if (arrayCapacity >= 0)
                {
                    XmlSchemaParser.handleError(
                        subTypeNode,
                        String.format("'array' member should not reference to 'type'\n" +
                        "and it can only reference to 'enum', 'set' and 'composite'."));
                }
                break;

            case "enum":
            {
                final String enumReferencedName = referencedName != null ? referencedName : typeName;
                type = addType(subTypeNode, subTypeName,
                    new EnumType(subTypeNode, givenName, enumReferencedName, arrayCapacity));
                break;
            }

            case "set":
            {
                final String setReferencedName = referencedName != null ? referencedName : typeName;
                type = addType(subTypeNode, subTypeName,
                    new SetType(subTypeNode, givenName, setReferencedName, arrayCapacity));
                break;
            }

            case "composite":
                final String compositeReferencedName = referencedName != null ? referencedName : typeName;
                type = addType(
                    subTypeNode,
                    subTypeName,
                    new CompositeType(subTypeNode, givenName, compositeReferencedName, compositesPath, arrayCapacity));
                break;

            case "array":
            case "ref":
            {
                final XPath xPath = XPathFactory.newInstance().newXPath();
                int length = -1;
                if (nodeName.equals("array"))
                {
                    length = Integer.parseInt(XmlSchemaParser.getAttributeValue(subTypeNode, "length"));
                }

                final String refName = XmlSchemaParser.getAttributeValue(subTypeNode, "name");
                final String refTypeName = XmlSchemaParser.getAttributeValue(subTypeNode, "type");
                final int refOffset = Integer.parseInt(XmlSchemaParser.getAttributeValue(subTypeNode, "offset", "-1"));
                final Node refTypeNode = (Node)xPath.compile(
                    "/*[local-name() = 'messageSchema']/types/*[@name='" + refTypeName + "']")
                    .evaluate(subTypeNode.getOwnerDocument(), XPathConstants.NODE);

                if (refTypeNode == null)
                {
                    XmlSchemaParser.handleError(subTypeNode,
                        String.format("%1$s type not found: %2$s", nodeName, refTypeName));
                }
                else
                {
                    if (compositesPath.contains(refTypeName))
                    {
                        final String err = String.format("%1$s types cannot create circular dependencies.", nodeName);
                        XmlSchemaParser.handleError(refTypeNode, err);
                        throw new IllegalStateException(err);
                    }

                    type = processType(refTypeNode, refName, refName, refTypeName, length);

                    if (-1 != refOffset)
                    {
                        type.offsetAttribute(refOffset);
                    }
                }

                break;
            }

            default:
                throw new IllegalStateException("Unknown node type: name=" + nodeName);
        }

        return type;
    }

    private Type addType(final Node subTypeNode, final String name, final Type type)
    {
        if (containedTypeByNameMap.put(name, type) != null)
        {
            XmlSchemaParser.handleError(subTypeNode, "composite already contains a type named: " + name);
        }

        return type;
    }

    public String toString()
    {
        return "CompositeType{" +
            "compositesPath=" + compositesPath +
            ", containedTypeByNameMap=" + containedTypeByNameMap +
            '}';
    }
}
