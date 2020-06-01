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
#ifndef _SBE_MARKET_DATA_CODEC_BENCH_HPP
#define _SBE_MARKET_DATA_CODEC_BENCH_HPP

#include "CodecBench.h"
#include "uk_co_real_logic_sbe_benchmarks_fix/uk_co_real_logic_sbe_benchmarks_fix_cpp.h"

using namespace uk::co::real_logic::sbe::benchmarks::fix;

class SbeMarketDataCodecBench : public CodecBench<SbeMarketDataCodecBench>
{
public:
    virtual std::uint64_t encode(char *buffer, const std::uint64_t bufferLength)
    {
        messageHeader_.wrap(buffer, 0, 0, bufferLength);
        messageHeader_.blockLength(marketData_.sbeBlockLength());
        messageHeader_.templateId(marketData_.sbeTemplateId());
        messageHeader_.schemaId(marketData_.sbeSchemaId());
        messageHeader_.version(marketData_.sbeSchemaVersion());

        marketData_.wrapForEncode(buffer + messageHeader_.encodedLength(), 0, bufferLength);
        marketData_.TransactTime(1234L);
        marketData_.EventTimeDelta(987);
        marketData_.MatchEventIndicator(MatchEventIndicator::END_EVENT);

        MarketDataIncrementalRefreshTradesGroups::MdIncGrp &mdIncGrp = marketData_.MdIncGrpCount(2);

        mdIncGrp.next();
        mdIncGrp.TradeId(1234L);
        mdIncGrp.SecurityId(56789L);
        mdIncGrp.MdEntryPx().mantissa(50);
        mdIncGrp.MdEntrySize().mantissa(10);
        mdIncGrp.NumberOfOrders(1);
        mdIncGrp.MdUpdateAction(MDUpdateAction::NEW);
        mdIncGrp.RptSeq((short)1);

        mdIncGrp.next();
        mdIncGrp.TradeId(1234L);
        mdIncGrp.SecurityId(56789L);
        mdIncGrp.MdEntryPx().mantissa(50);
        mdIncGrp.MdEntrySize().mantissa(10);
        mdIncGrp.NumberOfOrders(1);
        mdIncGrp.MdUpdateAction(MDUpdateAction::NEW);
        mdIncGrp.RptSeq((short)1);

        return messageHeader::encodedLength() + marketData_.encodedLength();
    };

    virtual std::uint64_t decode(const char *buffer, const std::uint64_t bufferLength)
    {
        int64_t actingVersion;
        int64_t actingBlockLength;

        messageHeader_.wrap((char *)buffer, 0, 0, bufferLength);

        actingBlockLength = messageHeader_.blockLength();
        actingVersion = messageHeader_.version();


        marketData_.wrapForDecode((char *)buffer, messageHeader_.encodedLength(), actingBlockLength, actingVersion, bufferLength);

        static_cast<void>(marketData_.TransactTime());
        static_cast<void>(marketData_.EventTimeDelta());
        static_cast<void>(marketData_.MatchEventIndicator());

        MarketDataIncrementalRefreshTradesGroups::MdIncGrp &mdIncGrp = marketData_.MdIncGrp();
        while (mdIncGrp.hasNext())
        {
            mdIncGrp.next();
            static_cast<void>(mdIncGrp.TradeId());
            static_cast<void>(mdIncGrp.SecurityId());
            static_cast<void>(mdIncGrp.MdEntryPx().mantissa());
            static_cast<void>(mdIncGrp.MdEntrySize().mantissa());
            static_cast<void>(mdIncGrp.NumberOfOrders());
            static_cast<void>(mdIncGrp.MdUpdateAction());
            static_cast<void>(mdIncGrp.RptSeq());
            static_cast<void>(mdIncGrp.AggressorSide());
            static_cast<void>(mdIncGrp.MdEntryType());
        }

        return messageHeader::encodedLength() + marketData_.encodedLength();
    };

private:
    messageHeader messageHeader_;
    MarketDataIncrementalRefreshTrades marketData_;
};

#endif /* _SBE_MARKET_DATA_CODEC_BENCH_HPP */
