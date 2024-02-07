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
#include <stdexcept>
#include <string>
#include <vector>

class ConfigurationFileContentMetaData
{
public:
	ConfigurationFileContentMetaData() = default;
	ConfigurationFileContentMetaData(std::string name, std::string sourceFilePath, size_t dataLengthInBytes)
		: name(move(name))
		, sourceFilePath(move(sourceFilePath))
		, dataLengthInBytes(dataLengthInBytes) {}
	std::string name;
	std::string sourceFilePath;
	size_t dataLengthInBytes{0};
};

class ConfigurationFileContentError : public std::logic_error
{
public:
	using logic_error::logic_error;
};

class ConfigurationFileContentInterface
{
public:
	ConfigurationFileContentInterface() = default;
	ConfigurationFileContentInterface(const ConfigurationFileContentInterface& other) = default;
	ConfigurationFileContentInterface(ConfigurationFileContentInterface&& other) noexcept = default;
	ConfigurationFileContentInterface& operator=(const ConfigurationFileContentInterface& other) = default;
	ConfigurationFileContentInterface& operator=(ConfigurationFileContentInterface&& other) noexcept = default;
	virtual ~ConfigurationFileContentInterface() = default;
	virtual const ConfigurationFileContentMetaData& metaData() const = 0;
	virtual const std::vector<std::vector<unsigned char>>& dataSegments() const = 0;
	virtual std::string toString() const = 0;
	virtual std::string toDatabaseString() const = 0;
	virtual std::vector<unsigned char> serialize() const = 0;
};

class ConfigurationFileContent : public ConfigurationFileContentInterface
{
public:
	ConfigurationFileContent() = default;
	ConfigurationFileContent(std::string name, std::string sourceFilePath, std::vector<std::vector<unsigned char>> dataSegments);
	ConfigurationFileContent(
		std::string name,
		std::string sourceFilePath,
		std::vector<unsigned char> dataSegment);
	ConfigurationFileContent(const ConfigurationFileContent& other) = default;
	ConfigurationFileContent(ConfigurationFileContent&& other) noexcept = default;
	ConfigurationFileContent& operator=(const ConfigurationFileContent& other) = default;
	ConfigurationFileContent& operator=(ConfigurationFileContent&& other) noexcept = default;
	static ConfigurationFileContent deserialize(const std::vector<unsigned char>& serializedContent);
	~ConfigurationFileContent() override = default;

	bool operator==(const ConfigurationFileContent& other) const;

	const ConfigurationFileContentMetaData& metaData() const override;
	const std::vector<std::vector<unsigned char>>& dataSegments() const override;
	std::string toString() const override;
	std::string toDatabaseString() const override;
	std::vector<unsigned char> serialize() const override;

private:
	ConfigurationFileContentMetaData m_metaData;
	std::vector<std::vector<unsigned char>> m_dataSegments;

	size_t calculateDataLengthInBytes() const;
	static std::vector<unsigned char> createSerializedNumber(unsigned value);
	static std::vector<unsigned char> createSerializedString(const std::string& value);
	static std::vector<unsigned char> createSerializedSegments(const std::vector<std::vector<unsigned char>>& segments);
	static size_t extractNumberAs4bytes(const std::vector<unsigned char>& content, size_t location);
	static std::string extractString(const std::vector<unsigned char>& content, size_t location);
	static size_t getSerializedStringLength(const std::vector<unsigned char>& content, size_t location);
	static std::vector<std::vector<unsigned char>> extractDataSegments(const std::vector<unsigned char>& content, size_t location);
	static void throwIfNotEnoughDataToRead(const std::vector<unsigned char>& content, size_t lastReadPosition);
};