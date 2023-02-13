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

#include "StringParser.h"
#include <algorithm>
using namespace std;

vector<string> StringParser::split(const string& input, char delimiter)
{
	vector<string> strings;
	string inputCopy = input;

	auto nullTerminatorLocation = inputCopy.find_first_of('\0');
	if (nullTerminatorLocation != string::npos)
	{
		inputCopy = inputCopy.substr(0, nullTerminatorLocation + 1);
	}

	try
	{
		std::stringstream stream(inputCopy);
		std::string substring;

		while (stream.eof() == false)
		{
			getline(stream, substring, delimiter);
			// remove /0's from string
			substring.erase(std::remove(substring.begin(), substring.end(), '\0'), substring.end());

			if (substring.size() > 0)
			{
				strings.push_back(substring);
			}
		}
	}
	catch (...)
	{
		// swallow any errors
	}

	return strings;
}

std::string StringParser::join(std::vector<std::string> strings, char delimiter)
{
	std::string retString = "";
	retString.push_back(delimiter);
	for (std::vector<std::string>::iterator string = strings.begin(); string != strings.end(); ++string)
	{
		retString += *string;
		retString.push_back(delimiter);
	}

	return retString;
}

std::string StringParser::removeString(const std::string& input, const std::string& substring)
{
	string inputCopy = input;
	std::size_t foundPosition = inputCopy.find(substring);

	if (foundPosition != std::string::npos)
	{
		inputCopy.erase(foundPosition, substring.length());
	}

	return inputCopy;
}

std::string StringParser::removeAllString(const std::string& input, const std::string& substring)
{
	string inputCopy = input;
	std::size_t foundPosition = inputCopy.find(substring);

	while (foundPosition != std::string::npos)
	{
		inputCopy.erase(foundPosition, substring.length());
		foundPosition = inputCopy.find(substring);
	}

	return inputCopy;
}

std::string StringParser::removeCharacter(const std::string& input, char character)
{
	string inputCopy = input;
	std::size_t foundPosition = inputCopy.find(character);

	if (foundPosition != std::string::npos)
	{
		inputCopy.erase(foundPosition, 1);
	}

	return inputCopy;
}

std::string StringParser::replaceAll(
	const std::string& input,
	const std::string& findString,
	const std::string& replaceString)
{
	std::string inputCopy = input;
	size_t index = inputCopy.find(findString, 0);
	while (index != (size_t)std::string::npos)
	{
		inputCopy.replace(index, findString.size(), replaceString);
		index = inputCopy.find(findString, index + 1);
	}
	return inputCopy;
}

std::string StringParser::removeAll(const std::string& input, char character)
{
	std::string inputCopy = input;
	inputCopy.erase(std::remove(inputCopy.begin(), inputCopy.end(), character), inputCopy.end());

	return inputCopy;
}