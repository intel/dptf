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
#include <list>
#include <map>
#include <string>

class FilePathDirectory
{
public:
	enum class Path
	{
		InstallFolder,
		IpfLogFolder,
		ConfigurationInstallFolder,
		ConfigurationOverrideFolder,
		ConfigurationUpdaterFolder,
		DttLogFolder
	};

	FilePathDirectory(const std::string& logPath);
	std::string getPath(Path resource) const;
	std::list<std::string> getConfigurationFilePaths() const;

private:

	std::map<Path, std::string> m_paths;
	static std::string getInstallPath();
	static std::string getConfigurationInstallPath();
	static std::string getConfigurationOverridePath();
	static std::string getConfigurationUpdaterPath();
	static std::string getDttLogFolderPath();
};
