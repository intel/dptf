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

#pragma once

#include "Dptf.h"
#include "ParticipantServicesInterface.h"
#include "DomainType.h"
#include "DomainFunctionalityVersions.h"
#include "ControlFactoryList.h"
#include "DomainControlList.h"
#include <memory>

class UnifiedDomain
{
public:
	UnifiedDomain(
		const Guid& guid,
		UIntN participantIndex,
		UIntN domainIndex,
		Bool domainEnabled,
		DomainType::Type domainType,
		std::string domainName,
		std::string domainDescription,
		DomainFunctionalityVersions domainFunctionalityVersions,
		const ControlFactoryList& classFactories,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	~UnifiedDomain(void);

	Guid getGuid(void);
	Bool isEnabled(void);
	void enable(void);
	void disable(void);

	DomainType::Type getDomainType(void);
	std::string getName(void);
	std::string getDescription(void);
	DomainFunctionalityVersions getDomainFunctionalityVersions(void);
	std::shared_ptr<XmlNode> getXml();
	std::shared_ptr<XmlNode> getArbitratorStatusForPolicy(UIntN policyIndex, ControlFactoryType::Type type) const;

	void clearAllCachedData(void);
	void clearAllCachedResults(void);

	std::shared_ptr<DomainActiveControlBase> getActiveControl(void);
	std::shared_ptr<DomainActivityStatusBase> getActivityStatusControl(void);
	std::shared_ptr<DomainCoreControlBase> getCoreControl(void);
	std::shared_ptr<DomainDisplayControlBase> getDisplayControl(void);
	std::shared_ptr<DomainEnergyControlBase> getEnergyControl(void);
	std::shared_ptr<DomainPeakPowerControlBase> getPeakPowerControl(void);
	std::shared_ptr<DomainPerformanceControlBase> getPerformanceControl(void);
	std::shared_ptr<DomainPowerControlBase> getPowerControl(void);
	std::shared_ptr<DomainPowerStatusBase> getPowerStatusControl(void);
	std::shared_ptr<DomainSystemPowerControlBase> getSystemPowerControl(void);
	std::shared_ptr<DomainPlatformPowerStatusBase> getPlatformPowerStatusControl(void);
	std::shared_ptr<DomainPriorityBase> getDomainPriorityControl(void);
	std::shared_ptr<DomainRfProfileControlBase> getRfProfileControl(void);
	std::shared_ptr<DomainRfProfileStatusBase> getRfProfileStatusControl(void);
	std::shared_ptr<DomainTemperatureBase> getTemperatureControl(void);
	std::shared_ptr<DomainProcessorControlBase> getProcessorControl(void);
	std::shared_ptr<DomainUtilizationBase> getUtilizationControl(void);
	std::shared_ptr<DomainBatteryStatusBase> getBatteryStatusControl(void);
	std::shared_ptr<DomainSocWorkloadClassificationBase> getSocWorkloadClassificationControl(void);
	std::shared_ptr<DomainDynamicEppBase> getDynamicEppControl();
	std::shared_ptr<DomainBiasControlBase> getBiasControl();

private:
	// hide the copy constructor and = operator
	UnifiedDomain(const UnifiedDomain& rhs);
	UnifiedDomain& operator=(const UnifiedDomain& rhs);

	Guid m_guid;
	UIntN m_participantIndex;
	UIntN m_domainIndex;
	Bool m_enabled;
	DomainType::Type m_domainType;
	std::string m_name;
	std::string m_description;
	DomainFunctionalityVersions m_domainFunctionalityVersions;
	std::shared_ptr<ParticipantServicesInterface> m_participantServicesInterface;
	std::shared_ptr<DomainControlList> m_domainControls;

	std::shared_ptr<ParticipantServicesInterface> getParticipantServices();
	void throwIfDomainNotEnabled(void);
};
