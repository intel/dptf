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

#include "DttConfigurationSegment.h"
#include <regex>
#include "MapOps.h"
#include "StringConverter.h"
#include "ThirdParty/nlohmann_json/json.hpp"
using namespace std;

const string environmentCpuIdKeyPattern = "/ApplicableEnvironment/PlatformCpuIds/.*"s;

DttConfigurationSegment::DttConfigurationSegment(const map<string, string>& keyValues)
	: m_keyValues(keyValues)
{
}

DttConfigurationSegment DttConfigurationSegment::createFromBson(const vector<unsigned char>& bson)
{
	const auto data = nlohmann::json::from_bson(bson);
	const auto flattenedData = data.flatten();

	map<string, string> values;
	for (const auto& item : flattenedData.items())
	{
		values[item.key()] = StringConverter::trimQuotes(to_string(item.value()));
	}
	return values;
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

bool DttConfigurationSegment::matchesEnvironmentProfile(const EnvironmentProfile& environmentProfile) const
{
	// build up properties list
	const string cpuIdValuePattern = environmentProfile.cpuIdWithoutStepping;
	const set<DttConfigurationProperty> properties{DttConfigurationProperty(
		DttConfigurationQuery(environmentCpuIdKeyPattern), DttConfigurationQuery(cpuIdValuePattern))
	};
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

void DttConfigurationSegment::throwIfKeyDoesNotExist(const string& key) const
{
	const auto result = m_keyValues.find(key);
	if (result == m_keyValues.end())
	{
		throw dptf_out_of_range("Policy Configuration Key "s + key + " does not exist"s);
	}
}