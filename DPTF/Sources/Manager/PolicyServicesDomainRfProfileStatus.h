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
#include "PolicyServices.h"
#include "DomainRfProfileStatusInterface.h"
#include "RfProfileDataSet.h"

class PolicyServicesDomainRfProfileStatus final : public PolicyServices, public DomainRfProfileStatusInterface
{
public:
	PolicyServicesDomainRfProfileStatus(DptfManagerInterface* dptfManager, UIntN policyIndex);

	RfProfileDataSet getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getWifiCapabilities(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getRfiDisable(UIntN participantIndex, UIntN domainIndex) override;
	UInt64 getDvfsPoints(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getDlvrSsc(UIntN participantIndex, UIntN domainIndex) override;
	Frequency getDlvrCenterFrequency(UIntN participantIndex, UIntN domainIndex) override;
	void setDdrRfiTable(
		UIntN participantIndex,
		UIntN domainIndex,
		const DdrfChannelBandPackage::WifiRfiDdr& ddrRfiStruct) override;
	void sendMasterControlStatus(UIntN participantIndex, UIntN domainIndex, UInt32 masterControlStatus)
		override;
	void setProtectRequest(UIntN participantIndex, UIntN domainIndex, UInt64 frequencyRate) override;
	void setRfProfileOverride(UIntN participantIndex, UIntN domainIndex, const DptfBuffer& rfProfileBufferData)
		override;
	void setDlvrCenterFrequency(UIntN participantIndex, UIntN domainIndex, Frequency frequency) override;
};
