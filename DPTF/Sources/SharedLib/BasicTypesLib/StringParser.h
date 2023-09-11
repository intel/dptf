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

class StringParser final
{
public:
	static std::vector<std::string> split(const std::string& input, char delimiter);
	static std::string join(std::vector<std::string> strings, char delimiter);
	static std::string removeString(const std::string& input, const std::string& substring);
	static std::string removeAllString(const std::string& input, const std::string& substring);
	static std::string removeCharacter(const std::string& input, char character);
	static std::string replaceAll(
		const std::string& input,
		const std::string& findString,
		const std::string& replaceString);
	static std::string removeAll(const std::string& input, char character);
	static std::string removeTrailingZeros(const std::string& input);
	static size_t countWords(const std::string& input, const std::string& word);
	static std::string fromCharacters(const std::vector<unsigned char>& characters);
	static std::string trimWhitespace(const std::string& input, const std::string& whitespace = " \t\n\r\f\v");
	static std::string trimNulls(const std::string& input);
};
