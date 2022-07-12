/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "DomainRfProfileStatusBase.h"

DomainRfProfileStatusBase::DomainRfProfileStatusBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainRfProfileStatusBase::~DomainRfProfileStatusBase()
{
}

void DomainRfProfileStatusBase::initializeRfProfileData(EsifCapabilityData *capability)
{
	for (UInt32 channelNumber = 0; channelNumber < MAX_FREQUENCY_CHANNEL_NUM; channelNumber++)
	{
		(*capability).data.rfProfileStatus.rfProfileFrequencyData[channelNumber].centerFrequency = Constants::Invalid;
		(*capability).data.rfProfileStatus.rfProfileFrequencyData[channelNumber].leftFrequencySpread = Constants::Invalid;
		(*capability).data.rfProfileStatus.rfProfileFrequencyData[channelNumber].rightFrequencySpread = Constants::Invalid;
	}
}
