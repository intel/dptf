/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "esif_ccb_file.h"
#include <fstream>
#include <algorithm>
using namespace std;

const string separatorForward = "/"; // accepted by both Windows and Linux
const string separatorBackward = "\\"; // used only by Windows
const string illegalFileNameCharacters = "/\\";
const string illegalFilePathCharacters = "*<>:|?\"";

bool hasEndingPathSeparator(const std::string& folderPath)
{
	if (folderPath.empty())
	{
		return true;
	}
	else
	{
		const auto lastCharacter = string(1, folderPath.at(folderPath.size() - 1));
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

bool IFileIO::fileNameContainsIllegalCharacters(const std::string& fileName)
{
	const auto containsIllegalFileNameCharacter = fileName.find_first_of(illegalFileNameCharacters, 0) != string::npos;
	const auto containsIllegalFilePathCharacter = fileName.find_first_of(illegalFilePathCharacters, 0) != string::npos;
	return (containsIllegalFileNameCharacter || containsIllegalFilePathCharacter);
}

Bool IFileIO::filePathStartsWithIllegalCharacter(const std::string& filePath)
{
	const auto posStartChar = filePath.find_first_of(illegalFilePathCharacters);
	return (posStartChar == 0);
}

Bool IFileIO::filePathContainsColonOutsideOfDriveSection(const std::string& filePath)
{
	// 'C:' is allowed, but : outside of 2nd char is not
	const auto posDrive = filePath.find_first_of(':');
	return ((posDrive != std::string::npos) && (posDrive != 1));
}

Bool IFileIO::filePathContainsIllegalCharacterOutsideOfDriveSection(const std::string& filePath)
{
	const auto posIllegalCharacter = filePath.find_last_of(illegalFilePathCharacters);
	return ((posIllegalCharacter != std::string::npos) && (posIllegalCharacter != 1));
}

Bool IFileIO::filePathEndsWithOtherwiseAllowedCharacter(const std::string& filePath)
{
	return ((filePath.back() == '\\') || (filePath.back() == '/') || (filePath.back() == '.') || (filePath.back() == ' '));
}

Bool IFileIO::filePathContainsDoubleSlashes(const std::string& filePath)
{
	return (filePath.find("\\\\") != std::string::npos) || (filePath.find("//") != std::string::npos);
}

Bool IFileIO::isControlCharacter(const char c)
{
	return iscntrl(c);
}

Bool IFileIO::filePathContainsControlCharacters(const std::string& filePath)
{
	return std::any_of(filePath.cbegin(), filePath.cend(), isControlCharacter);
}

Bool IFileIO::filePathContainsIllegalCharacters(const std::string& filePath)
{
	return
		filePathStartsWithIllegalCharacter(filePath) ||
		filePathContainsColonOutsideOfDriveSection(filePath) || 
		filePathContainsIllegalCharacterOutsideOfDriveSection(filePath) || 
		filePathEndsWithOtherwiseAllowedCharacter(filePath) || 
		filePathContainsDoubleSlashes(filePath) || 
		filePathContainsControlCharacters(filePath);
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

void FileIO::writeData(const std::string& filePath, const std::string& data)
{
	throwIfFilePathHasIllegalCharacters(filePath);
	throwIfFilePathIsSymbolicLink(filePath);
	fstream fp;
	fp.open(filePath, ios::out);
	throwIfFileNotOpened(fp, filePath);
	fp << data;
	fp.close();
}

void FileIO::writeData(const std::string& filePath, const DptfBuffer& data)
{
	throwIfFilePathHasIllegalCharacters(filePath);
	throwIfFilePathIsSymbolicLink(filePath);
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

void FileIO::throwIfFilePathHasIllegalCharacters(const std::string& filePath)
{
	if (filePathContainsIllegalCharacters(filePath))
	{
		throw file_open_create_failure(string("File path contains illegal characters."));
	}
}

void FileIO::throwIfFilePathIsSymbolicLink(const std::string& filePath)
{
	if (esif_ccb_issymlink(filePath.c_str()))
	{
		throw file_open_create_failure(string("File path given is a symbolic link."));
	}
}