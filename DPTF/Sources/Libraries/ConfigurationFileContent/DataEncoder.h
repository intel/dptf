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

#pragma once
#include <memory>
#include <vector>
#include "DataCompressor.h"
#include "DataFormat.h"

class IDataEncoder
{
public:
	virtual ~IDataEncoder() = default;
	virtual std::vector<unsigned char> encodeCompressed(const std::vector<unsigned char>& data) const = 0;
	virtual std::vector<unsigned char> encodeUncompressed(const std::vector<unsigned char>& data) const = 0;
};

class DataEncoder : public IDataEncoder
{
public:
	DataEncoder(std::shared_ptr<DataCompressor> compressor);
	std::vector<unsigned char> encodeCompressed(const std::vector<unsigned char>& data) const override;
	std::vector<unsigned char> encodeUncompressed(const std::vector<unsigned char>& data) const override;

private:
	static DttConfigurationHeaderV0 createHeaderCompressed(const std::vector<unsigned char>& payload);
	static DttConfigurationHeaderV0 createHeaderUncompressed(const std::vector<unsigned char>& payload);
	static std::vector<unsigned char> createByteStreamFromHeader(const DttConfigurationHeaderV0& header);
	std::shared_ptr<DataCompressor> m_compressor;
};

