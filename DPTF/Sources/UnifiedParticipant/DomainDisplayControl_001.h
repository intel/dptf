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
#include "DomainDisplayControlBase.h"
#include "CachedValue.h"

//
// Implements the regular display controls, v001.
//

class DomainDisplayControl_001 : public DomainDisplayControlBase
{
public:
	DomainDisplayControl_001(
		UIntN participantIndex,
		UIntN domainIndex,
		const std::shared_ptr<ParticipantServicesInterface>& participantServicesInterface);
	~DomainDisplayControl_001() override;

	// remove the copy constructor and = operator
	DomainDisplayControl_001(const DomainDisplayControl_001& rhs) = delete;
	DomainDisplayControl_001& operator=(const DomainDisplayControl_001& rhs) = delete;

	DisplayControlDynamicCaps getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override;
	DisplayControlStatus getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	UIntN getUserPreferredDisplayIndex(UIntN participantIndex, UIntN domainIndex) override;
	UIntN getUserPreferredSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex) override;
	Bool isUserPreferredIndexModified(UIntN participantIndex, UIntN domainIndex) override;
	UIntN getSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex) override;
	DisplayControlSet getDisplayControlSet(UIntN participantIndex, UIntN domainIndex) override;
	void setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex) override;
	void setSoftBrightness(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex) override;
	void updateUserPreferredSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex) override;
	void restoreUserPreferredSoftBrightness(UIntN participantIndex, UIntN domainIndex) override;
	void setDisplayControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		DisplayControlDynamicCaps newCapabilities) override;
	void setDisplayCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;

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
	DisplayControlDynamicCaps createDisplayControlDynamicCaps(UIntN domainIndex);
	DisplayControlSet createDisplayControlSet(UIntN domainIndex) const;
	void throwIfControlIndexIsOutOfRange(UIntN displayControlIndex, UIntN domainIndex);
	static void throwIfDisplaySetIsEmpty(UIntN sizeOfSet);
	UIntN getLowerLimitIndex(UIntN domainIndex, DisplayControlSet displaySet) const;
	UIntN getUpperLimitIndex(UIntN domainIndex, DisplayControlSet displaySet) const;
	UIntN getAllowableDisplayBrightnessIndex(UIntN participantIndex, UIntN domainIndex, UIntN requestedIndex);
	
	// Vars (external)
	CachedValue<DisplayControlDynamicCaps> m_displayControlDynamicCaps;
	CachedValue<DisplayControlSet> m_displayControlSet;

	// Vars (internal)
	UIntN m_userPreferredIndex;
	UIntN m_lastSetDisplayBrightness;
	UIntN m_userPreferredSoftBrightnessIndex;
	Bool m_isUserPreferredIndexModified;
	Bool m_capabilitiesLocked;
};
