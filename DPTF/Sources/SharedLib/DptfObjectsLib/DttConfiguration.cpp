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

#include "DttConfiguration.h"
using namespace std;

DttConfiguration::DttConfiguration(const DttConfigurationSegment& configSegment)
{
	m_segments.emplace_back(configSegment);
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
	else
	{
		return m_segments[0];
	}
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

std::vector<DttConfigurationSegment> DttConfiguration::getSegmentsWithEnvironmentProfile(
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
