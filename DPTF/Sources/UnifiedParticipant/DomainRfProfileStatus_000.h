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
#include "DomainRfProfileStatusBase.h"
#include "RfProfileDataSet.h"

//
// Implements the Null Object pattern.  In the case that the functionality isn't implemented, we use
// this in place so we don't have to check for NULL pointers all throughout the participant implementation.
//

class DomainRfProfileStatus_000 : public DomainRfProfileStatusBase
{
public:
	DomainRfProfileStatus_000(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);

	// DomainRfProfileStatusInterface
	virtual RfProfileDataSet getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getWifiCapabilities(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getRfiDisable(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt64 getDvfsPoints(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getDlvrSsc(UIntN participantIndex, UIntN domainIndex) override;
	virtual Frequency getDlvrCenterFrequency(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setDdrRfiTable(
		UIntN participantIndex,
		UIntN domainIndex,
		const DdrfChannelBandPackage::WifiRfiDdr& ddrRfiStruct) override;
	virtual void sendMasterControlStatus(UIntN participantIndex, UIntN domainIndex, UInt32 masterControlStatus)
		override;
	virtual void setProtectRequest(UIntN participantIndex, UIntN domainIndex, UInt64 frequencyRate) override;
	virtual void setRfProfileOverride(UIntN participantIndex, UIntN domainIndex, const DptfBuffer& rfProfileBufferData)
		override;
	virtual void setDlvrCenterFrequency(UIntN participantIndex, UIntN domainIndex, Frequency frequency) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;
};
