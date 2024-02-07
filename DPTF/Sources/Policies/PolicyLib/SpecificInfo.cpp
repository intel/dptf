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

#include "SpecificInfo.h"
#include <algorithm>
using namespace std;

Bool compareTripPointsOnTemperature(
	pair<ParticipantSpecificInfoKey::Type, Temperature> lhs,
	pair<ParticipantSpecificInfoKey::Type, Temperature> rhs);
Bool compareTripPointsOnKey(
	pair<ParticipantSpecificInfoKey::Type, Temperature> lhs,
	pair<ParticipantSpecificInfoKey::Type, Temperature> rhs);

SpecificInfo::SpecificInfo()
	: m_specificInfo()
	, m_sortedTripPointsByValue()
	, m_sortedTripPointsByKey()
{
}

SpecificInfo::SpecificInfo(const map<ParticipantSpecificInfoKey::Type, Temperature>& specificInfo)
	: m_specificInfo(specificInfo)
{
	sortInfoByKey();
	sortInfoByValue();
}

vector<pair<ParticipantSpecificInfoKey::Type, Temperature>> SpecificInfo::getSortedByValue()
{
	return m_sortedTripPointsByValue;
}

vector<pair<ParticipantSpecificInfoKey::Type, Temperature>> SpecificInfo::getSortedByKey()
{
	return m_sortedTripPointsByKey;
}

Bool SpecificInfo::hasKey(ParticipantSpecificInfoKey::Type key)
{
	return (m_specificInfo.find(key) != m_specificInfo.end());
}

Temperature SpecificInfo::getTemperature(ParticipantSpecificInfoKey::Type key)
{
	const auto item = m_specificInfo.find(key);
	if (item != m_specificInfo.end())
	{
		return item->second;
	}
	else
	{
		throw dptf_exception(
			"Specific info key "
			+ string(ParticipantSpecificInfoKey::ToString(key))
			+ " not present.");
	}
}

Bool SpecificInfo::operator==(const SpecificInfo& rhs) const
{
	return rhs.m_specificInfo == this->m_specificInfo;
}

Bool SpecificInfo::operator!=(const SpecificInfo& rhs) const
{
	return !(*this == rhs);
}

shared_ptr<XmlNode> SpecificInfo::getXml() const
{
	auto wrapper = XmlNode::createWrapperElement("specific_info");
	for (auto tp : m_specificInfo)
	{
		const auto node = XmlNode::createDataElement(
			ParticipantSpecificInfoKey::ToString(tp.first),
			tp.second.toString());
		wrapper->addChild(node);
	}
	return wrapper;
}

void SpecificInfo::sortInfoByValue()
{
	m_sortedTripPointsByValue.insert(m_sortedTripPointsByValue.end(), m_specificInfo.begin(), m_specificInfo.end());
	sort(m_sortedTripPointsByValue.begin(), m_sortedTripPointsByValue.end(), compareTripPointsOnTemperature);
}

void SpecificInfo::sortInfoByKey()
{
	m_sortedTripPointsByKey.insert(m_sortedTripPointsByKey.end(), m_specificInfo.begin(), m_specificInfo.end());
	sort(m_sortedTripPointsByKey.begin(), m_sortedTripPointsByKey.end(), compareTripPointsOnKey);
}

Bool compareTripPointsOnTemperature(
	pair<ParticipantSpecificInfoKey::Type, Temperature> lhs,
	pair<ParticipantSpecificInfoKey::Type, Temperature> rhs)
{
	return (lhs.second < rhs.second);
}

Bool compareTripPointsOnKey(
	pair<ParticipantSpecificInfoKey::Type, Temperature> lhs,
	pair<ParticipantSpecificInfoKey::Type, Temperature> rhs)
{
	return (lhs.first < rhs.first);
}
