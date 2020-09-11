/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "FileIO.h"
#include <fstream>
using namespace std;

string separatorForward = "/"; // accepted by both Windows and Linux
string separatorBackward = "\\"; // used only by Windows

bool hasEndingPathSeparator(const std::string& folderPath)
{
	if (folderPath.size() == 0)
	{
		return true;
	}
	else
	{
		auto lastCharacter = string(1, folderPath.at(folderPath.size() - 1));
		if ((lastCharacter == separatorForward) || (lastCharacter == separatorBackward))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

std::string getCommonSeparator(const std::string& folderPath)
{
	if (folderPath.find_first_of(separatorForward, 0) != string::npos)
	{
		return separatorForward;
	}
	else if (folderPath.find_first_of(separatorBackward, 0) != string::npos)
	{
		return separatorBackward;
	}
	else
	{
		return separatorForward;
	}
}

bool IFileIO::fileNameHasIllegalChars(const std::string& fileName)
{
	return fileName.find_first_of("*<>:/\\|?", 0) != string::npos;
}

std::string IFileIO::generatePathWithTrailingSeparator(const std::string& folderPath)
{
	if (hasEndingPathSeparator(folderPath))
	{
		return folderPath;
	}
	else
	{
		return folderPath + getCommonSeparator(folderPath);
	}
}

FileIO::FileIO()
{
}

FileIO::~FileIO()
{
}

void FileIO::writeData(const std::string& filePath, const std::string& data)
{
	fstream fp;
	fp.open(filePath, ios::out);
	throwIfFileNotOpened(fp, filePath);
	fp << data;
	fp.close();
}

void FileIO::writeData(const std::string& filePath, const DptfBuffer& data)
{
	fstream fp;
	fp.open(filePath, ios::out);
	throwIfFileNotOpened(fp, filePath);
	fp << data;
	fp.close();
}

void FileIO::throwIfFileNotOpened(const std::fstream& fp, const std::string& filePath)
{
	if (!fp.is_open())
	{
		throw file_open_create_failure(string("Failed to open path \"") + filePath + string("\""));
	}
}
