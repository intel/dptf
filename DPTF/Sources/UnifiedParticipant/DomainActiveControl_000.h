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
#include "ActiveControlStaticCaps.h"
#include "ActiveControlDynamicCaps.h"
#include "ActiveControlStatus.h"
#include "ActiveControlSet.h"

//
// Implements the Null Object pattern.  In the case that the functionality isn't implemented, we use
// this in place so we don't have to check for NULL pointers all throughout the participant implementation.
//

class DomainActiveControl_000 : public DomainActiveControlBase
{
public:
	DomainActiveControl_000(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

protected:
	virtual DptfBuffer getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override;
	virtual DptfBuffer getActiveControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override;
	virtual DptfBuffer getActiveControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual DptfBuffer getActiveControlSet(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getActiveControlFanOperatingMode(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getActiveControlFanCapabilities(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setActiveControl(UIntN participantIndex, UIntN domainIndex, const Percentage& fanSpeed) override;
	virtual void setActiveControlFanDirection(UInt32 fanDirection) override;
	virtual void setActiveControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		ActiveControlDynamicCaps newCapabilities) override;
	virtual void setFanCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;
	virtual void setActiveControlFanOperatingMode(UInt32 fanOperatingMode) override;
};
