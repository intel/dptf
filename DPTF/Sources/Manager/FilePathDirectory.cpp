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

#include "FilePathDirectory.h"
#include "ConfigurationUpdaterFactory.h"
#include "EsifLibrary.h"
#include "FileIo.h"
#include <string>

using namespace std;

FilePathDirectory::FilePathDirectory(const string& logPath)
	: m_paths{}
{
	m_paths[Path::InstallFolder] = getInstallPath();
	m_paths[Path::ConfigurationInstallFolder] = getConfigurationInstallPath();
	m_paths[Path::ConfigurationOverrideFolder] = getConfigurationOverridePath();
	m_paths[Path::ConfigurationUpdaterFolder] = getConfigurationUpdaterPath();
	m_paths[Path::IpfLogFolder] = IFileIo::generatePathWithTrailingSeparator(logPath);
	m_paths[Path::DttLogFolder] = getDttLogFolderPath();
}

string FilePathDirectory::getPath(Path resource) const
{
	return m_paths.at(resource);
}

list<string> FilePathDirectory::getConfigurationFilePaths() const
{
	return {
		m_paths.at(Path::ConfigurationInstallFolder),
		m_paths.at(Path::ConfigurationUpdaterFolder),
		m_paths.at(Path::ConfigurationOverrideFolder)
	};
}

string FilePathDirectory::getInstallPath()
{
	EsifLibrary dptfLib;
	dptfLib.load();
	string path = IFileIo::generatePathWithTrailingSeparator(dptfLib.getLibDirectory());
	dptfLib.unload();
	return path;
}

string FilePathDirectory::getConfigurationInstallPath()
{
	#if defined(ESIF_ATTR_OS_CHROME) // Only for Chrome suggestion from Google to move to /etc/dptf/configuration
	return R"(/etc/dptf/configuration)";
	#else // Same path for Windows and Linux ( /usr/share/dptf/ufx64/configuration)
	const auto installPath = getInstallPath();
	return IFileIo::generatePathWithTrailingSeparator(installPath + "configuration"s);
	#endif
}

string FilePathDirectory::getConfigurationOverridePath()
{
	return R"(/usr/share/dptf/configuration)";
}

string FilePathDirectory::getConfigurationUpdaterPath()
{
	try
	{
		auto configUpdater = ConfigurationUpdaterFactory::make();
		std::string configurationUpdaterInstallPath =
			IFileIo::generatePathWithTrailingSeparator(configUpdater->getConfigurationUpdaterPath());
		return IFileIo::generatePathWithTrailingSeparator(configurationUpdaterInstallPath + "configuration"s);
	}
	catch (const exception&)
	{
		return "";
	}
}

string FilePathDirectory::getDttLogFolderPath()
{
		return R"(/var/log/dtt/)";
}