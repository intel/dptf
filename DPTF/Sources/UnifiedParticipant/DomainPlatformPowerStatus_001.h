/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
#include "DomainPlatformPowerStatusBase.h"

class DomainPlatformPowerStatus_001 : public DomainPlatformPowerStatusBase
{
public:
	DomainPlatformPowerStatus_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainPlatformPowerStatus_001(void);

	// DomainPlatformPowerStatusInterface
	virtual Power getMaxBatteryPower(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getPlatformRestOfPower(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getAdapterPowerRating(UIntN participantIndex, UIntN domainIndex) override;
	virtual DptfBuffer getBatteryStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual DptfBuffer getBatteryInformation(UIntN participantIndex, UIntN domainIndex) override;
	virtual PlatformPowerSource::Type getPlatformPowerSource(UIntN participantIndex, UIntN domainIndex) override;
	virtual ChargerType::Type getChargerType(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getPlatformBatterySteadyState(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getACNominalVoltage(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getACOperationalCurrent(UIntN participantIndex, UIntN domainIndex) override;
	virtual Percentage getAC1msPercentageOverload(UIntN participantIndex, UIntN domainIndex) override;
	virtual Percentage getAC2msPercentageOverload(UIntN participantIndex, UIntN domainIndex) override;
	virtual Percentage getAC10msPercentageOverload(UIntN participantIndex, UIntN domainIndex) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void clearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	// hide the copy constructor and = operator
	DomainPlatformPowerStatus_001(const DomainPlatformPowerStatus_001& rhs);
	DomainPlatformPowerStatus_001& operator=(const DomainPlatformPowerStatus_001& rhs);
	void initializeDataStructures(void);

	Power m_maxBatteryPower; // PMAX in mW
	Power m_adapterRating; // ARTG in mW
	Power m_platformRestOfPower; // PROP in mW
	PlatformPowerSource::Type m_platformPowerSource; // PSRC
	ChargerType::Type m_chargerType; // CTYP
	Power m_batterySteadyState; // PBSS in mW
	UInt32 m_acNominalVoltage; // AVOL in mV
	UInt32 m_acOperationalCurrent; // ACUR in mA
	Percentage m_ac1msPercentageOverload; // AP01 in %
	Percentage m_ac2msPercentageOverload; // AP02 in %
	Percentage m_ac10msPercentageOverload; // AP10 in %
};
