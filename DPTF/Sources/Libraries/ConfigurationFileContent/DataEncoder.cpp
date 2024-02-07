/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "DataEncoder.h"
using namespace std;

DataEncoder::DataEncoder(shared_ptr<DataCompressor> compressor)
	: IDataEncoder(), m_compressor(move(compressor))
{
}

vector<unsigned char> DataEncoder::encodeCompressed(const vector<unsigned char>& data) const
{
	const vector<unsigned char> compressedPayload(m_compressor->encode(data));
	const DttConfigurationHeaderV0 header = createHeaderCompressed(compressedPayload);
	auto encodedData = createByteStreamFromHeader(header);
	for (auto p : compressedPayload)
	{
		encodedData.push_back(p);
	}
	return encodedData;
}

vector<unsigned char> DataEncoder::encodeUncompressed(const vector<unsigned char>& data) const
{
	const DttConfigurationHeaderV0 header = createHeaderUncompressed(data);
	auto encodedData = createByteStreamFromHeader(header);
	for (auto p : data)
	{
		encodedData.push_back(p);
	}
	return encodedData;
}

DttConfigurationHeaderV0 DataEncoder::createHeaderCompressed(const vector<unsigned char>& payload)
{
	DttConfigurationHeaderV0 header;
	header.signature = DttConfigurationHeaderSignature::value;
	header.version = 0;
	header.flags = 1;
	header.payload_offset = sizeof(header);
	header.payload_checksum = DataFormat::calculateChecksum(payload);
	header.payload_size = static_cast<unsigned>(payload.size());
	return header;
}

DttConfigurationHeaderV0 DataEncoder::createHeaderUncompressed(const vector<unsigned char>& payload)
{
	DttConfigurationHeaderV0 header;
	header.signature = DttConfigurationHeaderSignature::value;
	header.version = 0;
	header.flags = 0;
	header.payload_offset = sizeof(header);
	header.payload_checksum = DataFormat::calculateChecksum(payload);
	header.payload_size = static_cast<unsigned>(payload.size());
	return header;
}

vector<unsigned char> DataEncoder::createByteStreamFromHeader(const DttConfigurationHeaderV0& header)
{
	DttConfigurationHeaderV0 headerCopy = header;
	const unsigned char* headerData = reinterpret_cast<unsigned char*>(&headerCopy);
	vector<unsigned char> headerBytes(sizeof(header));
	for (size_t i = 0; i < sizeof(header); ++i)
	{
		headerBytes[i] = headerData[i];
	}
	return headerBytes;
}