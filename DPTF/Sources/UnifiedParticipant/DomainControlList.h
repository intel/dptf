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

#pragma once

#include "Dptf.h"
#include "ParticipantServicesInterface.h"
#include "ControlBase.h"
#include "DomainType.h"
#include "DomainFunctionalityVersions.h"
#include "DomainActiveControlFactory.h"
#include "DomainActivityStatusFactory.h"
#include "DomainCoreControlFactory.h"
#include "DomainDisplayControlFactory.h"
#include "DomainPeakPowerControlFactory.h"
#include "DomainPerformanceControlFactory.h"
#include "DomainPowerControlFactory.h"
#include "DomainPowerStatusFactory.h"
#include "DomainPriorityFactory.h"
#include "DomainRfProfileControlFactory.h"
#include "DomainRfProfileStatusFactory.h"
#include "DomainProcessorControlFactory.h"
#include "ParticipantGetSpecificInfoFactory.h"
#include "ParticipantSetSpecificInfoFactory.h"
#include "DomainSystemPowerControlFactory.h"
#include "DomainPlatformPowerStatusFactory.h"
#include "DomainTemperatureFactory.h"
#include "DomainUtilizationFactory.h"
#include "DomainActiveControlBase.h"
#include "DomainActivityStatusBase.h"
#include "DomainCoreControlBase.h"
#include "DomainDisplayControlBase.h"
#include "DomainEnergyControlBase.h"
#include "DomainPeakPowerControlBase.h"
#include "DomainPerformanceControlBase.h"
#include "DomainPowerControlBase.h"
#include "DomainPowerStatusBase.h"
#include "DomainPriorityBase.h"
#include "DomainRfProfileControlBase.h"
#include "DomainRfProfileStatusBase.h"
#include "DomainTemperatureBase.h"
#include "DomainProcessorControlBase.h"
#include "DomainUtilizationBase.h"
#include "DomainSystemPowerControlBase.h"
#include "DomainPlatformPowerStatusBase.h"
#include "DomainBatteryStatusBase.h"
#include "DomainSocWorkloadClassificationBase.h"
#include "DomainDynamicEppBase.h"
#include "ControlFactoryList.h"
#include "XmlNode.h"
#include <memory>

class DomainControlList
{
public:
	DomainControlList(
		UIntN participantIndex,
		UIntN domainIndex,
		DomainFunctionalityVersions domainFunctionalityVersions,
		const ControlFactoryList& controlFactoryList,
		std::shared_ptr<ParticipantServicesInterface> participantServices);
	~DomainControlList(void);

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

	void clearAllCachedData(void);
	void clearAllCachedResults(void);
	std::shared_ptr<XmlNode> getXml();
	std::shared_ptr<XmlNode> getArbitratorStatusForPolicy(UIntN policyIndex, ControlFactoryType::Type type) const;

private:
	// hide the copy constructor and = operator
	DomainControlList(const DomainControlList& rhs);
	DomainControlList& operator=(const DomainControlList& rhs);

	const UIntN m_participantIndex;
	const UIntN m_domainIndex;
	DomainFunctionalityVersions m_domainFunctionalityVersions;
	const ControlFactoryList m_controlFactoryList;
	std::shared_ptr<ParticipantServicesInterface> m_participantServices;
	std::map<ControlFactoryType::Type, std::shared_ptr<ControlBase>> m_controlList;

	void makeAllControls();
	template <typename T> std::shared_ptr<T> makeControl(ControlFactoryType::Type factoryType, UInt8& controlVersion);
	template <typename T>
	std::shared_ptr<T> makeControl(
		ControlFactoryType::Type factoryType,
		UInt8& controlVersion,
		UInt8& associatedControlVersion);
	std::shared_ptr<ParticipantServicesInterface> getParticipantServices() const;
};
