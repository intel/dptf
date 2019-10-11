/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
	pair<ParticipantSpecificInfoKey::Type, Temperature> left,
	pair<ParticipantSpecificInfoKey::Type, Temperature> right);
Bool compareTripPointsOnKey(
	pair<ParticipantSpecificInfoKey::Type, Temperature> left,
	pair<ParticipantSpecificInfoKey::Type, Temperature> right);

SpecificInfo::SpecificInfo()
{
}

SpecificInfo::SpecificInfo(std::map<ParticipantSpecificInfoKey::Type, Temperature> specificInfo)
	: m_specificInfo(specificInfo)
{
	sortInfoByKey();
	sortInfoByValue();
}

SpecificInfo::~SpecificInfo(void)
{
}

std::vector<std::pair<ParticipantSpecificInfoKey::Type, Temperature>> SpecificInfo::getSortedByValue()
{
	return m_sortedTripPointsByValue;
}

std::vector<std::pair<ParticipantSpecificInfoKey::Type, Temperature>> SpecificInfo::getSortedByKey()
{
	return m_sortedTripPointsByKey;
}

Bool SpecificInfo::hasKey(ParticipantSpecificInfoKey::Type key)
{
	return (m_specificInfo.find(key) != m_specificInfo.end());
}

Temperature SpecificInfo::getTemperature(ParticipantSpecificInfoKey::Type key)
{
	auto item = m_specificInfo.find(key);
	if (item != m_specificInfo.end())
	{
		return item->second;
	}
	else
	{
		throw dptf_exception(
			"Specific info key "
			+ std::string(ParticipantSpecificInfoKey::ToString((ParticipantSpecificInfoKey::Type)(key)))
			+ " not present.");
	}
}

Bool SpecificInfo::operator==(SpecificInfo& rhs)
{
	return rhs.m_specificInfo == this->m_specificInfo;
}

Bool SpecificInfo::operator!=(SpecificInfo& rhs)
{
	return !(*this == rhs);
}

std::shared_ptr<XmlNode> SpecificInfo::getXml() const
{
	auto wrapper = XmlNode::createWrapperElement("specific_info");
	for (auto tp = m_specificInfo.begin(); tp != m_specificInfo.end(); tp++)
	{
		auto node = XmlNode::createDataElement(
			ParticipantSpecificInfoKey::ToString((ParticipantSpecificInfoKey::Type)tp->first), tp->second.toString());
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
	pair<ParticipantSpecificInfoKey::Type, Temperature> left,
	pair<ParticipantSpecificInfoKey::Type, Temperature> right)
{
	return (left.second < right.second);
}

Bool compareTripPointsOnKey(
	pair<ParticipantSpecificInfoKey::Type, Temperature> left,
	pair<ParticipantSpecificInfoKey::Type, Temperature> right)
{
	return (left.first < right.first);
}
