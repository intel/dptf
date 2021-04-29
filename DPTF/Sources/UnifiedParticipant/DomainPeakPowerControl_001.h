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
#include "DomainPeakPowerControlBase.h"
#include "CachedValue.h"

//
// Implements the Null Object pattern.  In the case that the functionality isn't implemented, we use
// this in place so we don't have to check for NULL pointers all throughout the participant implementation.
//

class DomainPeakPowerControl_001 : public DomainPeakPowerControlBase
{
public:
	DomainPeakPowerControl_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainPeakPowerControl_001(void);

	// DomainPeakPowerControlInterface
	virtual Power getACPeakPower(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setACPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& acPeakPower) override;
	virtual Power getDCPeakPower(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setDCPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& dcPeakPower) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	void throwIfInvalidPower(const Power& power);

	// hide the copy constructor and = operator
	DomainPeakPowerControl_001(const DomainPeakPowerControl_001& rhs);
	DomainPeakPowerControl_001& operator=(const DomainPeakPowerControl_001& rhs);

	CachedValue<Power> m_acPeakPower;
	CachedValue<Power> m_dcPeakPower;
};
