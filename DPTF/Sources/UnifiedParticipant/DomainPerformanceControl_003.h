/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainPerformanceControl_003(void);

	// DomainPerformanceControlInterface
	virtual PerformanceControlStaticCaps getPerformanceControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
		override;
	virtual PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
		override;
	virtual PerformanceControlStatus getPerformanceControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual PerformanceControlSet getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setPerformanceControl(UIntN participantIndex, UIntN domainIndex, UIntN performanceControlIndex)
		override;
	virtual void setPerformanceControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PerformanceControlDynamicCaps newCapabilities) override;
	virtual void setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

protected:
	virtual void capture(void) override;
	virtual void restore(void) override;
	virtual UIntN getCurrentPerformanceControlIndex(UIntN participantIndex, UIntN domainIndex) override;

private:
	// hide the copy constructor and = operator
	DomainPerformanceControl_003(const DomainPerformanceControl_003& rhs);
	DomainPerformanceControl_003& operator=(const DomainPerformanceControl_003& rhs);

	PerformanceControlSet createPerformanceControlSet(UIntN domainIndex);
	UIntN snapIfPerformanceControlIndexIsOutOfBounds(UIntN domainIndex, UIntN performanceControlIndex);
	PerformanceControlDynamicCaps createPerformanceControlDynamicCaps(UIntN domainIndex);
	PerformanceControlStaticCaps createPerformanceControlStaticCaps(void);

	CachedValue<PerformanceControlStaticCaps> m_performanceControlStaticCaps;
	CachedValue<PerformanceControlDynamicCaps> m_performanceControlDynamicCaps;
	CachedValue<PerformanceControlSet> m_performanceControlSet;
	CachedValue<PerformanceControlStatus> m_performanceControlStatus;
	CachedValue<PerformanceControlDynamicCaps> m_initialStatus;
	Bool m_capabilitiesLocked;
};
