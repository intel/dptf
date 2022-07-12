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

#include "DomainRfProfileStatus_000.h"

DomainRfProfileStatus_000::DomainRfProfileStatus_000(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainRfProfileStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
	// Do nothing.  Not an error.
}

RfProfileDataSet DomainRfProfileStatus_000::getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainRfProfileStatus_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainRfProfileStatus_000::getWifiCapabilities(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainRfProfileStatus_000::getRfiDisable(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt64 DomainRfProfileStatus_000::getDvfsPoints(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainRfProfileStatus_000::setDdrRfiTable(
	UIntN participantIndex,
	UIntN domainIndex,
	DdrfChannelBandPackage::WifiRfiDdr ddrRfiStruct)
{
	throw not_implemented();
}

void DomainRfProfileStatus_000::setProtectRequest(UIntN participantIndex, UIntN domainIndex, UInt64 frequencyRate)
{
	throw not_implemented();
}

void DomainRfProfileStatus_000::onClearCachedData(void)
{
	// Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainRfProfileStatus_000::getXml(UIntN domainIndex)
{
	throw not_implemented();
}

std::string DomainRfProfileStatus_000::getName(void)
{
	return "RF Profile Status (Version 0)";
}
