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

#include <string>
#include <vector>
#include "esif_ccb.h"
#include "esif_ccb_rc.h"

class Base64
{
public:
	static std::vector<unsigned char> decode(const std::vector<unsigned char>& base64EncodedData);
	static std::vector<unsigned char> decode(const std::string& base64EncodedData);
	static std::vector<unsigned char> encode(const std::vector<unsigned char>& data);
	static std::vector<unsigned char> encode(const std::string& data);

private:
	static esif_error_t base64Decode(
		void* targetBuf,
		size_t* targetLen,
		const char* encodedBuf,
		size_t encodedLen
	);
	static esif_error_t base64Encode(
		char* target_str, 
		size_t target_len, 
		const void* source_buf, 
		size_t source_len);
	static std::vector<unsigned char> trimTrailingZeros(const std::vector<unsigned char>& decodedData);
};
