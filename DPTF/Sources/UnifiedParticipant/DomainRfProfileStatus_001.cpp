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

#include "DomainRfProfileStatus_001.h"
#include "StatusFormat.h"
#include "BinaryParse.h"

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
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to update RF Channel Info. "; });
	}
	return RfProfileDataSet(rfProfileDataSet);
}

UInt32 DomainRfProfileStatus_001::getWifiCapabilities(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainRfProfileStatus_001::getRfiDisable(UIntN participantIndex, UIntN domainIndex)
{
	UInt32 rfiDisable = Constants::Invalid;

	try
	{
		rfiDisable =
			getParticipantServices()->primitiveExecuteGetAsUInt32(esif_primitive_type::GET_RFI_DISABLE, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to get RFI Enable/Disable Info. "; });
	}

	return rfiDisable;
}

UInt64 DomainRfProfileStatus_001::getDvfsPoints(UIntN participantIndex, UIntN domainIndex)
{
	UInt64 numberOfDvfsPoints = Constants::Invalid;

	try
	{
		numberOfDvfsPoints = getParticipantServices()->primitiveExecuteGetAsUInt64(
			esif_primitive_type::GET_DDR_DVFS_DATA_RATE, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to get DVFS Data Rate Info. "; });
	}
	return numberOfDvfsPoints;
}

void DomainRfProfileStatus_001::setDdrRfiTable(
	UIntN participantIndex,
	UIntN domainIndex,
	DdrfChannelBandPackage::WifiRfiDdr ddrRfiStruct)
{
	throw not_implemented();
}

void DomainRfProfileStatus_001::setProtectRequest(UIntN participantIndex, UIntN domainIndex, UInt64 frequencyRate)
{
	try
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return (
				"Setting the DVFS Rate Protect Request for the rate: " + StatusFormat::friendlyValue(frequencyRate));
		});

		getParticipantServices()->primitiveExecuteSetAsUInt64(
			esif_primitive_type::SET_DDR_DVFS_RFI_RESTRICTION, frequencyRate, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return (
				"Failed to set the DVFS Rate Protect Request for the rate: "
				+ StatusFormat::friendlyValue(frequencyRate));
		});
	}
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
			initializeRfProfileData(&capability);

			auto rfProfileDataSet = getRfProfileDataSet(participantIndex, domainIndex).getRfProfileData();
			/* Processor or PCH only have one channel with info */
			UInt32 channelNumber = 0;
			auto rfProfileData = rfProfileDataSet.begin();
			if (rfProfileData != rfProfileDataSet.end())
			{
				capability.data.rfProfileStatus.rfProfileFrequencyData[channelNumber].centerFrequency =
					(UInt32)rfProfileData->getCenterFrequency();
				capability.data.rfProfileStatus.rfProfileFrequencyData[channelNumber].leftFrequencySpread =
					(UInt32)rfProfileData->getLeftFrequencySpread();
				capability.data.rfProfileStatus.rfProfileFrequencyData[channelNumber].rightFrequencySpread =
					(UInt32)rfProfileData->getRightFrequencySpread();

				getParticipantServices()->sendDptfEvent(
					ParticipantEvent::DptfParticipantControlAction,
					domainIndex,
					Capability::getEsifDataFromCapabilityData(&capability));
			}
		}
	}
	catch (...)
	{
		// skip if there are any issue in sending log data
	}
}

void DomainRfProfileStatus_001::onClearCachedData(void)
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

void DomainRfProfileStatus_001::setRfProfileOverride(
	UIntN participantIndex,
	UIntN domainIndex,
	const DptfBuffer& rfProfileBufferData)
{
	throw not_implemented();
}
