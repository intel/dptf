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

#include "DomainRfProfileControl_001.h"

DomainRfProfileControl_001::DomainRfProfileControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainRfProfileControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	// TODO: does this need to capture or restore anything?
}

DomainRfProfileControl_001::~DomainRfProfileControl_001(void)
{
}

RfProfileCapabilities DomainRfProfileControl_001::getRfProfileCapabilities(UIntN participantIndex, UIntN domainIndex)
{

	Frequency defaultCenterFrequency(0);
	Frequency centerFrequency(0);
	Frequency frequencyAdjustResolution(0);
	Frequency minFrequency(0);
	Frequency maxFrequency(0);
	Percentage ssc(0.0);

	try
	{
		defaultCenterFrequency = getParticipantServices()->primitiveExecuteGetAsFrequency(
			esif_primitive_type::GET_RFPROFILE_DEFAULT_CENTER_FREQUENCY, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return "Failed to get default center frequency. ";
			});
	}
	try
	{
		centerFrequency = getParticipantServices()->primitiveExecuteGetAsFrequency(
			esif_primitive_type::GET_RFPROFILE_CENTER_FREQUENCY, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return "Failed to get center frequency. ";
			});
	}
	try
	{
		frequencyAdjustResolution = getParticipantServices()->primitiveExecuteGetAsFrequency(
			esif_primitive_type::GET_RFPROFILE_FREQUENCY_ADJUST_RESOLUTION, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return "Failed to get frequency adjust resolution. ";
			});
	}
	try
	{
		minFrequency = getParticipantServices()->primitiveExecuteGetAsFrequency(
			esif_primitive_type::GET_RFPROFILE_MIN_FREQUENCY, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return "Failed to get min frequency. ";
			});
	}
	try
	{
		maxFrequency = getParticipantServices()->primitiveExecuteGetAsFrequency(
			esif_primitive_type::GET_RFPROFILE_MAX_FREQUENCY, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return "Failed to get max frequency. ";
			});
	}
	try
	{
		ssc = getParticipantServices()->primitiveExecuteGetAsPercentage(
			esif_primitive_type::GET_RFPROFILE_SSC, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			return "Failed to get ssc. ";
			});
	}

	RfProfileCapabilities rfProfileCapabilities(
		defaultCenterFrequency,
		centerFrequency,
		frequencyAdjustResolution,
		minFrequency,
		maxFrequency,
		ssc);

	return rfProfileCapabilities;
}

void DomainRfProfileControl_001::setRfProfileCenterFrequency(
	UIntN participantIndex,
	UIntN domainIndex,
	const Frequency& centerFrequency)
{
	getParticipantServices()->primitiveExecuteSetAsFrequency(
		esif_primitive_type::SET_RFPROFILE_CENTER_FREQUENCY, centerFrequency, domainIndex, Constants::Esif::NoInstance);
}

void DomainRfProfileControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_RFPROFILE_CONTROL;
			capability.size = sizeof(capability);
			capability.data.rfProfileControl.rfProfileMinFrequency =
				(UInt32)getRfProfileCapabilities(participantIndex, domainIndex).getMinFrequency();
			capability.data.rfProfileControl.rfProfileCenterFrequency =
				(UInt32)getRfProfileCapabilities(participantIndex, domainIndex).getCenterFrequency();
			capability.data.rfProfileControl.rfProfileMaxFrequency =
				(UInt32)getRfProfileCapabilities(participantIndex, domainIndex).getMaxFrequency();
			capability.data.rfProfileControl.rfProfileSSC =
				(UInt32)getRfProfileCapabilities(participantIndex, domainIndex).getSsc();

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

void DomainRfProfileControl_001::onClearCachedData(void)
{
	// FIXME: do we clear the cache for this control?
}

std::shared_ptr<XmlNode> DomainRfProfileControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("rfprofile_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	return root;
}

std::string DomainRfProfileControl_001::getName(void)
{
	return "RF Profile Control";
}

Percentage DomainRfProfileControl_001::getSscBaselineSpreadValue(UIntN participantIndex, UIntN domainIndex)
{
	Percentage sscBaselineSpreadValue(Percentage::createInvalid());

	try
	{
		sscBaselineSpreadValue = getParticipantServices()->primitiveExecuteGetAsPercentage(
			esif_primitive_type::GET_BASELINE_SSC_SPREAD_VALUE, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to get ssc baseline spread value. "; });
	}

	return sscBaselineSpreadValue;
}

Percentage DomainRfProfileControl_001::getSscBaselineThreshold(UIntN participantIndex, UIntN domainIndex)
{
	Percentage sscBaselineThreshold(Percentage::createInvalid());

	try
	{
		sscBaselineThreshold = getParticipantServices()->primitiveExecuteGetAsPercentage(
			esif_primitive_type::GET_BASELINE_SSC_THRESHOLD, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to get ssc baseline threshold. "; });
	}

	return sscBaselineThreshold;
}

Percentage DomainRfProfileControl_001::getSscBaselineGuardBand(UIntN participantIndex, UIntN domainIndex)
{
	Percentage sscBaselineGuardBand(Percentage::createInvalid());

	try
	{
		sscBaselineGuardBand = getParticipantServices()->primitiveExecuteGetAsPercentage(
			esif_primitive_type::GET_BASELINE_SSC_GUARD_BAND, domainIndex);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to get ssc baseline guard band. "; });
	}

	return sscBaselineGuardBand;
}