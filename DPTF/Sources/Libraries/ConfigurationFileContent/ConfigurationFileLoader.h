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

#include <memory>
#include <string>
#include <vector>
#include "ConfigurationFileContent.h"
#include "FileIo.h"
#include "DataDecoder.h"


class ConfigurationFileLoaderInterface
{
public:
	ConfigurationFileLoaderInterface() = default;
	ConfigurationFileLoaderInterface(const ConfigurationFileLoaderInterface& other) = default;
	ConfigurationFileLoaderInterface(ConfigurationFileLoaderInterface&& other) noexcept = default;
	ConfigurationFileLoaderInterface& operator=(const ConfigurationFileLoaderInterface& other) = default;
	ConfigurationFileLoaderInterface& operator=(ConfigurationFileLoaderInterface&& other) noexcept = default;
	virtual ~ConfigurationFileLoaderInterface() = default;
	virtual std::shared_ptr<ConfigurationFileContentInterface> readContentFrom(
		const std::string& configurationFilePath) const = 0;
};

class ConfigurationFileLoader : public ConfigurationFileLoaderInterface
{
public:
	ConfigurationFileLoader(std::shared_ptr<IFileIo> fileIo, std::shared_ptr<IDataDecoder> dataDecoder);	
	
	std::shared_ptr<ConfigurationFileContentInterface> readContentFrom(
		const std::string& configurationFilePath) const override;

private:
	std::shared_ptr<IFileIo> m_fileIo;
	std::shared_ptr<IDataDecoder> m_dataDecoder;

	std::vector<std::vector<unsigned char>> decodeDataSegments(const std::vector<unsigned char>& data) const;
};
