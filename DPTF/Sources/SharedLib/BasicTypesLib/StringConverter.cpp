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

#include <cctype>
#include "StringConverter.h"
#include <algorithm>

using namespace std;

UInt32 StringConverter::toUInt32(const string& input)
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

UInt32 StringConverter::toUInt32(const string& input, UInt32 defaultValue)
{
	try
	{
		return toUInt32(input);
	}
	catch (...)
	{
		return defaultValue;
	}
}


UInt64 StringConverter::toUInt64(const string& input)
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

string StringConverter::toUpper(const string& input)
{
	stringstream stream;

	for (const char i : input)
	{
		stream << (char)toupper(i);
	}

	return stream.str();
}

string StringConverter::toLower(const string& input)
{
	stringstream stream;
	for (const char i : input)
	{
		stream << (char)tolower(i);
	}
	return stream.str();
}

string StringConverter::trimWhitespace(const string& input)
{
	const string delimiters = " \f\n\r\t\v";
	string trimmedString = input;
	trimmedString.erase(0, trimmedString.find_first_not_of(delimiters));
	trimmedString.erase(trimmedString.find_last_not_of(delimiters) + 1);
	trimmedString.erase(find(trimmedString.begin(), trimmedString.end(), '\0'), trimmedString.end());
	return trimmedString;
}

string StringConverter::trimQuotes(const string& input)
{
	string trimmedString = input;
	while (!trimmedString.empty() && ((trimmedString.front() == '\"') || (trimmedString.front() == '\'')))
	{
		trimmedString.erase(0, 1);
	}
	while (!trimmedString.empty() && ((trimmedString.back() == '\"') || (trimmedString.back() == '\'')))
	{
		trimmedString.pop_back();
	}
	return trimmedString;
}

Int32 StringConverter::toInt32(const string& input)
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

double StringConverter::toDouble(const string& input)
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

float StringConverter::toFloat(const string& input)
{
	float value(0);
	istringstream stream(input);
	stream >> value;
	if (stream.fail())
	{
		throw dptf_exception("Failed to convert string \"" + input + "\" to float value.");
	}
	return value;
}

string StringConverter::toHexString(const UInt8 value)
{
	stringstream stream;
	stream << "0x" << uppercase << hex << (UInt32)value;
	return stream.str();
}

string StringConverter::toHexString(const UInt16 value)
{
	stringstream stream;
	stream << "0x" << uppercase << hex << value;
	return stream.str();
}

string StringConverter::toHexString(const UInt32 value)
{
	stringstream stream;
	stream << "0x" << uppercase << hex << value;
	return stream.str();
}

string StringConverter::toHexString(const UInt64 value)
{
	stringstream stream;
	stream << "0x" << uppercase << hex << value;
	return stream.str();
}

string StringConverter::clone(const string& input)
{
	string clonedString{};
	clonedString.reserve(input.size());
	for_each(input.begin(), input.end(), [&clonedString](const char c){clonedString.push_back(c);});
	return clonedString;
}

string StringConverter::toString(const wstring& input)
{
	string result{};
	result.reserve(input.size());
	for_each(input.begin(), input.end(), 
		[&result](const wchar_t c) { result.push_back(static_cast<char>(c)); });
	return result;
}

wstring StringConverter::toWideString(const string& input)
{
	wstring result{};
	result.reserve(input.size());
	for_each(input.begin(), input.end(), 
		[&result](const char c) { result.push_back(static_cast<wchar_t>(c)); });
	return result;
}
