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

#pragma once

#include "Dptf.h"
#include "DomainCoreControlBase.h"
#include "CachedValue.h"

//
// Core Controls (Processor)
//

class DomainCoreControl_001 : public DomainCoreControlBase
{
public:
	DomainCoreControl_001(
		UIntN participantIndex,
		UIntN domainIndex,
		const std::shared_ptr<ParticipantServicesInterface>& participantServicesInterface);
	~DomainCoreControl_001() override;

	// remove the copy constructor and = operator
	DomainCoreControl_001(const DomainCoreControl_001& rhs) = delete;
	DomainCoreControl_001& operator=(const DomainCoreControl_001& rhs) = delete;

	// DomainCoreControlInterface
	CoreControlStaticCaps getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override;
	CoreControlDynamicCaps getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override;
	CoreControlLpoPreference getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex) override;
	CoreControlStatus getCoreControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	void setActiveCoreControl(
		UIntN participantIndex,
		UIntN domainIndex,
		const CoreControlStatus& coreControlStatus) override;

	// ParticipantActivityLoggingInterface
	void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	void onClearCachedData() override;
	std::string getName() override;
	std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

protected:
	void restore() override;

private:
	// Functions
	CoreControlStaticCaps createCoreControlStaticCaps(UIntN domainIndex) const;
	CoreControlDynamicCaps createCoreControlDynamicCaps(UIntN domainIndex);
	void verifyCoreControlStatus(UIntN domainIndex, const CoreControlStatus& coreControlStatus);

	// Vars (external)
	CachedValue<CoreControlStaticCaps> m_coreControlStaticCaps;
	CachedValue<CoreControlDynamicCaps> m_coreControlDynamicCaps;
	CachedValue<CoreControlStatus> m_coreControlStatus;
};
