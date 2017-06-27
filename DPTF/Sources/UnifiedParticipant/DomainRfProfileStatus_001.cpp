/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "DomainRfProfileStatus_001.h"

//
// version 001 is for fivr
//

DomainRfProfileStatus_001::DomainRfProfileStatus_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainRfProfileStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainRfProfileStatus_001::~DomainRfProfileStatus_001(void)
{
}

RfProfileDataSet DomainRfProfileStatus_001::getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex)
{
	std::vector<RfProfileData> rfProfileDataSet;
	try 
	{
		Frequency centerFrequency = getParticipantServices()->primitiveExecuteGetAsFrequency(
			esif_primitive_type::GET_RFPROFILE_CENTER_FREQUENCY, domainIndex);
		RfProfileSupplementalData rfProfileSupplementalData(0, 0, 0, 0, RadioConnectionStatus::NotConnected, 0);
		RfProfileData rfProfileData(centerFrequency, Frequency(0), Frequency(0), rfProfileSupplementalData);
		rfProfileDataSet.insert(rfProfileDataSet.end(), rfProfileData);
	}
	catch (...)
	{
		getParticipantServices()->writeMessageDebug(
			ParticipantMessage(FLF, "Failed to update RF Channel Info. "));
	}
	return RfProfileDataSet(rfProfileDataSet);
}

void DomainRfProfileStatus_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_RFPROFILE_STATUS;
			capability.size = sizeof(capability);

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));
		}
	}
	catch (...)
	{
		// skip if there are any issue in sending log data
	}
}

void DomainRfProfileStatus_001::clearCachedData(void)
{
	// For now nothing is cached
}

std::shared_ptr<XmlNode> DomainRfProfileStatus_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("rfprofile_status");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	return root;
}

std::string DomainRfProfileStatus_001::getName(void)
{
	return "FIVR RF Profile Status";
}
