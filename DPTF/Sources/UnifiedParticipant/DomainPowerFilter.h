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

#pragma once

#include "Dptf.h"
#include "CachedValue.h"
#include "ParticipantServicesInterface.h"

class DomainPowerFilter
{
public:
	DomainPowerFilter(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServices);
	virtual ~DomainPowerFilter();

	Power getAveragePower(const PowerControlDynamicCaps& capabilities, Power currentPower);
	Power getLastEwmaPower(PowerControlType::Type type);
	Power getLastPowerUsed(PowerControlType::Type type);
	void clearCachedData(void);
	std::shared_ptr<XmlNode> getXml();

protected:
	Power getPowerSeed(const PowerControlDynamicCaps& capabilities);
	double getPowerAlpha(PowerControlType::Type type);
	Power getMaxPowerChange(const PowerControlDynamicCaps& capabilities);
	void setLastEwmaPower(PowerControlType::Type type, Power value);
	void setLastPowerUsed(PowerControlType::Type type, Power value);
	Power calculatePowerHigh(PowerControlType::Type type, Power currentPower, Power newEwmaPower);
	Power calculatePowerLow(const PowerControlDynamicCaps& capabilities, Power currentPower, Power lastPowerUsed);
	double createFromHundredth(UInt32 hundredthValue);
	void updateHistory(PowerControlType::Type type, Power newPowerUsed, Power newEwmaPower);

private:
	// hide the copy constructor and = operator
	DomainPowerFilter(const DomainPowerFilter& rhs);
	DomainPowerFilter& operator=(const DomainPowerFilter& rhs);

	std::shared_ptr<XmlNode> createStatusNode(PowerControlType::Type type);

	// Cached Values
	std::map<PowerControlType::Type, CachedValue<Power>> m_powerSeed;
	std::map<PowerControlType::Type, CachedValue<double>> m_powerAlpha;
	std::map<PowerControlType::Type, CachedValue<double>> m_powerDelta;
	std::map<PowerControlType::Type, CachedValue<Power>> m_lastPowerUsed;
	std::map<PowerControlType::Type, CachedValue<Power>> m_lastEwmaPower;

	UIntN m_participantIndex;
	UIntN m_domainIndex;
	std::shared_ptr<ParticipantServicesInterface> m_participantServices;
};
