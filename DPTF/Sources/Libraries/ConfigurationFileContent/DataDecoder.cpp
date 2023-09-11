/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#include "DataDecoder.h"
using namespace std;

DataDecoder::DataDecoder(shared_ptr<DataCompressor> compressor)
	: m_compressor(compressor)
{
}

vector<vector<unsigned char>> DataDecoder::decode(const vector<unsigned char>& data) const
{
	vector<unsigned char> remainingRawData = data;
	vector<vector<unsigned char>> dataSegments;
	do
	{
		const auto header = extractFirstHeader(remainingRawData);
		const auto payload = extractFirstPayload(header, remainingRawData);
		const auto decompressedPayload = decompress(header, payload);
		dataSegments.push_back(decompressedPayload);

		const auto newBegin = remainingRawData.begin() + header.payload_offset + header.payload_size;
		remainingRawData = vector<unsigned char>(newBegin, remainingRawData.end());
	} while (remainingRawData.size() > 0);

	return dataSegments;
}

DttConfigurationHeaderV0 DataDecoder::extractFirstHeader(const vector<unsigned char>& rawData)
{
	throwIfDataSmallerThanHeader(rawData);
	const auto header = (const DttConfigurationHeaderV0*)rawData.data();
	throwIfInvalidHeader(header);
	throwIfInvalidHeaderSignature(*header);
	throwIfInvalidHeaderVersion(*header);
	return *header;
}

vector<unsigned char> DataDecoder::extractFirstPayload(const DttConfigurationHeaderV0& header, const vector<unsigned char>& rawData)
{
	throwIfInvalidHeaderPayloadOffset(header);
	throwIfInvalidPayloadSize(header, rawData);
	const auto posDataBegin = rawData.begin() + header.payload_offset;
	const auto posDataEnd = posDataBegin + header.payload_size;
	vector<unsigned char> payload{posDataBegin, posDataEnd};
	throwIfPayloadIsCorrupted(header, payload);
	return payload;
}

vector<unsigned char> DataDecoder::decompress(const DttConfigurationHeaderV0& header, const vector<unsigned char>& payload) const
{
	vector<unsigned char> extractedPayload(payload);
	if (header.flags & DttConfigurationHeaderFlagMasks::compression)
	{
		extractedPayload = m_compressor->decode(payload);
	}
	return extractedPayload;
}

void DataDecoder::throwIfInvalidHeader(const DttConfigurationHeaderV0* header)
{
	if (header == nullptr)
	{
		throw runtime_error("Header buffer is empty");
	}
}

void DataDecoder::throwIfDataSmallerThanHeader(const vector<unsigned char>& data)
{
	if (data.size() < sizeof(DttConfigurationHeaderV0))
	{
		throw runtime_error("Unexpected end of header segment");
	}
}

void DataDecoder::throwIfInvalidHeaderSignature(const DttConfigurationHeaderV0& header)
{
	if (header.signature != DttConfigurationHeaderSignature::value)
	{
		throw runtime_error("Data is corrupted, header signature does not match original");
	}
}

void DataDecoder::throwIfInvalidHeaderVersion(const DttConfigurationHeaderV0& header)
{
	if (header.version != 0)
	{
		throw runtime_error("Header version not supported");
	}
}

void DataDecoder::throwIfInvalidHeaderPayloadOffset(const DttConfigurationHeaderV0& header)
{
	if (header.payload_offset != sizeof(DttConfigurationHeaderV0))
	{
		throw runtime_error("Header payload offset invalid for header version");
	}
}

void DataDecoder::throwIfPayloadIsCorrupted(const DttConfigurationHeaderV0& header, const vector<unsigned char>& payload)
{
	const auto checksum = DataFormat::calculateChecksum(payload);
	if (header.payload_checksum != checksum)
	{
		throw runtime_error("Data is corrupted, checksum does not match original");
	}
}

void DataDecoder::throwIfInvalidPayloadSize(const DttConfigurationHeaderV0& header, const vector<unsigned char>& rawData)
{
	if (header.payload_size > rawData.size())
	{
		throw runtime_error("Payload size is larger than remaining data");
	}
}
