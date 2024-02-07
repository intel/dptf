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

#include "DttConfiguration.h"
using namespace std;

#define CONFIGDB_ENTRY_DELIMITER ","
constexpr auto INDENT_WIDTH = 4;

DttConfiguration::DttConfiguration(const DttConfigurationSegment& configSegment)
{
	m_segments.emplace_back(configSegment);
}

DttConfiguration::DttConfiguration(const std::vector<DttConfigurationSegment>& segments)
{
	m_segments = segments;
}

DttConfiguration::DttConfiguration(const ConfigurationFileContent& configurationFileContent)
{
	for (const auto& segment : configurationFileContent.dataSegments())
	{
		const auto configSegment = DttConfigurationSegment::createFromBson(segment);
		m_segments.emplace_back(configSegment);
	}
}

DttConfiguration::DttConfiguration(const std::shared_ptr<ConfigurationFileContentInterface>& configurationFileContent)
{
	for (const auto& segment : configurationFileContent->dataSegments())
	{
		const auto configSegment = DttConfigurationSegment::createFromBson(segment);
		m_segments.emplace_back(configSegment);
	}
}

vector<DttConfigurationSegment> DttConfiguration::getSegmentsWithValue(const string& value) const
{
	vector<DttConfigurationSegment> segments;
	for (const auto& segment : m_segments)
	{
		if (segment.hasValue(value))
		{
			segments.emplace_back(segment);
		}
	}
	return segments;
}

DttConfigurationSegment DttConfiguration::getDefaultSegment() const
{
	if (m_segments.empty())
	{
		throw dptf_exception("Default policy configuration cannot be selected since no segments exist"s);
	}
	return m_segments.front();
}

size_t DttConfiguration::numberOfSegments() const
{
	return m_segments.size();
}

const DttConfigurationSegment& DttConfiguration::getSegment(size_t segmentNumber) const
{
	if (segmentNumber < numberOfSegments())
	{
		return m_segments[segmentNumber];
	}
	throw dptf_exception("Invalid segment number"s);
}

bool DttConfiguration::empty() const
{
	return m_segments.empty();
}

DttConfiguration DttConfiguration::operator+(const DttConfiguration& other) const
{
	auto segmentsCopy = m_segments;
	segmentsCopy.insert(segmentsCopy.end(), other.m_segments.begin(), other.m_segments.end());
	return DttConfiguration(segmentsCopy);
}

bool DttConfiguration::operator<(const DttConfiguration& other) const
{
	return m_segments < other.m_segments;
}

vector<DttConfigurationSegment> DttConfiguration::getSegmentsWithAllProperties(
	const set<DttConfigurationProperty>& properties) const
{
	vector<DttConfigurationSegment> segments;
	for (const auto& segment : m_segments)
	{
		if (segment.hasProperties(properties))
		{
			segments.emplace_back(segment);
		}
	}
	return segments;
}

vector<DttConfigurationSegment> DttConfiguration::getSegmentsWithAllPropertiesIncludingEmpty(
	const set<DttConfigurationProperty>& properties) const
{
	vector<DttConfigurationSegment> segments;
	for (const auto& segment : m_segments)
	{
		if (segment.hasPropertiesIncludingEmptyValue(properties))
		{
			segments.emplace_back(segment);
		}
	}
	return segments;
}

vector<DttConfigurationSegment> DttConfiguration::getSegmentsWithEnvironmentProfile(
	const EnvironmentProfile& environmentProfile) const
{
	auto segments = getSegmentsWithExactEnvironmentProfile(environmentProfile);
	auto segmentsWithCpuId = getSegmentsWithMatchedCpuIdInEpoSegments(environmentProfile.cpuIdWithoutStepping);
	auto segmentsWithEmpties = getSegmentsWithEnvironmentProfileIncludingEmpty(environmentProfile);
	auto segmentsWithDefaultLabel = getSegmentsWithEnvironmentProfileAndLabel(environmentProfile, "Default");

	addUniqueSegments(segments, segmentsWithCpuId);
	addUniqueSegments(segments, segmentsWithEmpties);
	combineWithDefaultSegments(segments, segmentsWithDefaultLabel);

	return segments;
}

vector<DttConfigurationSegment> DttConfiguration::getSegmentsWithExactEnvironmentProfile(
	const EnvironmentProfile& environmentProfile) const
{
	vector<DttConfigurationSegment> segments;
	for (const auto& segment : m_segments)
	{
		if (segment.matchesEnvironmentProfile(environmentProfile))
		{
			segments.emplace_back(segment);
		}
	}
	return segments;
}

vector<DttConfigurationSegment> DttConfiguration::getSegmentsWithEnvironmentProfileIncludingEmpty(
	const EnvironmentProfile& environmentProfile) const
{
	vector<DttConfigurationSegment> segments;
	for (const auto& segment : m_segments)
	{
		if (segment.matchesEnvironmentProfileIncludingEmptyValue(environmentProfile))
		{
			segments.emplace_back(segment);
		}
	}
	return segments;
}

vector<DttConfigurationSegment> DttConfiguration::getSegmentsWithMatchedCpuIdInEpoSegments(
	const string& cpuId) const
{
	vector<DttConfigurationSegment> segments;
	for (const auto& segment : m_segments)
	{
		if (segment.matchesCpuIdInEpoSegment(cpuId))
		{
			segments.emplace_back(segment);
		}
	}
	return segments;
}

vector<DttConfigurationSegment> DttConfiguration::getSegmentsWithEnvironmentProfileAndLabel(
	const EnvironmentProfile& environmentProfile,
	const string& label) const
{
	vector<DttConfigurationSegment> segments;
	for (const auto& segment : m_segments)
	{
		if (segment.hasLabel(label)
			&& segment.matchesEnvironmentProfileIncludingEmptyValue(environmentProfile))
		{
			segments.emplace_back(segment);
		}
	}
	return segments;
}

string DttConfiguration::toActiveConfigurationString(
	const EnvironmentProfile& environmentProfile,
	const regex& regularExp) const
{
	string result;

	const auto matchedDataSegments = getSegmentsWithEnvironmentProfile(environmentProfile);

	if (!matchedDataSegments.empty())
	{
		const auto matchedDataSegment = matchedDataSegments.front();
		auto formattedJsonText = matchedDataSegment.toKeyValueString();
		string regexMatchedEntry;

		while (formattedJsonText.find(CONFIGDB_ENTRY_DELIMITER) != string::npos)
		{
			const auto delimiterIndex = formattedJsonText.find(CONFIGDB_ENTRY_DELIMITER);
			const auto keyValueEntry = formattedJsonText.substr(0, delimiterIndex);
			if (regex_search(keyValueEntry, regularExp))
			{
				regexMatchedEntry.append(keyValueEntry + CONFIGDB_ENTRY_DELIMITER);
			}
			formattedJsonText.erase(0, delimiterIndex + 1);
		}
		result.append(regexMatchedEntry);
	}
	return result;
}

void DttConfiguration::addUniqueSegments(
	vector<DttConfigurationSegment>& existingSegments,
	vector<DttConfigurationSegment>& segmentsToAdd) const
{
	for (const auto& segment : segmentsToAdd)
	{
		if (find(existingSegments.begin(), existingSegments.end(), segment) == existingSegments.end())
		{
			existingSegments.emplace_back(segment);
		}
	}
}

void DttConfiguration::combineWithDefaultSegments(
	vector<DttConfigurationSegment>& existingSegments,
	vector<DttConfigurationSegment>& defaultSegments) const
{
	if (!defaultSegments.empty())
	{
		DttConfigurationSegment defaultSuperSet;
		for (const auto& segment : defaultSegments)
		{
			existingSegments.erase(remove(existingSegments.begin(), existingSegments.end(), segment));
			defaultSuperSet = defaultSuperSet + segment;
		}

		for (auto& segment : existingSegments)
		{
			segment = defaultSuperSet + segment;
		}

		existingSegments.emplace_back(defaultSuperSet);
	}
}