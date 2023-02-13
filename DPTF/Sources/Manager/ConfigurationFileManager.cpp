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

#include "ConfigurationFileManager.h"
#include "LzmaDataCompressor.h"
#include "MessageLogger.h"
#include "StringConverter.h"

using namespace std;

const string ConfigFileExtension("*.config"s);

ConfigurationFileManager::ConfigurationFileManager(
	shared_ptr<MessageLogger> messageLogger,
	shared_ptr<IFileIo> fileIo,
	shared_ptr<ConfigurationFileLoaderFactoryInterface> configFactory,
	const list<string>& configFileSearchPaths)
	: m_messageLogger(messageLogger)
	, m_fileIo(fileIo)
	, m_configFactory(configFactory)
	, m_configFileSearchPaths(configFileSearchPaths)
	, m_configFiles{}
	, m_configurations{}
{
}

ConfigurationFileManager::~ConfigurationFileManager()
{
}

shared_ptr<ConfigurationFileManager> ConfigurationFileManager::makeDefault(
	shared_ptr<MessageLogger> logger,
	shared_ptr<IFileIo> fileIo,
	const list<string>& configFileSearchPaths)
{
	auto dataCompressor = make_shared<LzmaDataCompressor>();
	auto dataDecoder = make_shared<DataDecoder>(dataCompressor);
	auto configFactory = make_shared<ConfigurationFileLoaderFactory>(fileIo, dataDecoder);
	return make_shared<ConfigurationFileManager>(logger, fileIo, configFactory, configFileSearchPaths);
}

set<string> ConfigurationFileManager::filesLoaded() const
{
	return m_configFiles;
}

set<string> ConfigurationFileManager::getContentNames() const
{
	set<string> keys;
	for (const auto& pair : m_configurations)
	{
		keys.insert(pair.first);
	}
	return keys;
}

shared_ptr<ConfigurationFileContentInterface> ConfigurationFileManager::getContent(const string& name) const
{
	const auto cf = m_configurations.find(StringConverter::toLower(name));
	if (cf == m_configurations.end())
	{
		throw out_of_range("No Configuration exists with name "s + name);
	}
	else
	{
		return cf->second;
	}
}

bool ConfigurationFileManager::contentExists(const string& name) const
{
	return (m_configurations.find(StringConverter::toLower(name)) != m_configurations.end());
}

void ConfigurationFileManager::loadFiles()
{
	const auto allConfigFiles = enumerateConfigFiles();
	m_configFiles = buildConfigFilePathList(allConfigFiles);
	m_configurations = createConfigurations(m_configFiles);
}

DptfRequestResult ConfigurationFileManager::processRequest(const PolicyRequest& policyRequest)
{
	if (canProcessRequest(policyRequest))
	{
		const auto& data = policyRequest.getRequest().getData();
		const string contentName{(char*)data.get(), data.size()};
		if (contentExists(contentName))
		{
			const auto content = getContent(contentName);
			const auto serializedContent = content->serialize();
			DptfRequestResult result{true, "Successfully processed request"s, policyRequest.getRequest()};
			result.setData(DptfBuffer::fromExistingByteVector(serializedContent));
			return result;
		}
		else
		{
			return {false, "No content exists with name "s + contentName, policyRequest.getRequest()};
		}
	}
	else
	{
		const auto requestTypeName = DptfRequestType::ToString(policyRequest.getRequest().getRequestType());
		return {false, "Cannot process request type "s + requestTypeName, policyRequest.getRequest()};
	}
}

Bool ConfigurationFileManager::canProcessRequest(const PolicyRequest& policyRequest)
{
	return policyRequest.getRequest().getRequestType() == DptfRequestType::DataGetConfigurationFileContent;
}

map<string, string> ConfigurationFileManager::enumerateConfigFiles() const
{
	// Enumerate config files in each search path in order.
	// Any file with the same name found in previous search path replaces the old one.
	map<string, string> prioritizedFilePaths;
	for (const auto& searchPath : m_configFileSearchPaths)
	{
		const auto files = m_fileIo->enumerateFiles(searchPath, ConfigFileExtension);
		for (const auto& file : files)
		{
			const auto fileName = IFileIo::getFileNameFromPath(file);
			prioritizedFilePaths[fileName] = file;
		}
	}
	return prioritizedFilePaths;
}

set<string> ConfigurationFileManager::buildConfigFilePathList(
	const map<string, string>& prioritizedFilePaths)
{
	set<string> configFiles;
	for (const auto& item : prioritizedFilePaths)
	{
		configFiles.insert(item.second);
	}
	return configFiles;
}

map<string, shared_ptr<ConfigurationFileContentInterface>> ConfigurationFileManager::createConfigurations(
	const set<string>& configFilePaths)
{
	map<string, shared_ptr<ConfigurationFileContentInterface>> configurations;
	const auto loader = m_configFactory->make();
	for (const auto& configFilePath : configFilePaths)
	{
		try
		{
			const auto content = loader->readContentFrom(configFilePath);
			configurations.insert(pair<string, shared_ptr<ConfigurationFileContentInterface>>(StringConverter::toLower(content->metaData().name), content));
		}
		catch (const exception& e)
		{
			LOG_MESSAGE_FRAMEWORK_WARNING(m_messageLogger, 
				{ return "Failed to create configuration space: "s + string(e.what()); })
		}
	}
	return configurations;
}
