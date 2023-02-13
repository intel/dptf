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

#pragma once
#include "Dptf.h"
#include "ConfigurationFileContent.h"
#include "DttConfigurationProperty.h"
#include "DttConfigurationSegment.h"
#include "EnvironmentProfile.h"

class dptf_export DttConfiguration
{
public:

	DttConfiguration() = default;
	explicit DttConfiguration(const DttConfigurationSegment& configSegment);
	explicit DttConfiguration(const ConfigurationFileContent& configurationFileContent);
	explicit DttConfiguration(const std::shared_ptr<ConfigurationFileContentInterface>& configurationFileContent);

	std::vector<DttConfigurationSegment> getSegmentsWithValue(const std::string& value) const;
	std::vector<DttConfigurationSegment> getSegmentsWithAllProperties(
		const std::set<DttConfigurationProperty>& properties) const;
	std::vector<DttConfigurationSegment> getSegmentsWithEnvironmentProfile(
		const EnvironmentProfile& environmentProfile) const;
	DttConfigurationSegment getDefaultSegment() const;
	size_t numberOfSegments() const;
	const DttConfigurationSegment& getSegment(size_t segmentNumber) const;

private:

	std::vector<DttConfigurationSegment> m_segments;

};
