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
package uk.co.real_logic.sbe.ir;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;

/**
 * Common code generation utility functions to be used by the different language specific backends.
 */
public final class GenerationUtil
{
    public static int collectFields(final List<Token> tokens, final int index, final List<Token> fields)
    {
        return collect(Signal.BEGIN_FIELD, tokens, index, fields);
    }

    public static int collectGroups(final List<Token> tokens, final int index, final List<Token> groups)
    {
        return collect(Signal.BEGIN_GROUP, tokens, index, groups);
    }

    public static int collectVarData(final List<Token> tokens, final int index, final List<Token> varData)
    {
        return collect(Signal.BEGIN_VAR_DATA, tokens, index, varData);
    }

    public static int collect(
        final Signal signal, final List<Token> tokens, final int index, final List<Token> collected)
    {
        int i = index;
        while (i < tokens.size())
        {
            final Token token = tokens.get(i);
            if (signal != token.signal())
            {
                break;
            }

            final int tokenCount = token.componentTokenCount();
            for (final int limit = i + tokenCount; i < limit; i++)
            {
                collected.add(tokens.get(i));
            }
        }

        return i;
    }

    public static List<Token> getMessageBody(final List<Token> tokens)
    {
        return tokens.subList(1, tokens.size() - 1);
    }

    public static int findEndSignal(
        final List<Token> tokens, final int startIndex, final Signal signal, final String name)
    {
        int result = tokens.size() - 1;

        for (int i = startIndex, endIndex = tokens.size() - 1; i < endIndex; i++)
        {
            final Token token = tokens.get(i);

            if (signal == token.signal() && name.equals(token.name()))
            {
                result = i;
                break;
            }
        }

        return result;
    }

    public static List<String> findSubGroupNames(final List<Token> tokens)
    {
        final ArrayList<String> groupNames = new ArrayList<>();
        int level = 0;

        for (final Token token : tokens)
        {
            if (token.signal() == Signal.BEGIN_GROUP)
            {
                if (level++ == 0)
                {
                    groupNames.add(token.name());
                }
            }

            if (token.signal() == Signal.END_GROUP)
            {
                level--;
            }
        }

        return groupNames;
    }

    public static int findSignal(final List<Token> tokens, final Signal signal)
    {
        for (int i = 0, endIndex = tokens.size() - 1; i < endIndex; i++)
        {
            if (signal == tokens.get(i).signal())
            {
                return i;
            }
        }

        return -1;
    }

    public static void getCompositeChildren(
        final HashMap<String, ArrayList<String>> graph,
        final HashMap<String, Long> inDegrees,
        final List<Token> tokens)
    {
        final Token firstToken = tokens.get(0);
        for (int i = 1; i < tokens.size() - 1;)
        {
            final Token fieldToken = tokens.get(i);
            switch (fieldToken.signal())
            {
                case BEGIN_COMPOSITE:
                {
                    if (inDegrees.containsKey(fieldToken.applicableTypeName()))
                    {
                        final long newValue = inDegrees.get(firstToken.applicableTypeName()) + 1;
                        inDegrees.put(firstToken.applicableTypeName(), newValue);
                        final ArrayList<String> list = graph.getOrDefault(
                            fieldToken.applicableTypeName(), new ArrayList<String>());
                        list.add(firstToken.applicableTypeName());
                        graph.put(fieldToken.applicableTypeName(), list);
                    }
                    else
                    {
                        System.out.println(String.format("Can not found key %1$s for %1$s",
                            fieldToken.applicableTypeName(), firstToken.applicableTypeName()));
                    }
                    break;
                }

                default:
                    break;
            }

            i += tokens.get(i).componentTokenCount();
        }
    }

    public static ArrayList<List<Token>> sortTypes(final Ir ir)
    {
        final HashMap<String, Long> inDegrees = new HashMap<String, Long>();
        final HashMap<String, ArrayList<String>> graph = new HashMap<String, ArrayList<String>>();
        final ArrayList<List<Token>> enumTokens = new ArrayList<List<Token>>();
        final ArrayList<List<Token>> otherTokens = new ArrayList<List<Token>>();
        final Collection<List<Token>> types = ir.types();
        for (final List<Token> type : types)
        {
            switch (type.get(0).signal())
            {
                case BEGIN_COMPOSITE:
                    inDegrees.putIfAbsent(type.get(0).applicableTypeName(), 0L);
                    break;
                default:
                    break;
            }
        }
        for (final List<Token> type : types)
        {
            switch (type.get(0).signal())
            {
                case BEGIN_COMPOSITE:
                    getCompositeChildren(graph, inDegrees, type);
                    break;
                case BEGIN_ENUM:
                    enumTokens.add(type);
                    break;
                default:
                    otherTokens.add(type);
                    break;
            }
        }
        enumTokens.addAll(otherTokens);
        // TopologicalSorts
        while (inDegrees.size() > 0)
        {
            final String[] keys = inDegrees.keySet().toArray(new String[inDegrees.size()]);
            for (final String key : keys)
            {
                if (inDegrees.get(key) == 0)
                {
                    inDegrees.remove(key);
                    if (ir.getType(key) == null)
                    {
                        System.out.println(String.format("Key not found %1$s", key));
                    }
                    else
                    {
                        enumTokens.add(ir.getType(key));
                    }
                    for (final String edge: graph.getOrDefault(key, new ArrayList<String>()))
                    {
                        inDegrees.put(edge, inDegrees.get(edge) - 1);
                    }
                }
            }
        }
        return enumTokens;
    }

    public static class MessageItem
    {
        public final MessageItem parent;
        public final Token rootToken;
        public final List<Token> tokens;
        public final List<Token> fields;
        public final List<Token> varData;
        public final ArrayList<MessageItem> children;
        public final ArrayList<String> classPath;

        public MessageItem(
            final List<Token> fields,
            final List<Token> varData
        )
        {
            this.rootToken = null;
            this.parent = null;
            this.tokens = new ArrayList<>();
            this.fields = fields;
            this.varData = varData;
            this.children = new ArrayList<>();
            this.classPath = null;
        }

        public MessageItem(
            final MessageItem parent,
            final List<Token> tokens
        )
        {
            this.parent = parent;
            this.rootToken = tokens.get(0);
            this.tokens = tokens;
            fields = new ArrayList<>();
            varData = new ArrayList<>();
            children = new ArrayList<>();
            this.classPath = getClassPath();
        }

        public ArrayList<String> getClassPath()
        {
            MessageItem info = this;
            final ArrayList<String> names = new ArrayList<String>();
            while (info != null)
            {
                info = info.parent;
                if (info != null)
                {
                    names.add(0, info.rootToken.name());
                }
            }
            return names;
        }

        public boolean isConst()
        {
            return this.children.size() + this.varData.size() == 0;
        }
    }

    public static MessageItem getMessageItemList(
        final ArrayList<MessageItem> infoList,
        final MessageItem parent,
        final List<Token> tokens)
    {
        final MessageItem info = new MessageItem(parent, tokens);

        int i = 1;
        if (tokens.get(0).signal() == Signal.BEGIN_GROUP)
        {
            final int groupHeaderTokenCount = tokens.get(i).componentTokenCount();
            i += groupHeaderTokenCount;
        }

        i = collectFields(tokens, i, info.fields);

        while (i < tokens.size())
        {
            if (tokens.get(i).signal() != Signal.BEGIN_GROUP)
            {
                break;
            }
            final int tokenCount = tokens.get(i).componentTokenCount();
            final List<Token> groupTokens = tokens.subList(i, i + tokenCount);
            i += tokenCount;
            info.children.add(getMessageItemList(infoList, info, groupTokens));
        }

        i = collectVarData(tokens, i, info.varData);

        if ((i + 1) != tokens.size())
        {
            throw new IllegalStateException(
                String.format(
                    "tokens must be just handled once fields/groups/varData are handled i:%d size:%d",
                    i, tokens.size()));
        }
        infoList.add(info);
        return info;
    }

    /**
     * Format a scope to a String.
     *
     * @param scope to be fromat.
     * @return the string formatted as a name.
     */
    public static String formatScope(final CharSequence[] scope)
    {
        return String.join("_", scope).toLowerCase();
    }
}
