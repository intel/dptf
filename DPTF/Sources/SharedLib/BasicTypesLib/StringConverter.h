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

#include "Dptf.h"

// TODO: consider extending std::string and putting this function there.
class StringConverter final
{
public:
	static UInt32 toUInt32(const std::string& input);
	static UInt64 toUInt64(const std::string& input);
	static UInt32 toUInt32(const std::string& input, UInt32 defaultValue);
	static Int32 toInt32(const std::string& input);
	static double toDouble(const std::string& input);
	static float toFloat(const std::string& input);
	static std::string toHexString(UInt8 value);
	static std::string toHexString(UInt16 value);
	static std::string toHexString(UInt32 value);
	static std::string toHexString(UInt64 value);
	static std::string toUpper(const std::string& input);
	static std::string toLower(const std::string& input);
	static std::string trimWhitespace(const std::string& input);
	static std::string trimQuotes(const std::string& input);
	static std::string clone(const std::string& input);
	static std::string toString(const std::wstring& input);
	static std::wstring toWideString(const std::string& input);
};
