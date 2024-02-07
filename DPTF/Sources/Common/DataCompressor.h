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

class DataCompressor
{
public:
	DataCompressor() = default;
	DataCompressor(const DataCompressor& other) = default;
	DataCompressor(DataCompressor&& other) noexcept = default;
	DataCompressor& operator=(const DataCompressor& other) = default;
	DataCompressor& operator=(DataCompressor&& other) noexcept = default;
	virtual ~DataCompressor() = default;
	virtual std::vector<unsigned char> encode(const std::vector<unsigned char>& data) const = 0;
	virtual std::vector<unsigned char> decode(const std::vector<unsigned char>& data) const = 0;
};