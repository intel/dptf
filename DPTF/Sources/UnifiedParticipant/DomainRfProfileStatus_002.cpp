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

#include "DomainRfProfileStatus_002.h"
#include "StatusFormat.h"

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

void DomainRfProfileStatus_002::setRfProfileOverride(
	UIntN participantIndex,
	UIntN domainIndex, 
	const DptfBuffer& rfProfileBufferData)
{
	auto domainType = getParticipantServices()->getDomainType(domainIndex);
	PARTICIPANT_LOG_MESSAGE_INFO({
		std::stringstream message;
		message << "setRfProfileOverride" << getParticipantIndex() << ", "
				<< "domain " << getName() << " "
				<< "DomainType " << domainType;
		return message.str();
	});
	if (rfProfileBufferData.size() >= sizeof(esif_data_rfprofile) && domainType == DomainType::WwanRfim)
	{
		m_overrideRfProfileDataSet = RfProfileDataSet::createRfProfileDataFromDptfBuffer(rfProfileBufferData);
		auto guardband = getRfProfileGuardband(participantIndex, domainIndex) / 2;
		auto rfProfileData = m_overrideRfProfileDataSet.getRfProfileData();
		std::vector<RfProfileData> newRfProfileDataSet;
		for (auto rfData = rfProfileData.begin(); rfData != rfProfileData.end(); rfData++)
		{
			RfProfileData newRfProfileData(
				rfData->is5G(),
				rfData->getServingCellInfo(),
				rfData->getCenterFrequency(),
				rfData->getLeftFrequencySpread(),
				rfData->getRightFrequencySpread(),
				guardband,
				rfData->getChannelNumber(),
				rfData->getBand(),
				rfData->getSupplementalData());
			newRfProfileDataSet.insert(newRfProfileDataSet.end(), newRfProfileData);
		}
		m_overrideRfProfileDataSet = RfProfileDataSet(newRfProfileDataSet);
	}
}

RfProfileDataSet DomainRfProfileStatus_002::getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (m_overrideRfProfileDataSet.getRfProfileData().size() != 0)
		{
			m_rfProfileDataSet = m_overrideRfProfileDataSet; 
		}
		else 
		{
			DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
				esif_primitive_type::GET_RF_CHANNEL_INFO, ESIF_DATA_BINARY, domainIndex);
			m_rfProfileDataSet = RfProfileDataSet::createRfProfileDataFromDptfBuffer(buffer);	
			auto guardband = getRfProfileGuardband(participantIndex, domainIndex) / 2;
			auto rfProfileData = m_rfProfileDataSet.getRfProfileData();
			std::vector<RfProfileData> newRfProfileDataSet;
			for (auto rfData = rfProfileData.begin(); rfData != rfProfileData.end(); rfData++)
			{
				RfProfileData newRfProfileData(
					rfData->is5G(),
					rfData->getServingCellInfo(),
					rfData->getCenterFrequency(),
					rfData->getLeftFrequencySpread(),
					rfData->getRightFrequencySpread(),
					guardband,
					rfData->getChannelNumber(),
					rfData->getBand(),
					rfData->getSupplementalData());
				newRfProfileDataSet.insert(newRfProfileDataSet.end(), newRfProfileData);
			}
			m_rfProfileDataSet = RfProfileDataSet(newRfProfileDataSet);
		}	
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to get Rf Channel Info. "; });
	}

	return m_rfProfileDataSet;
}

UInt32 DomainRfProfileStatus_002::getWifiCapabilities(UIntN participantIndex, UIntN domainIndex)
{
	UInt32 wifiCapabilities = Constants::Invalid;

	try
	{
		wifiCapabilities = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_WIFI_CAPABILITIES, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to get Wifi Capabilities Info. "; });
	}

	return wifiCapabilities;
}

UInt32 DomainRfProfileStatus_002::getRfiDisable(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt64 DomainRfProfileStatus_002::getDvfsPoints(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainRfProfileStatus_002::setDdrRfiTable(
	UIntN participantIndex,
	UIntN domainIndex,
	DdrfChannelBandPackage::WifiRfiDdr ddrRfiStruct)
{
	try
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return (
				"Setting the DDR RFI table for a total of "
				+ StatusFormat::friendlyValue(ddrRfiStruct.numberOfDvfsPoints) + " dvfs points");
		});

		getParticipantServices()->primitiveExecuteSet(
			SET_DDR_RFI_TABLE, ESIF_DATA_STRUCTURE, &ddrRfiStruct, sizeof(ddrRfiStruct), sizeof(ddrRfiStruct));
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to set DDR RFI table. "; });
	}
}

void DomainRfProfileStatus_002::setProtectRequest(UIntN participantIndex, UIntN domainIndex, UInt64 frequencyRate)
{
	throw not_implemented();
}

Frequency DomainRfProfileStatus_002::getRfProfileGuardband(UIntN participantIndex, UIntN domainIndex)
{
	Frequency rfProfileGuardband(0);
	try
	{
		auto domainType = getParticipantServices()->getDomainType(domainIndex);
		if (domainType == DomainType::Wireless)
		{
			auto centerFrequency = m_rfProfileDataSet.getRfProfileData().front().getCenterFrequency();
			if (centerFrequency < Frequency(5000000000))
			{
				rfProfileGuardband = getParticipantServices()->primitiveExecuteGetAsFrequency(
					esif_primitive_type::GET_RFPROFILE_WIFI24_FREQ_GUARD_BAND, domainIndex);
			}
			else
			{
				rfProfileGuardband = getParticipantServices()->primitiveExecuteGetAsFrequency(
					esif_primitive_type::GET_RFPROFILE_WIFI5_FREQ_GUARD_BAND, domainIndex);
			}
		}
		else
		{
			rfProfileGuardband = getParticipantServices()->primitiveExecuteGetAsFrequency(
				esif_primitive_type::GET_RFPROFILE_WWAN_FREQ_GUARD_BAND, domainIndex);
		}
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to get Rf Profile Guardband. "; });
	}
	return rfProfileGuardband;
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
			initializeRfProfileData(&capability);

			auto rfProfileDataSet = getRfProfileDataSet(participantIndex, domainIndex).getRfProfileData();
			auto domainType = getParticipantServices()->getDomainType(domainIndex);
			UInt32 channelNumber = 0;

			for (auto rfProfileData = rfProfileDataSet.begin(); rfProfileData != rfProfileDataSet.end();
				 rfProfileData++)
			{
				/* WWAN can have five or less channels with info and Wireless only has one channel with info */
				if ((domainType == DomainType::WwanRfim) || (domainType == DomainType::Wireless && channelNumber == 0))
				{
					capability.data.rfProfileStatus.rfProfileFrequencyData[channelNumber].centerFrequency =
						(UInt32)rfProfileData->getCenterFrequency();
					capability.data.rfProfileStatus.rfProfileFrequencyData[channelNumber].leftFrequencySpread =
						(UInt32)rfProfileData->getLeftFrequencySpreadWithGuardband();
					capability.data.rfProfileStatus.rfProfileFrequencyData[channelNumber].rightFrequencySpread =
						(UInt32)rfProfileData->getRightFrequencySpreadWithGuardband();
					channelNumber++;
				}
			}

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

void DomainRfProfileStatus_002::onClearCachedData(void)
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
