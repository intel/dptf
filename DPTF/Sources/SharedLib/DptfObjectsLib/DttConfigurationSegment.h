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
#include "DttConfigurationQuery.h"
#include <set>
#include <map>
#include <string>
#include <vector>
#include "DttConfigurationProperty.h"
#include "EnvironmentProfile.h"

class dptf_export DttConfigurationSegment
{
public:

	DttConfigurationSegment() = default;
	DttConfigurationSegment(const std::map<std::string, std::string>& keyValues);
	static DttConfigurationSegment createFromBson(const std::vector<unsigned char>& bson);
	static DttConfigurationSegment createFromJsonString(const std::string& jsonString);

	DttConfigurationSegment operator+(const DttConfigurationSegment& higherPriority) const;
	bool operator<(const DttConfigurationSegment& other) const;
	bool operator==(const DttConfigurationSegment& other) const;
	bool operator!=(const DttConfigurationSegment& other) const;

	std::set<std::string> getKeys() const;
	std::set<std::string> getKeysThatMatch(const DttConfigurationQuery& query) const;
	UInt32 getValueAsUInt32(const std::string& key) const;
	std::string getValueAsString(const std::string& key) const;
	bool hasValue(const std::string& value) const;
	bool hasProperties(const std::set<DttConfigurationProperty>& properties) const;
	bool matchesEnvironmentProfile(const EnvironmentProfile& environmentProfile) const;
	bool matchesCpuIdInEpoSegment(const std::string& environmentProfile) const;
	bool matchesEnvironmentProfileIncludingEmptyValue(const EnvironmentProfile& environmentProfile) const;
	bool hasLabel(const std::string& label) const;
	std::set<std::string> getKeysWithValue(const std::string& value) const;
	bool empty() const;
	void keepOnlyKeysThatMatch(const DttConfigurationQuery& query);
	void keepOnlyKeysThatMatch(const std::set<DttConfigurationQuery>& regexPatterns);
	std::string toString() const;

private:

	std::map<std::string, std::string> m_keyValues;
	void throwIfKeyDoesNotExist(const std::string& key) const;
	bool hasPropertiesIncludingEmptyValue(const std::set<DttConfigurationProperty>& properties) const;
};
