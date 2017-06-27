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

#include "DomainRfProfileStatus_002.h"

//
// version 002 is for wireless
//

DomainRfProfileStatus_002::DomainRfProfileStatus_002(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainRfProfileStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainRfProfileStatus_002::~DomainRfProfileStatus_002(void)
{
}

RfProfileDataSet DomainRfProfileStatus_002::getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex)
{
	RfProfileDataSet rfProfileDataSet;
	try 
	{
		DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
			esif_primitive_type::GET_RF_CHANNEL_INFO, ESIF_DATA_BINARY, domainIndex);
		rfProfileDataSet = RfProfileDataSet::createRfProfileDataFromDptfBuffer(buffer);
	}
	catch (...)
	{
		getParticipantServices()->writeMessageDebug(
			ParticipantMessage(FLF, "Failed to get Rf Channel Info. "));
	}
	return rfProfileDataSet;
}

void DomainRfProfileStatus_002::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
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

void DomainRfProfileStatus_002::clearCachedData(void)
{
	// FIXME: do we clear the cache for this control?
}

std::shared_ptr<XmlNode> DomainRfProfileStatus_002::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("rfprofile_status");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	return root;
}

std::string DomainRfProfileStatus_002::getName(void)
{
	return "Wireless RF Profile Status";
}
