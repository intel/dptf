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
#include "DomainActiveControlBase.h"
#include "CachedValue.h"
#include "FanCapabilities.h"

class DomainActiveControl_001 : public DomainActiveControlBase
{
public:
	DomainActiveControl_001(
		UIntN participantIndex,
		UIntN domainIndex,
		const std::shared_ptr<ParticipantServicesInterface>& participantServicesInterface);
	~DomainActiveControl_001() override;

	// remove the copy constructor and = operator
	DomainActiveControl_001(const DomainActiveControl_001& rhs) = delete;
	DomainActiveControl_001& operator=(const DomainActiveControl_001& rhs) = delete;

	// ParticipantActivityLoggingInterface
	void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	void onClearCachedData() override;
	std::string getName() override;
	std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

protected:
	void capture() override;
	void restore() override;

	DptfBuffer getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override;
	DptfBuffer getActiveControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override;
	DptfBuffer getActiveControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	DptfBuffer getActiveControlSet(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getActiveControlFanOperatingMode(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getActiveControlFanCapabilities(UIntN participantIndex, UIntN domainIndex) override;
	void setActiveControl(UIntN participantIndex, UIntN domainIndex, const Percentage& fanSpeed) override;
	void setActiveControlFanDirection(UInt32 fanDirection) override;
	void setActiveControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		ActiveControlDynamicCaps newCapabilities) override;
	void setFanCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;
	void setActiveControlFanOperatingMode(UInt32 fanOperatingMode) override;

private:

	// Functions
	ActiveControlSet createActiveControlSet(UIntN domainIndex) const;
	void throwIfFineGrainedControlIsNotSupported(UIntN participantIndex, UIntN domainIndex);

	// Vars
	CachedValue<ActiveControlStatus> m_initialStatus;
	Bool m_capabilitiesLocked;
	FanCapabilities m_fanCapabilities;
};
