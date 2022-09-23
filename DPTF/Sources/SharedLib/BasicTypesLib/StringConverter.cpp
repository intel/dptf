/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include <ctype.h>
#include "StringConverter.h"
#include <algorithm>

using namespace std;

UInt32 StringConverter::toUInt32(const std::string& input)
{
	UInt32 integer(0);
	istringstream stream(input);
	stream >> integer;
	if (stream.fail())
	{
		throw dptf_exception("Failed to convert string \"" + input + "\" to UInt32 value.");
	}
	return integer;
}

UInt64 StringConverter::toUInt64(const std::string& input)
{
	UInt64 integer(0);
	istringstream stream(input);
	stream >> integer;
	if (stream.fail())
	{
		throw dptf_exception("Failed to convert string \"" + input + "\" to UInt64 value.");
	}
	return integer;
}

std::string StringConverter::toUpper(const std::string& input)
{
	std::string upperCaseString;

	for (auto i = input.begin(); i != input.end(); i++)
	{
		upperCaseString += (char)toupper(*i);
	}

	return upperCaseString;
}

std::string StringConverter::toLower(const std::string& input)
{
	std::string lowerCaseString;

	for (auto i = input.begin(); i != input.end(); i++)
	{
		lowerCaseString += (char)tolower(*i);
	}

	return lowerCaseString;
}

std::string StringConverter::trimWhitespace(const std::string& input)
{
	std::string delimiters = " \f\n\r\t\v";
	std::string trimmedString = input;
	trimmedString.erase(0, trimmedString.find_first_not_of(delimiters));
	trimmedString.erase(trimmedString.find_last_not_of(delimiters) + 1);
	trimmedString.erase(std::find(trimmedString.begin(), trimmedString.end(), '\0'), trimmedString.end());
	return trimmedString;
}

Int32 StringConverter::toInt32(const std::string& input)
{
	Int32 integer(0);
	istringstream stream(input);
	stream >> integer;
	if (stream.fail())
	{
		throw dptf_exception("Failed to convert string \"" + input + "\" to Int32 value.");
	}
	return integer;
}

double StringConverter::toDouble(const std::string& input)
{
	double value(0);
	istringstream stream(input);
	stream >> value;
	if (stream.fail())
	{
		throw dptf_exception("Failed to convert string \"" + input + "\" to double value.");
	}
	return value;
}
