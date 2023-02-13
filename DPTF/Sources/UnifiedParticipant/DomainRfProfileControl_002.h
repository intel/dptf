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
#include "DomainRfProfileControlBase.h"

class DomainRfProfileControl_002 : public DomainRfProfileControlBase
{
public:
	DomainRfProfileControl_002(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainRfProfileControl_002(void);

	// DomainRfProfileControlInterface
	virtual RfProfileCapabilities getRfProfileCapabilities(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setRfProfileCenterFrequency(
		UIntN participantIndex,
		UIntN domainIndex,
		const Frequency& centerFrequency) override;
	virtual Percentage getSscBaselineSpreadValue(UIntN participantIndex, UIntN domainIndex) override;
	virtual Percentage getSscBaselineThreshold(UIntN participantIndex, UIntN domainIndex) override;
	virtual Percentage getSscBaselineGuardBand(UIntN participantIndex, UIntN domainIndex) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	// hide the copy constructor and = operator
	DomainRfProfileControl_002(const DomainRfProfileControl_002& rhs);
	DomainRfProfileControl_002& operator=(const DomainRfProfileControl_002& rhs);
};
