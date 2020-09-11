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
#include "DomainPowerStatusBase.h"
#include "DomainPowerFilter.h"
#include "CachedValue.h"

class DomainPowerStatus_001 : public DomainPowerStatusBase
{
public:
	DomainPowerStatus_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainPowerStatus_001();

	// DomainPowerStatusInterface
	virtual PowerStatus getPowerStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getAveragePower(
		UIntN participantIndex,
		UIntN domainIndex,
		const PowerControlDynamicCaps& capabilities) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

private:
	// hide the copy constructor and = operator
	DomainPowerStatus_001(const DomainPowerStatus_001& rhs);
	DomainPowerStatus_001& operator=(const DomainPowerStatus_001& rhs);

	Power getPower(UIntN domainIndex);
	Power m_lastPowerSentToFilter;
	DomainPowerFilter m_domainPowerFilter;
};
