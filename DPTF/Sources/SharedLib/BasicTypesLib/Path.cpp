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

#include "Path.h"

using namespace std;

const string illegalConfigurationPathCharacters = "*<>:|?";

Path::Path(const string& basePath, const string& key)
	: basePath(basePath)
	, key(key)
	, fullPath(basePath + "\\" + key)
{
}

Path::Path(const string& fullPath)
	: basePath(extractBasePathIfValid(fullPath))
	, key(extractKeyIfValid(fullPath))
	, fullPath(fullPath)
{
}

string Path::extractBasePathIfValid(const string& configPath)
{
	throwIfInvalidConfigPath(configPath);
	size_t pathDelimiterIndex = configPath.find_last_of("/\\");

	return configPath.substr(0, pathDelimiterIndex);
}

string Path::extractKeyIfValid(const string& configPath)
{
	throwIfInvalidConfigPath(configPath);
	size_t pathDelimiterIndex = configPath.find_last_of("/\\");

	return configPath.substr(pathDelimiterIndex + 1, strlen(configPath.c_str()) - (pathDelimiterIndex + 1));
}

void Path::throwIfInvalidConfigPath(const string& configPath)
{
	if (configPath.find_first_of(illegalConfigurationPathCharacters) != string::npos)
	{
		throw dptf_exception("Configuration Path is invalid");
	}
}