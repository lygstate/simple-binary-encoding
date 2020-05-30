/*
 * Copyright 2013-2020 Real Logic Limited.
 * Copyright 2017 MarketFactory Inc.
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

import uk.co.real_logic.sbe.TestUtil;

import org.junit.jupiter.api.Test;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.core.Is.is;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.fail;
import static uk.co.real_logic.sbe.xml.XmlSchemaParser.parse;

public class ErrorHandlerTest
{
    @Test
    public void shouldNotExitOnTypeErrorsAndWarnings()
        throws Exception
    {
        final String testXmlString =
            "<messageSchema package=\"SBE tests\" id=\"3\" semanticVersion=\"5.2\">" +
            "<types>" +
            "<enum name=\"NullBoolean\" encodingType=\"uint8\" nullValue=\"255\" semanticType=\"Boolean\">" +
            "    <validValue name=\"false\">0</validValue>" +
            "    <validValue name=\"true\">1</validValue>" +
            "</enum>" +
            "<enum name=\"DupNameBoolean\" encodingType=\"uint8\" semanticType=\"Boolean\">" +
            "    <validValue name=\"false\">0</validValue>" +
            "    <validValue name=\"anotherFalse\">0</validValue>" +
            "    <validValue name=\"true\">1</validValue>" +
            "</enum>" +
            "<enum name=\"DupValBoolean\" encodingType=\"uint8\" semanticType=\"Boolean\">" +
            "    <validValue name=\"false\">0</validValue>" +
            "    <validValue name=\"false\">2</validValue>" +
            "    <validValue name=\"true\">1</validValue>" +
            "</enum>" +
            "<set name=\"DupValueSet\" encodingType=\"uint8\">" +
            "    <choice name=\"Bit0\">0</choice>" +
            "    <choice name=\"AnotherBit0\">0</choice>" +
            "</set>" +
            "<set name=\"DupNameSet\" encodingType=\"uint8\">" +
            "    <choice name=\"Bit0\">0</choice>" +
            "    <choice name=\"Bit0\">1</choice>" +
            "</set>" +
            "<enum name=\"Valid16BitEnum\" encodingType=\"uint16\" semanticType=\"int\">" +
            "    <validValue name=\"min_value\">0</validValue>" +
            "    <validValue name=\"rand_value\">2000</validValue>" +
            "    <validValue name=\"max_value\">3999</validValue>" +
            "</enum>" +
            "<type name=\"Valid16BitUint\" primitiveType=\"uint16\" />" +
            "<enum name=\"Valid16BitEnumType\" encodingType=\"Valid16BitUint\" semanticType=\"int\">" +
            "    <validValue name=\"min_value\">0</validValue>" +
            "    <validValue name=\"rand_value\">2000</validValue>" +
            "    <validValue name=\"max_value\">3999</validValue>" +
            "</enum>" +
            "<set name=\"DupNameBitfield\" encodingType=\"uint8\">" +
            "    <bits name=\"Bits0\" msb=\"0\" lsb=\"0\" description=\"Bits 0\"></bits>" +
            "    <bits name=\"Bits0\" msb=\"1\" lsb=\"1\" description=\"Bits 1\"></bits>" +
            "</set>" +
            "<set name=\"DupValueBitfield\" encodingType=\"uint8\">" +
            "    <bits name=\"Bits0\" msb=\"0\" lsb=\"0\" description=\"Bits 0\"></bits>" +
            "    <bits name=\"Bits1\" msb=\"0\" lsb=\"0\" description=\"Bits 1\"></bits>" +
            "</set>" +
            "<set name=\"OverlapValueBitfield\" encodingType=\"uint8\">" +
            "    <bits name=\"Bits30\" msb=\"3\" lsb=\"0\" description=\"Bits 3-0\"></bits>" +
            "    <bits name=\"Bits73\" msb=\"7\" lsb=\"3\" description=\"Bits 7-3\"></bits>" +
            "</set>" +
            "<set name=\"MsbOutRangeBitfield\" encodingType=\"uint8\">" +
            "    <bits name=\"Bits9\" msb=\"9\" lsb=\"0\" description=\"Bit 9-0\"></bits>" +
            "</set>" +
            "<set name=\"LsbOutRangeBitfield\" encodingType=\"uint8\">" +
            "    <bits name=\"Bits9\" msb=\"0\" lsb=\"9\" description=\"Bit 0-9\"></bits>" +
            "</set>" +
            "<set name=\"EncodingTypeOutRangeBitfield\" encodingType=\"uint8\">" +
            "    <bits name=\"Bits03\" msb=\"0\" lsb=\"3\" type=\"uint16\" description=\"Bit 0-3\"></bits>" +
            "</set>" +
            "<set name=\"IndirectEnumEncodingTypeOutRangeBitfield\" encodingType=\"uint8\">" +
            "    <bits name=\"Bits03\" msb=\"0\" lsb=\"3\" type=\"Valid16BitEnum\"/>" +
            "</set>" +
            "<set name=\"IndirectEncodingTypeNotFoundBitfield\" encodingType=\"uint8\">" +
            "    <bits name=\"Bits03\" msb=\"0\" lsb=\"3\" type=\"NotFoundType\"/>" +
            "</set>" +
            "<set name=\"IndirectTypeEncodingTypeOutRangeBitfield\" encodingType=\"uint8\">" +
            "    <bits name=\"Bits03\" msb=\"0\" lsb=\"3\" type=\"Valid16BitUint\"/>" +
            "</set>" +
            "<set name=\"IndirectEnumTypeEncodingTypeOutRangeBitfield\" encodingType=\"uint8\">" +
            "    <bits name=\"Bits03\" msb=\"0\" lsb=\"3\" type=\"Valid16BitEnumType\"/>" +
            "</set>" +
            "<composite name=\"decimal\">" +
            "    <type name=\"mantissa\" primitiveType=\"int64\"/>" +
            "    <type name=\"mantissa\" primitiveType=\"int64\"/>" +
            "    <type name=\"exponent\" primitiveType=\"int8\"/>" +
            "</composite>" +
            "<type name=\"ConstButNoValue\" primitiveType=\"char\" presence=\"constant\"></type>" +
            "<type name=\"NullButNotOptional\" primitiveType=\"int8\" presence=\"required\" nullValue=\"10\"/>" +
            "</types>" +
            "</messageSchema>";

        final Map<String, Type> map = new HashMap<>();
        final ParserOptions options = ParserOptions.builder().suppressOutput(true).build();
        final ErrorHandler handler = new ErrorHandler(options);

        parseTestXmlAddToMap(map, "/messageSchema/types/composite", testXmlString, handler);
        parseTestXmlAddToMap(map, "/messageSchema/types/type", testXmlString, handler);
        parseTestXmlAddToMap(map, "/messageSchema/types/enum", testXmlString, handler);
        assertThat(handler.errorCount(), is(3));
        /* There is no duplicate warning for different language now */
        assertThat(handler.warningCount(), is(10));

        parseTestXmlAddToMap(map, "/messageSchema/types/set", testXmlString, handler);
        assertThat(handler.errorCount(), is(3 + 10));
        assertThat(handler.warningCount(), is(12));
    }

    @Test
    public void shouldExitAfterTypes()
        throws Exception
    {
        try
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).build();
            parse(TestUtil.getLocalResource("error-handler-types-schema.xml"), options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 2 errors", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    public void shouldExitAfterTypesWhenDupTypesDefined()
        throws Exception
    {
        try
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(TestUtil.getLocalResource("error-handler-types-dup-schema.xml"), options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 1 warnings", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    public void shouldExitAfterMessageWhenDupMessageIdsDefined()
        throws Exception
    {
        try
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(TestUtil.getLocalResource("error-handler-dup-message-schema.xml"), options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 1 errors", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    public void shouldExitAfterMessage()
        throws Exception
    {
        try
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(TestUtil.getLocalResource("error-handler-message-schema.xml"), options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 13 errors", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    public void shouldExitAfterMessageWhenGroupDimensionsNotComposite()
        throws Exception
    {
        try
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(TestUtil.getLocalResource("error-handler-group-dimensions-schema.xml"), options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 1 errors", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    public void shouldExitAfterTypesWhenCompositeOffsetsIncorrect()
        throws Exception
    {
        try
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(TestUtil.getLocalResource("error-handler-invalid-composite-offsets-schema.xml"), options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 2 errors", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    @Test
    public void shouldExitInvalidFieldNames()
        throws Exception
    {
        try
        {
            final ParserOptions options = ParserOptions.builder().suppressOutput(true).warningsFatal(true).build();
            parse(TestUtil.getLocalResource("error-handler-invalid-name.xml"), options);
        }
        catch (final IllegalStateException ex)
        {
            assertEquals("had 4 warnings", ex.getMessage());
            return;
        }

        fail("expected IllegalStateException");
    }

    private static void parseTestXmlAddToMap(
        final Map<String, Type> map, final String xPathExpr, final String xml, final ErrorHandler handler)
        throws ParserConfigurationException, XPathExpressionException, IOException, SAXException
    {
        final Document document = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(
            new ByteArrayInputStream(xml.getBytes()));
        final XPath xPath = XPathFactory.newInstance().newXPath();
        final NodeList list = (NodeList)xPath.compile(xPathExpr).evaluate(document, XPathConstants.NODESET);

        document.setUserData(XmlSchemaParser.ERROR_HANDLER_KEY, handler, null);

        for (int i = 0, size = list.getLength(); i < size; i++)
        {
            Type type = null;

            if (xPathExpr.endsWith("enum"))
            {
                type = new EnumType(list.item(i));
            }
            else if (xPathExpr.endsWith("set"))
            {
                type = new SetType(list.item(i));
            }
            else if (xPathExpr.endsWith("type"))
            {
                type = new EncodedDataType(list.item(i));
            }
            else if (xPathExpr.endsWith("composite"))
            {
                type = new CompositeType(list.item(i));
            }

            if (type != null)
            {
                map.put(type.name(), type);
            }
        }
    }
}
