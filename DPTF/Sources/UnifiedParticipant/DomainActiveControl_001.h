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
#include "DomainActiveControlBase.h"
#include "CachedValue.h"

class DomainActiveControl_001 : public DomainActiveControlBase
{
public:
	DomainActiveControl_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainActiveControl_001(void);

	// DomainActiveControlInterface
	virtual ActiveControlStaticCaps getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override;
	virtual ActiveControlDynamicCaps getActiveControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override;
	virtual ActiveControlStatus getActiveControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual ActiveControlSet getActiveControlSet(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setActiveControl(UIntN participantIndex, UIntN domainIndex, const Percentage& fanSpeed) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void clearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

protected:
	virtual void capture(void) override;
	virtual void restore(void) override;

private:
	// hide the copy constructor and = operator
	DomainActiveControl_001(const DomainActiveControl_001& rhs);
	DomainActiveControl_001& operator=(const DomainActiveControl_001& rhs);

	// Functions
	ActiveControlSet createActiveControlSet(UIntN domainIndex);
	ActiveControlStaticCaps createActiveControlStaticCaps(UIntN domainIndex);
	ActiveControlDynamicCaps createActiveControlDynamicCaps(UIntN domainIndex);
	ActiveControlStatus createActiveControlStatus(UIntN domainIndex);
	void throwIfFineGrainedControlIsNotSupported(UIntN participantIndex, UIntN domainIndex);

	// Vars
	CachedValue<ActiveControlStaticCaps> m_activeControlStaticCaps;
	CachedValue<ActiveControlDynamicCaps> m_activeControlDynamicCaps;
	CachedValue<ActiveControlStatus> m_activeControlStatus;
	CachedValue<ActiveControlSet> m_activeControlSet;
	CachedValue<ActiveControlStatus> m_initialStatus;
};
