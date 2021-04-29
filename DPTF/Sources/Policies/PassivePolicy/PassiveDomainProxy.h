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
#include "DomainProxy.h"

// represents a domain inside a participant.  holds cached records of all properties and potential controls for the
// domain.
class dptf_export PassiveDomainProxy : public DomainProxy
{
public:
	PassiveDomainProxy();
	PassiveDomainProxy(
		UIntN domainIndex,
		ParticipantProxyInterface* participant,
		const PolicyServicesInterfaceContainer& policyServices);
	virtual ~PassiveDomainProxy();

	// passive controls
	void requestLimit(UIntN target);
	void requestUnlimit(UIntN target);
	Bool canLimit(UIntN target);
	Bool canUnlimit(UIntN target);
	Bool commitLimits();
	void setArbitratedPowerLimit();
	void setArbitratedPerformanceLimit();
	void setArbitratedCoreLimit();
	void adjustPowerRequests();
	void adjustPerformanceRequests();
	void adjustCoreRequests();
	void setTstateUtilizationThreshold(UtilizationStatus tstateUtilizationThreshold);
	void clearAllRequestsForTarget(UIntN target);
	void clearAllPerformanceControlRequests();
	void clearAllPowerControlRequests();
	void clearAllCoreControlRequests();
	void clearAllDisplayControlRequests();
	void clearAllControlKnobRequests();
	std::shared_ptr<XmlNode> getXmlForPassiveControlKnobs();

private:
	// control knobs
	std::shared_ptr<PerformanceControlKnob> m_pstateControlKnob;
	std::shared_ptr<PerformanceControlKnob> m_tstateControlKnob;
	std::shared_ptr<PowerControlKnob> m_powerControlKnob;
	std::shared_ptr<DisplayControlKnob> m_displayControlKnob;
	std::shared_ptr<CoreControlKnob> m_coreControlKnob;
	std::shared_ptr<std::map<UIntN, UIntN>> m_perfControlRequests;

	// limiting/unlimiting helper functions
	Bool requestLimitPowerAndShouldContinue(UIntN target);
	Bool requestLimitPstatesWithCoresAndShouldContinue(UIntN target);
	Bool requestLimitCoresAndShouldContinue(UIntN target);
	Bool requestLimitTstatesAndContinue(UIntN target);
	Bool requestLimitDisplayAndContinue(UIntN target);
	Bool requestUnlimitDisplayAndContinue(UIntN target);
	Bool requestUnlimitTstatesAndContinue(UIntN target);
	Bool requestUnlimitCoresWithPstatesAndShouldContinue(UIntN target);
	Bool requestUnlimitPstatesAndShouldContinue(UIntN target);
	Bool requestUnlimitPowerAndShouldContinue(UIntN target);
	const PolicyServicesInterfaceContainer& getPolicyServices() const;
};