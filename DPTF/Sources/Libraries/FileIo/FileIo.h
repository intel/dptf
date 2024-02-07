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
#include <vector>
#include <string>

class IFileIo
{
public:
	IFileIo() = default;
	IFileIo(const IFileIo& other) = default;
	IFileIo(IFileIo&& other) noexcept = default;
	IFileIo& operator=(const IFileIo& other) = default;
	IFileIo& operator=(IFileIo&& other) noexcept = default;
	virtual ~IFileIo() = default;

	virtual std::vector<unsigned char> read(const std::string& filePath) const = 0;
	virtual void write(const std::string& filePath, const std::vector<unsigned char>& data) const = 0;
	virtual void write(const std::string& filePath, const std::string& data) const = 0;
	virtual void append(const std::string& filePath, const std::string& data) const = 0;
	virtual void createDirectoryPath(const std::string& filePath) const = 0;
	virtual bool pathExists(const std::string& path) const = 0;
	virtual std::list<std::string> enumerateFiles(const std::string& filePath, const std::string& filter) const = 0;

	static bool fileNameContainsIllegalCharacters(const std::string& fileName);
	static bool filePathContainsIllegalCharacters(const std::string& filePath);
	static std::string generatePathWithTrailingSeparator(const std::string& folderPath);
	static std::string getFileNameFromPath(const std::string& filePath);
	static std::string getFileNameWithoutExtensionFromPath(const std::string& filePath);

private:
	static bool filePathStartsWithIllegalCharacter(const std::string& filePath);
	static bool filePathContainsColonOutsideOfDriveSection(const std::string& filePath);
	static bool filePathContainsIllegalCharacterOutsideOfDriveSection(const std::string& filePath);
	static bool filePathEndsWithOtherwiseAllowedCharacter(const std::string& filePath);
	static bool filePathContainsDoubleSlashes(const std::string& filePath);
	static bool isControlCharacter(char c);
	static bool filePathContainsControlCharacters(const std::string& filePath);
	static std::string getCommonSeparator(const std::string& folderPath);
	static bool hasEndingPathSeparator(const std::string& folderPath);
};

class FileIo : public IFileIo
{
public:
	
	std::vector<unsigned char> read(const std::string& filePath) const override;
	void write(const std::string& filePath, const std::vector<unsigned char>& data) const override;
	void write(const std::string& filePath, const std::string& data) const override;
	void append(const std::string& filePath, const std::string& data) const override;
	bool pathExists(const std::string& path) const override;
	void createDirectoryPath(const std::string& filePath) const override;
	std::list<std::string> enumerateFiles(const std::string& filePath, const std::string& filter) const override;

private:
	static unsigned int getFileLength(std::ifstream& fileStream);
	static void throwIfFileCannotBeOpened(const std::ios& fileStream);
	static void throwIfFilePathHasIllegalCharacters(const std::string& filePath);
	static void throwIfFilePathIsSymbolicLink(const std::string& filePath);
};

