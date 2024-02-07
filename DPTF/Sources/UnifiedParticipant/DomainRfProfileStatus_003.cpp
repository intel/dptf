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

#include "DomainRfProfileStatus_003.h"
#include "StatusFormat.h"

//
// version 003 is for memory
//

DomainRfProfileStatus_003::DomainRfProfileStatus_003(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainRfProfileStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainRfProfileStatus_003::~DomainRfProfileStatus_003(void)
{
}

void DomainRfProfileStatus_003::setRfProfileOverride(
	UIntN participantIndex,
	UIntN domainIndex, 
	const DptfBuffer& rfProfileBufferData)
{
	throw not_implemented();
}

RfProfileDataSet DomainRfProfileStatus_003::getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex)
{
	std::vector<RfProfileData> rfProfileDataSet;
	try
	{
		Frequency centerFrequency = getParticipantServices()->primitiveExecuteGetAsFrequency(
			esif_primitive_type::GET_MEMORY_SPEED, domainIndex);
		RfProfileSupplementalData rfProfileSupplementalData(RadioConnectionStatus::NotConnected);
		RfProfileData rfProfileData(
			false,
			Constants::Invalid,
			centerFrequency,
			Frequency(0),
			Frequency(0),
			Frequency(0),
			Constants::Invalid,
			0,
			rfProfileSupplementalData);
		rfProfileDataSet.insert(rfProfileDataSet.end(), rfProfileData);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to update RF Channel Info. Failed to get Memory Bandwidth Info."; });
	}
	return RfProfileDataSet(rfProfileDataSet);
}

UInt32 DomainRfProfileStatus_003::getWifiCapabilities(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainRfProfileStatus_003::getRfiDisable(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt64 DomainRfProfileStatus_003::getDvfsPoints(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainRfProfileStatus_003::getDlvrSsc(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

Frequency DomainRfProfileStatus_003::getDlvrCenterFrequency(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainRfProfileStatus_003::setDdrRfiTable(
	UIntN participantIndex,
	UIntN domainIndex,
	const DdrfChannelBandPackage::WifiRfiDdr& ddrRfiStruct)
{
	throw not_implemented();
}

void DomainRfProfileStatus_003::sendMasterControlStatus(
	UIntN participantIndex,
	UIntN domainIndex,
	UInt32 masterControlStatus)
{
	throw not_implemented();
}

void DomainRfProfileStatus_003::setProtectRequest(UIntN participantIndex, UIntN domainIndex, UInt64 frequencyRate)
{
	throw not_implemented();
}

void DomainRfProfileStatus_003::setDlvrCenterFrequency(UIntN participantIndex, UIntN domainIndex, Frequency frequency)
{
	throw not_implemented();
}

void DomainRfProfileStatus_003::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainRfProfileStatus_003::onClearCachedData(void)
{
	// do nothing, no cached data
}

std::shared_ptr<XmlNode> DomainRfProfileStatus_003::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("rfprofile_status");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	return root;
}

std::string DomainRfProfileStatus_003::getName(void)
{
	return "Memory RF Profile Status";
}
