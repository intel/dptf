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
#include "ConfigurationFileContent.h"

#include <utility>
#include "nlohmann_json/json.hpp"
using namespace std;
using json = nlohmann::json;

#define CONFIGDB_ENTRY_DELIMITER ","

constexpr auto INDENT_WIDTH = 4;
constexpr auto CURRENT_SERIALIZED_VERSION = 1;

ConfigurationFileContent::ConfigurationFileContent(
	string name,
	string sourceFilePath,
	vector<vector<unsigned char>> dataSegments)
	: m_dataSegments(move(dataSegments))
{
	m_metaData.name = move(name);
	m_metaData.sourceFilePath = move(sourceFilePath);
	m_metaData.dataLengthInBytes = calculateDataLengthInBytes();
}

ConfigurationFileContent::ConfigurationFileContent(
	string name,
	string sourceFilePath,
	vector<unsigned char> dataSegment)
	: m_dataSegments{move(dataSegment)}
{
	m_metaData.name = move(name);
	m_metaData.sourceFilePath = move(sourceFilePath);
	m_metaData.dataLengthInBytes = calculateDataLengthInBytes();
}

ConfigurationFileContent ConfigurationFileContent::deserialize(const vector<unsigned char>& serializedContent)
{
	size_t location = 0;
	const auto version = extractNumberAs4bytes(serializedContent, location);
	if (version == CURRENT_SERIALIZED_VERSION)
	{
		location += 4;
		const auto name = extractString(serializedContent, location);
		location += getSerializedStringLength(serializedContent, location);
		const auto path = extractString(serializedContent, location);
		location += getSerializedStringLength(serializedContent, location);
		const auto dataSegments = extractDataSegments(serializedContent, location);
		return {name, path, dataSegments};
	}
	else
	{
		throw ConfigurationFileContentError(
			"Serialized configuration file content version "s + to_string(version) + " is not supported"s);
	}
}

bool ConfigurationFileContent::operator==(const ConfigurationFileContent& other) const
{
	return (m_metaData.name == other.m_metaData.name) && (m_metaData.sourceFilePath == other.m_metaData.sourceFilePath)
		   && (m_dataSegments == other.m_dataSegments);
}

size_t ConfigurationFileContent::extractNumberAs4bytes(const vector<unsigned char>& content, size_t location)
{
	throwIfNotEnoughDataToRead(content, location + 3);

	unsigned result = 0;
	result += content[location + 0] << 0;
	result += content[location + 1] << 8;
	result += content[location + 2] << 8;
	result += content[location + 3] << 8;
	return result;
}

string ConfigurationFileContent::extractString(const vector<unsigned char>& content, size_t location)
{
	const auto stringLength = extractNumberAs4bytes(content, location);
	throwIfNotEnoughDataToRead(content, 4 + location + stringLength);
	const auto locationBegin = content.begin() + location + 4;
	const auto locationEnd = locationBegin + stringLength;
	return string{locationBegin, locationEnd};
}

size_t ConfigurationFileContent::getSerializedStringLength(const vector<unsigned char>& content, size_t location)
{
	const auto stringLength = extractNumberAs4bytes(content, location);
	throwIfNotEnoughDataToRead(content, 4 + location + stringLength);
	return 4 + stringLength;
}

vector<vector<unsigned char>> ConfigurationFileContent::extractDataSegments(const vector<unsigned char>& content, size_t location)
{
	vector<vector<unsigned char>> result;
	size_t offset = 0;
	const auto numberOfSegments = extractNumberAs4bytes(content, location);
	offset += 4;
	for (unsigned i = 0; i < numberOfSegments; ++i)
	{
		const auto segmentLength = extractNumberAs4bytes(content, location + offset);
		offset += 4;
		const auto locationBegin = content.begin() + location + offset;
		const auto locationEnd = locationBegin + segmentLength;
		const vector<unsigned char> segment{locationBegin, locationEnd};
		offset += segmentLength;
		result.push_back(segment);
	}

	return result;
}

const vector<vector<unsigned char>>& ConfigurationFileContent::dataSegments() const
{
	return m_dataSegments;
}

string ConfigurationFileContent::toString() const
{
	string result;
	for (const auto& segment : m_dataSegments)
	{
		auto data = json::from_bson(segment);
		const auto formattedJsonText = data.dump(INDENT_WIDTH);
		result.append(formattedJsonText);
	}
	return result;
}

string ConfigurationFileContent::toDatabaseString() const
{
	string result;
	for (const auto& segment : m_dataSegments)
	{
		auto data = json::from_bson(segment);
		auto flattenedData = data.flatten();
		const auto formattedJsonText = flattenedData.dump(INDENT_WIDTH);
		result.append(formattedJsonText);
	}
	return result;
}

vector<unsigned char> ConfigurationFileContent::serialize() const
{
	vector<unsigned char> result;

	const auto serializedVersion = createSerializedNumber(CURRENT_SERIALIZED_VERSION);
	result.insert(result.end(), serializedVersion.begin(), serializedVersion.end());
	const auto serializedName = createSerializedString(m_metaData.name);
	result.insert(result.end(), serializedName.begin(), serializedName.end());
	const auto serializedPath = createSerializedString(m_metaData.sourceFilePath);
	result.insert(result.end(), serializedPath.begin(), serializedPath.end());
	const auto serializedSegments = createSerializedSegments(m_dataSegments);
	result.insert(result.end(), serializedSegments.begin(), serializedSegments.end());

	return result;
}

const ConfigurationFileContentMetaData& ConfigurationFileContent::metaData() const
{
	return m_metaData;
}

vector<unsigned char> ConfigurationFileContent::createSerializedNumber(unsigned int value)
{
	vector<unsigned char> serializedData{};
	const auto number = reinterpret_cast<unsigned char*>(&value);
	serializedData.insert(serializedData.end(), number, number + sizeof(value));
	return serializedData;
}

size_t ConfigurationFileContent::calculateDataLengthInBytes() const
{
	size_t totalSize = 0;
	for (const auto& segment : m_dataSegments)
	{
		totalSize += segment.size();
	}
	return totalSize;
}

vector<unsigned char> ConfigurationFileContent::createSerializedString(const string& value)
{
	vector<unsigned char> serializedData{};
	auto stringLength = value.size();
	const auto stringLengthBytes = reinterpret_cast<unsigned char*>(&stringLength);
	serializedData.insert(serializedData.end(), stringLengthBytes, stringLengthBytes + sizeof(unsigned int));
	serializedData.insert(serializedData.end(), value.begin(), value.end());
	return serializedData;
}

vector<unsigned char> ConfigurationFileContent::createSerializedSegments(const vector<vector<unsigned char>>& segments)
{
	vector<unsigned char> serializedData{};
	auto numSegments = segments.size();
	const auto numSegmentsBytes = reinterpret_cast<unsigned char*>(&numSegments);
	serializedData.insert(serializedData.end(), numSegmentsBytes, numSegmentsBytes + sizeof(unsigned int));

	for (const auto& segment : segments)
	{
		auto segmentLength = segment.size();
		const auto segmentLengthBytes = reinterpret_cast<unsigned char*>(&segmentLength);
		serializedData.insert(serializedData.end(), segmentLengthBytes, segmentLengthBytes + sizeof(unsigned int));
		serializedData.insert(serializedData.end(), segment.begin(), segment.end());
	}

	return serializedData;
}

void ConfigurationFileContent::throwIfNotEnoughDataToRead(const vector<unsigned char>& content, size_t lastReadPosition)
{
	if (content.size() <= lastReadPosition)
	{
		throw ConfigurationFileContentError("Data length too short"s);
	}
}