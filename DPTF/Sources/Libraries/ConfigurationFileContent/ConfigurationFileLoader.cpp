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
#include "nlohmann_json/json.hpp"
#include "ConfigurationFileLoader.h"
using namespace std;
using json = nlohmann::json;

ConfigurationFileLoader::ConfigurationFileLoader(shared_ptr<IFileIo> fileIo, shared_ptr<IDataDecoder> dataDecoder)
	: m_fileIo(move(fileIo))
	, m_dataDecoder(move(dataDecoder))
{

}

shared_ptr<ConfigurationFileContentInterface> ConfigurationFileLoader::readContentFrom(
	const string& configurationFilePath) const
{
	const auto name = IFileIo::getFileNameWithoutExtensionFromPath(configurationFilePath);
	const auto fileData = m_fileIo->read(configurationFilePath);
	const auto dataSegments = decodeDataSegments(fileData);
	return make_shared<ConfigurationFileContent>(name, configurationFilePath, dataSegments);
}

vector<vector<unsigned char>> ConfigurationFileLoader::decodeDataSegments(const vector<unsigned char>& data) const
{
	vector<vector<unsigned char>> result;
	const auto dataSegments = m_dataDecoder->decode(data);
	for (const auto& segment : dataSegments)
	{
		if (json::accept(segment))
		{
			result.push_back(json::to_bson(json::parse(segment)));
		}
	}
	return result;
}