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

#pragma once
#include <vector>
#include <memory>
#include "DataCompressor.h"
#include "DataFormat.h"

class IDataDecoder
{
public:
	IDataDecoder() = default;
	IDataDecoder(const IDataDecoder& other) = default;
	IDataDecoder(IDataDecoder&& other) noexcept = default;
	IDataDecoder& operator=(const IDataDecoder& other) = default;
	IDataDecoder& operator=(IDataDecoder&& other) noexcept = default;
	virtual ~IDataDecoder() = default;
	virtual std::vector<std::vector<unsigned char>> decode(const std::vector<unsigned char>& data) const = 0;
};

class DataDecoder : public IDataDecoder
{
public:
	DataDecoder(std::shared_ptr<DataCompressor> compressor);
	std::vector<std::vector<unsigned char>> decode(const std::vector<unsigned char>& data) const override;

private:
	static DttConfigurationHeaderV0 extractFirstHeader(const std::vector<unsigned char>& rawData);
	static std::vector<unsigned char> extractFirstPayload(const DttConfigurationHeaderV0& header, const std::vector<unsigned char>& rawData);
	std::vector<unsigned char> decompress(
		const DttConfigurationHeaderV0& header,
		const std::vector<unsigned char>& payload) const;

	static void throwIfInvalidHeader(const DttConfigurationHeaderV0* header);
	static void throwIfDataSmallerThanHeader(const std::vector<unsigned char>& data);
	static void throwIfInvalidHeaderSignature(const DttConfigurationHeaderV0& header);
	static void throwIfInvalidHeaderVersion(const DttConfigurationHeaderV0& header);
	static void throwIfInvalidHeaderPayloadOffset(const DttConfigurationHeaderV0& header);
	static void throwIfPayloadIsCorrupted(const DttConfigurationHeaderV0& header, const std::vector<unsigned char>& payload);
	static void throwIfInvalidPayloadSize(const DttConfigurationHeaderV0& header, const std::vector<unsigned char>& rawData);
	std::shared_ptr<DataCompressor> m_compressor;
};

