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

#include "FileIo.h"
#include "esif_ccb_file.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <stdexcept>

using namespace std;

const string separatorForward = "/"; // accepted by both Windows and Linux
const string separatorBackward = "\\"; // used only by Windows
const string illegalFileNameCharacters = "*<>:|?\"/\\";
const string illegalFilePathCharacters = "*<>:|?\"";

bool IFileIo::hasEndingPathSeparator(const string& folderPath)
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

string IFileIo::getCommonSeparator(const string& folderPath)
{
	if (folderPath.find_first_of(separatorBackward, 0) != string::npos)
	{
		return separatorBackward;
	}
	return separatorForward;
}

bool IFileIo::fileNameContainsIllegalCharacters(const string& fileName)
{
	const auto containsIllegalFileNameCharacter = fileName.find_first_of(illegalFileNameCharacters, 0) != string::npos;
	const auto containsIllegalFilePathCharacter = fileName.find_first_of(illegalFilePathCharacters, 0) != string::npos;
	return (containsIllegalFileNameCharacter || containsIllegalFilePathCharacter);
}

bool IFileIo::filePathStartsWithIllegalCharacter(const string& filePath)
{
	const auto posStartChar = filePath.find_first_of(illegalFilePathCharacters);
	return (posStartChar == 0);
}

bool IFileIo::filePathContainsColonOutsideOfDriveSection(const string& filePath)
{
	// 'C:' is allowed, but : outside of 2nd char is not
	const auto posDrive = filePath.find_first_of(':');
	return ((posDrive != string::npos) && (posDrive != 1));
}

bool IFileIo::filePathContainsIllegalCharacterOutsideOfDriveSection(const string& filePath)
{
	const auto posIllegalCharacter = filePath.find_last_of(illegalFilePathCharacters);
	return ((posIllegalCharacter != string::npos) && (posIllegalCharacter != 1));
}

bool IFileIo::filePathEndsWithOtherwiseAllowedCharacter(const string& filePath)
{
	return (
		(filePath.back() == '\\') || (filePath.back() == '/') || (filePath.back() == '.') || (filePath.back() == ' '));
}

bool IFileIo::filePathContainsDoubleSlashes(const string& filePath)
{
	return (filePath.find("\\\\") != string::npos) || (filePath.find("//") != string::npos);
}

bool IFileIo::isControlCharacter(const char c)
{
	return iscntrl(c);
}

bool IFileIo::filePathContainsControlCharacters(const string& filePath)
{
	return any_of(filePath.cbegin(), filePath.cend(), isControlCharacter);
}

bool IFileIo::filePathContainsIllegalCharacters(const string& filePath)
{
	return filePathStartsWithIllegalCharacter(filePath) || filePathContainsColonOutsideOfDriveSection(filePath)
		   || filePathContainsIllegalCharacterOutsideOfDriveSection(filePath)
		   || filePathEndsWithOtherwiseAllowedCharacter(filePath) || filePathContainsDoubleSlashes(filePath)
		   || filePathContainsControlCharacters(filePath);
}

string IFileIo::generatePathWithTrailingSeparator(const string& folderPath)
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

string IFileIo::getFileNameFromPath(const string& filePath)
{
	const auto pos = filePath.find_last_of(separatorForward + separatorBackward);
	if (pos != string::npos)
	{
		return filePath.substr(pos + 1);
	}
	else
	{
		return filePath;
	}
}

string IFileIo::getFileNameWithoutExtensionFromPath(const string& filePath)
{
	auto fileName = getFileNameFromPath(filePath);
	const auto pos = fileName.find_last_of('.');
	if (pos != string::npos)
	{
		return fileName.substr(0, pos);
	}
	else
	{
		return fileName;
	}
}

vector<unsigned char> FileIo::read(const string& filePath) const
{
	ifstream fileStream{filePath, ios::binary};
	throwIfFileCannotBeOpened(fileStream);
	vector<unsigned char> fileData(istreambuf_iterator<char>(fileStream), {});
	return fileData;
}

void FileIo::write(const string& filePath, const vector<unsigned char>& data) const
{
	ofstream fileStream{filePath, ios::binary};
	throwIfFileCannotBeOpened(fileStream);
	fileStream.write(reinterpret_cast<const char*>(data.data()), static_cast<streamsize>(data.size()));
}

void FileIo::write(const string& filePath, const string& data) const
{
	ofstream fileStream{filePath, ios::binary};
	throwIfFileCannotBeOpened(fileStream);
	fileStream.write(data.data(), static_cast<streamsize>(data.size()));
}

void FileIo::append(const string& filePath, const string& data) const
{
	ofstream fileStream{ filePath, ios::binary | ios::app};
	throwIfFileCannotBeOpened(fileStream);
	fileStream.write(data.c_str(), static_cast<streamsize>(data.size()));
}

bool FileIo::pathExists(const string& path) const
{
	return filesystem::exists(path);
}

void FileIo::createDirectoryPath(const string& filePath) const
{
	filesystem::create_directories(filePath);
}

list<string> FileIo::enumerateFiles(const string& filePath, const string& filter) const
{
	// TODO: C++17 has a library to easily enumerate a directory (#include <filesystem>, directory_iterator(path))
	const auto filePathWithTrailingSeparator = IFileIo::generatePathWithTrailingSeparator(filePath);
	const auto path = const_cast<esif_string>(filePathWithTrailingSeparator.c_str());
	const auto pattern = const_cast<esif_string>(filter.c_str());
	esif_ccb_file file{};
	esif_ccb_memset(file.filename, 0, sizeof(file.filename));
	const esif_ccb_file_enum_t fileHandle = esif_ccb_file_enum_first(path, pattern, &file);
	if (fileHandle == nullptr)
	{
		return {};
	}
	else
	{
		list<string> files;
		files.emplace_back(filePathWithTrailingSeparator + string(file.filename));
		const esif_ccb_file* rc = esif_ccb_file_enum_next(fileHandle, pattern, &file);
		while (rc != nullptr)
		{
			files.emplace_back(filePathWithTrailingSeparator + string(file.filename));
			rc = esif_ccb_file_enum_next(fileHandle, pattern, &file);
		}
		esif_ccb_file_enum_close(fileHandle);
		return files;
	}
}

unsigned int FileIo::getFileLength(ifstream& fileStream)
{
	if (fileStream)
	{
		fileStream.seekg(0, ifstream::end);
		const auto length = static_cast<unsigned int>(fileStream.tellg());
		fileStream.seekg(0, ifstream::beg);
		return length;
	}
	return 0;
}

void FileIo::throwIfFileCannotBeOpened(const ios& fileStream)
{
	if (!fileStream)
	{
		throw runtime_error("File cannot be opened");
	}
}

void FileIo::throwIfFilePathHasIllegalCharacters(const string& filePath)
{
	if (filePathContainsIllegalCharacters(filePath))
	{
		throw runtime_error("File path contains illegal characters.");
	}
}

void FileIo::throwIfFilePathIsSymbolicLink(const string& filePath)
{
	if (esif_ccb_issymlink(filePath.c_str()))
	{
		throw runtime_error("File path given is a symbolic link.");
	}
}