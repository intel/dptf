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
#include "DomainRfProfileStatusBase.h"
#include "RfProfileDataSet.h"

//
// version 001 is for fivr
//

class DomainRfProfileStatus_001 : public DomainRfProfileStatusBase
{
public:
	DomainRfProfileStatus_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainRfProfileStatus_001(void);

	// DomainRfProfileStatusInterface
	virtual RfProfileDataSet getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getWifiCapabilities(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getRfiDisable(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt64 getDvfsPoints(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setDdrRfiTable(
		UIntN participantIndex,
		UIntN domainIndex,
		DdrfChannelBandPackage::WifiRfiDdr ddrRfiStruct) override;
	virtual void setProtectRequest(UIntN participantIndex, UIntN domainIndex, UInt64 frequencyRate) override;
	virtual void setRfProfileOverride(UIntN participantIndex, UIntN domainIndex, const DptfBuffer& rfProfileBufferData)
		override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	// hide the copy constructor and = operator
	DomainRfProfileStatus_001(const DomainRfProfileStatus_001& rhs);
	DomainRfProfileStatus_001& operator=(const DomainRfProfileStatus_001& rhs);
};
