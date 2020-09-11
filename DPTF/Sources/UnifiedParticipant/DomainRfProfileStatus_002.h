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
#include "DomainRfProfileStatusBase.h"
#include "RfProfileDataSet.h"
//
// version 002 is for wireless
//

class DomainRfProfileStatus_002 : public DomainRfProfileStatusBase
{
public:
	DomainRfProfileStatus_002(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainRfProfileStatus_002(void);

	// DomainRfProfileStatusInterface
	virtual RfProfileDataSet getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	RfProfileDataSet m_rfProfileDataSet;
	// hide the copy constructor and = operator
	DomainRfProfileStatus_002(const DomainRfProfileStatus_002& rhs);
	DomainRfProfileStatus_002& operator=(const DomainRfProfileStatus_002& rhs);
	Frequency getRfProfileGuardband(UIntN participantIndex, UIntN domainIndex);
};
