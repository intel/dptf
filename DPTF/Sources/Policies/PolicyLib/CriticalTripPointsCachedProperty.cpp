/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "CriticalTripPointsCachedProperty.h"
#include "XmlCommon.h"
#include "PolicyLogger.h"
using namespace std;

CriticalTripPointsCachedProperty::CriticalTripPointsCachedProperty(
	const PolicyServicesInterfaceContainer& policyServices,
	UIntN participantIndex)
	: CachedProperty()
	, ParticipantProperty(participantIndex, policyServices)
	, m_criticalTripPoints(map<ParticipantSpecificInfoKey::Type, Temperature>())
{
}

CriticalTripPointsCachedProperty::~CriticalTripPointsCachedProperty(void)
{
}

void CriticalTripPointsCachedProperty::refreshData(void)
{
	vector<ParticipantSpecificInfoKey::Type> specificInfoList;
	specificInfoList.push_back(ParticipantSpecificInfoKey::Critical);
	specificInfoList.push_back(ParticipantSpecificInfoKey::Hot);
	specificInfoList.push_back(ParticipantSpecificInfoKey::Warm);
	m_criticalTripPoints = SpecificInfo(getPolicyServices().participantGetSpecificInfo->getParticipantSpecificInfo(
		getParticipantIndex(), specificInfoList));
}

const SpecificInfo& CriticalTripPointsCachedProperty::getTripPoints()
{
	if (isCacheValid() == false)
	{
		refresh();
	}
	return m_criticalTripPoints;
}

Bool CriticalTripPointsCachedProperty::supportsProperty(void)
{
	// make sure the critical trip points contain at least one of {warm, hot, critical}
	try
	{
		auto tripPoints = getTripPoints();

		if (tripPoints.hasKey(ParticipantSpecificInfoKey::Warm))
		{
			const Temperature warmTripPoint = tripPoints.getTemperature(ParticipantSpecificInfoKey::Warm);
			if (warmTripPoint.isValid() && static_cast<UInt32>(warmTripPoint) != Constants::MaxUInt32)
			{
				return true;
			}
		}

		if (tripPoints.hasKey(ParticipantSpecificInfoKey::Hot))
		{
			const Temperature hotTripPoint = tripPoints.getTemperature(ParticipantSpecificInfoKey::Hot);
			if (hotTripPoint.isValid() && static_cast<UInt32>(hotTripPoint) != Constants::MaxUInt32)
			{
				return true;
			}
		}

		if (tripPoints.hasKey(ParticipantSpecificInfoKey::Critical))
		{
			const Temperature criticalTripPoint = tripPoints.getTemperature(ParticipantSpecificInfoKey::Critical);
			if (criticalTripPoint.isValid() && static_cast<UInt32>(criticalTripPoint) != Constants::MaxUInt32)
			{
				return true;
			}
		}

		POLICY_LOG_MESSAGE_DEBUG({ return "No valid Warm, Hot and Critical trip points."; });
		return false;
	}
	catch (dptf_exception&)
	{
		return false;
	}
}

std::shared_ptr<XmlNode> CriticalTripPointsCachedProperty::getXml()
{
	return getTripPoints().getXml();
}
