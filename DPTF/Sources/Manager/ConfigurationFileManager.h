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
#include <set>
#include <list>
#include <map>
#include <string>
#include "Dptf.h"
#include "ConfigurationFileContent.h"
#include "ConfigurationFileLoaderFactory.h"
#include "FileIo.h"
#include "MessageLogger.h"
#include "RequestHandlerInterface.h"

class dptf_export ConfigurationFileManagerInterface : public RequestHandlerInterface
{
public:
	virtual ~ConfigurationFileManagerInterface() = default;

	virtual void loadFiles() = 0;
	virtual std::set<std::string> filesLoaded() const = 0;
	virtual std::set<std::string> getContentNames() const = 0;
	virtual std::shared_ptr<ConfigurationFileContentInterface> getContent(const std::string& name) const = 0;
	virtual bool contentExists(const std::string& name) const = 0;
};

class dptf_export ConfigurationFileManager : public ConfigurationFileManagerInterface
{
public:
	ConfigurationFileManager(
		std::shared_ptr<MessageLogger> messageLogger,
		std::shared_ptr<IFileIo> fileIo,
		std::shared_ptr<ConfigurationFileLoaderFactoryInterface> configFactory, 
		const std::list<std::string>& configFileSearchPaths);
	virtual ~ConfigurationFileManager(void);

	static std::shared_ptr<ConfigurationFileManager> makeDefault(
		std::shared_ptr<MessageLogger> logger,
		std::shared_ptr<IFileIo> fileIo,
		const std::list<std::string>& configFileSearchPaths);

	// ConfigurationFileManagerInterface
	void loadFiles() override;
	std::set<std::string> filesLoaded() const override;
	std::set<std::string> getContentNames() const override;
	std::shared_ptr<ConfigurationFileContentInterface> getContent(const std::string& name) const override;
	bool contentExists(const std::string& name) const override;

	// RequestHandlerInterface
	DptfRequestResult processRequest(const PolicyRequest& policyRequest) override;
	Bool canProcessRequest(const PolicyRequest& policyRequest) override;

private:
	std::shared_ptr<MessageLogger> m_messageLogger;
	std::shared_ptr<IFileIo> m_fileIo;
	std::shared_ptr<ConfigurationFileLoaderFactoryInterface> m_configFactory;
	std::list<std::string> m_configFileSearchPaths;
	std::set<std::string> m_configFiles;
	std::map<std::string, std::shared_ptr<ConfigurationFileContentInterface>> m_configurations;

	std::map<std::string, std::string> enumerateConfigFiles() const;
	static std::set<std::string> buildConfigFilePathList(const std::map<std::string, std::string>& prioritizedFilePaths);
	std::map<std::string, std::shared_ptr<ConfigurationFileContentInterface>> createConfigurations(
		const std::set<std::string>& configFilePaths);
	
};
