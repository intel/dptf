/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

vector<string> StringParser::split(string input, char delimiter)
{
	vector<string> strings;

	try
	{
		std::stringstream stream(input);
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

std::string StringParser::removeString(std::string input, std::string substring)
{
	std::size_t foundPosition = input.find(substring);

	if (foundPosition != std::string::npos)
	{
		input.erase(foundPosition, substring.length());
	}

	return input;
}
