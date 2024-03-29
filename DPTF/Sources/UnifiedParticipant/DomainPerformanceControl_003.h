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
#include "DomainPerformanceControlBase.h"
#include "CachedValue.h"

// Graphics Performance Controls

class DomainPerformanceControl_003 : public DomainPerformanceControlBase
{
public:
	DomainPerformanceControl_003(
		UIntN participantIndex,
		UIntN domainIndex,
		const std::shared_ptr<ParticipantServicesInterface>& participantServicesInterface);
	~DomainPerformanceControl_003() override;

	// remove the copy constructor and = operator
	DomainPerformanceControl_003(const DomainPerformanceControl_003& rhs) = delete;
	DomainPerformanceControl_003& operator=(const DomainPerformanceControl_003& rhs) = delete;

	// DomainPerformanceControlInterface
	PerformanceControlStaticCaps getPerformanceControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
		override;
	PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
		override;
	PerformanceControlStatus getPerformanceControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	PerformanceControlSet getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex) override;
	void setPerformanceControl(UIntN participantIndex, UIntN domainIndex, UIntN performanceControlIndex)
		override;
	void setPerformanceControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PerformanceControlDynamicCaps newCapabilities) override;
	void setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;

	// ComponentExtendedInterface
	void onClearCachedData() override;
	std::string getName() override;
	std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

protected:
	void capture() override;
	void restore() override;
	UIntN getCurrentPerformanceControlIndex(UIntN participantIndex, UIntN domainIndex) override;

private:
	PerformanceControlSet createPerformanceControlSet(UIntN domainIndex) const;
	UIntN snapIfPerformanceControlIndexIsOutOfBounds(UIntN domainIndex, UIntN performanceControlIndex);
	PerformanceControlDynamicCaps createPerformanceControlDynamicCaps(UIntN domainIndex);
	static PerformanceControlStaticCaps createPerformanceControlStaticCaps();

	CachedValue<PerformanceControlStaticCaps> m_performanceControlStaticCaps;
	CachedValue<PerformanceControlDynamicCaps> m_performanceControlDynamicCaps;
	CachedValue<PerformanceControlSet> m_performanceControlSet;
	CachedValue<PerformanceControlStatus> m_performanceControlStatus;
	CachedValue<PerformanceControlDynamicCaps> m_initialStatus;
	Bool m_capabilitiesLocked;
};
