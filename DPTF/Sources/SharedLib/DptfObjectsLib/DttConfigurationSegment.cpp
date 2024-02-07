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

#include "DttConfigurationSegment.h"
#include <regex>
#include "MapOps.h"
#include "StringConverter.h"
#include "ThirdParty/nlohmann_json/json.hpp"
using namespace std;
using json = nlohmann::json; 

const string environmentCpuIdKeyPattern = "/ApplicableEnvironment/PlatformCpuIds/.*"s;
const string environmentSocBasePowerKeyPattern = "/ApplicableEnvironment/SocBasePower/.*"s;
const string environmentLabelKeyPattern = "/ApplicableEnvironment/Label/.*"s;
const string environmentEpoCpuIdKeyPattern = ".*Platform.*cpuid.*"s;
constexpr auto INDENT_WIDTH = 4;

map<string, string> generateKeysAndValues(const nlohmann::basic_json<>& jsonObject)
{
	const auto flattenedData = jsonObject.flatten();

	map<string, string> values;
	for (const auto& item : flattenedData.items())
	{
		values[item.key()] = StringConverter::trimQuotes(to_string(item.value()));
	}
	return values;
}

DttConfigurationSegment::DttConfigurationSegment(const map<string, string>& keyValues)
	: m_keyValues(keyValues)
{
}

DttConfigurationSegment DttConfigurationSegment::createFromBson(const vector<unsigned char>& bson)
{
	try
	{
		const auto jsonObject = nlohmann::json::from_bson(bson);
		return generateKeysAndValues(jsonObject);
	}
	catch (json::parse_error& e)
	{
		throw dptf_exception("json parsing error: "s + e.what());
	}
}

DttConfigurationSegment DttConfigurationSegment::createFromJsonString(const std::string& jsonString)
{
	try
	{
		const auto jsonObject = nlohmann::json::parse(jsonString);
		return generateKeysAndValues(jsonObject);
	}
	catch (json::parse_error& e)
	{
		throw dptf_exception("json parsing error: "s + e.what());
	}
}

DttConfigurationSegment DttConfigurationSegment::operator+(const DttConfigurationSegment& higherPriority) const
{
	auto combined = m_keyValues;
	for (const auto& higherPriorityItem : higherPriority.m_keyValues)
	{
		combined[higherPriorityItem.first] = higherPriorityItem.second;
	}
	return combined;
}

bool DttConfigurationSegment::operator<(const DttConfigurationSegment& other) const
{
	return m_keyValues < other.m_keyValues;
}

bool DttConfigurationSegment::operator==(const DttConfigurationSegment& other) const
{
	return m_keyValues == other.m_keyValues;
}

bool DttConfigurationSegment::operator!=(const DttConfigurationSegment& other) const
{
	return !(*this == other);
}

set<string> DttConfigurationSegment::getKeys() const
{
	return MapOps<string, string>::getKeys(m_keyValues);
}

set<string> DttConfigurationSegment::getKeysThatMatch(const DttConfigurationQuery& query) const
{
	set<string> result;
	for (const auto& kv : m_keyValues)
	{
		if (regex_match(kv.first, query.toRegex()))
		{
			result.emplace(kv.first);
		}
	}
	return result;
}

UInt32 DttConfigurationSegment::getValueAsUInt32(const string& key) const
{
	throwIfKeyDoesNotExist(key);
	return StringConverter::toUInt32(StringConverter::trimQuotes(m_keyValues.at(key)));
}

string DttConfigurationSegment::getValueAsString(const string& key) const
{
	throwIfKeyDoesNotExist(key);
	return StringConverter::trimQuotes(m_keyValues.at(key));
}

bool DttConfigurationSegment::hasValue(const string& value) const
{
	return any_of(m_keyValues.begin(), m_keyValues.end(), 
	[value](const pair<string, string>& kv) { return StringConverter::trimQuotes(kv.second) == value; });
}

bool DttConfigurationSegment::hasProperties(const std::set<DttConfigurationProperty>& properties) const
{
	for (const auto& property : properties)
	{
		const auto keys = getKeysThatMatch(property.key);
		if (none_of(keys.begin(), keys.end(),
			[property, this](const string& key)
			{
				const auto value = StringConverter::trimQuotes(this->m_keyValues.at(key));
				return regex_match(value, property.value.toRegex());
			}))
		{
			return false;
		}
	}
	return true;
}

bool DttConfigurationSegment::hasPropertiesIncludingEmptyValue(
	const std::set<DttConfigurationProperty>& properties) const
{
	for (const auto& property : properties)
	{
		const auto keys = getKeysThatMatch(property.key);
		if (!keys.empty()
			&& none_of(
				keys.begin(),
				keys.end(),
				[property, this](const string& key)
				{
					const auto value = StringConverter::trimQuotes(this->m_keyValues.at(key));
					if (value.empty())
					{
						return true;
					}
					return regex_match(value, property.value.toRegex());
				}))
		{
			return false;
		}
	}
	return true;
}

bool DttConfigurationSegment::matchesEnvironmentProfile(const EnvironmentProfile& environmentProfile) const
{
	set<DttConfigurationProperty> properties{};
	const auto cpuProperty = DttConfigurationProperty(
		DttConfigurationQuery(environmentCpuIdKeyPattern),
		DttConfigurationQuery(environmentProfile.cpuIdWithoutStepping));
	properties.insert(cpuProperty);

	if (environmentProfile.socBasePower.isValid())
	{
		const auto socBasePowerProperty = DttConfigurationProperty(
			DttConfigurationQuery(environmentSocBasePowerKeyPattern),
			DttConfigurationQuery(environmentProfile.socBasePower.toStringAsWatts(0)));
		properties.insert(socBasePowerProperty);
	}

	return hasProperties(properties);
}

bool DttConfigurationSegment::matchesCpuIdInEpoSegment(const string& cpuId) const
{
	set<DttConfigurationProperty> properties{};
	const auto cpuProperty = DttConfigurationProperty(
		DttConfigurationQuery(environmentEpoCpuIdKeyPattern), DttConfigurationQuery(cpuId));
	properties.insert(cpuProperty);

	return hasProperties(properties);
}

bool DttConfigurationSegment::matchesEnvironmentProfileIncludingEmptyValue(
	const EnvironmentProfile& environmentProfile) const
{
	set<DttConfigurationProperty> properties{};
	const auto cpuProperty = DttConfigurationProperty(
		DttConfigurationQuery(environmentCpuIdKeyPattern),
		DttConfigurationQuery(environmentProfile.cpuIdWithoutStepping));
	properties.insert(cpuProperty);
	
	if (environmentProfile.socBasePower.isValid())
	{
		const auto socBasePowerProperty = DttConfigurationProperty(
			DttConfigurationQuery(environmentSocBasePowerKeyPattern),
			DttConfigurationQuery(environmentProfile.socBasePower.toStringAsWatts(0)));
		properties.insert(socBasePowerProperty);
	}

	return hasPropertiesIncludingEmptyValue(properties);
}

bool DttConfigurationSegment::hasLabel(const string& label) const
{
	set<DttConfigurationProperty> properties{};
	const auto labelProperty = DttConfigurationProperty(DttConfigurationQuery(environmentLabelKeyPattern), DttConfigurationQuery(label));
	properties.insert(labelProperty);

	return hasProperties(properties);
}

set<string> DttConfigurationSegment::getKeysWithValue(const string& value) const
{
	set<string> result;
	for (const auto& kv : m_keyValues)
	{
		if (StringConverter::trimQuotes(kv.second) == value)
		{
			result.emplace(kv.first);
		}
	}
	return result;
}

bool DttConfigurationSegment::empty() const
{
	return m_keyValues.empty();
}

void DttConfigurationSegment::keepOnlyKeysThatMatch(const DttConfigurationQuery& query)
{
	map<string, string> result;
	for (const auto& kv : m_keyValues)
	{
		if (regex_match(kv.first, query.toRegex()))
		{
			result.insert(kv);
		}
	}
	m_keyValues = result;
}

void DttConfigurationSegment::keepOnlyKeysThatMatch(const std::set<DttConfigurationQuery>& regexPatterns)
{
	map<string,string> result;
	for (const auto& kv : m_keyValues)
	{
		if (any_of(regexPatterns.begin(), regexPatterns.end(),
			[kv](const DttConfigurationQuery& pattern)
			{
				return regex_match(kv.first, pattern.toRegex());
			}))
		{
			result.insert(kv);
		}
	}
	m_keyValues = result;
}

string DttConfigurationSegment::toKeyValueString() const
{
	json jsonObj;

	for (const auto& pair : m_keyValues)
	{
		jsonObj[pair.first] = pair.second;
	}

	return jsonObj.dump(INDENT_WIDTH);
}

string DttConfigurationSegment::toJsonString() const
{
	json jsonObj;

	for (const auto& pair : m_keyValues)
	{
		jsonObj[pair.first] = pair.second;
	}

	jsonObj = jsonObj.unflatten();
	return jsonObj.dump(INDENT_WIDTH);
}

void DttConfigurationSegment::throwIfKeyDoesNotExist(const string& key) const
{
	const auto result = m_keyValues.find(key);
	if (result == m_keyValues.end())
	{
		throw dptf_out_of_range("Policy Configuration Key "s + key + " does not exist"s);
	}
}