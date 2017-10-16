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
		getParticipantServices()->writeMessageDebug(
			ParticipantMessage(FLF, "Failed to get default center frequency. "));
	}
	try
	{
		centerFrequency = getParticipantServices()->primitiveExecuteGetAsFrequency(
			esif_primitive_type::GET_RFPROFILE_CENTER_FREQUENCY, domainIndex);
	}
	catch (...)
	{
		getParticipantServices()->writeMessageDebug(
			ParticipantMessage(FLF, "Failed to get center frequency. "));
	}
	try
	{
		frequencyAdjustResolution = getParticipantServices()->primitiveExecuteGetAsFrequency(
			esif_primitive_type::GET_RFPROFILE_FREQUENCY_ADJUST_RESOLUTION, domainIndex);
	}
	catch (...)
	{
		getParticipantServices()->writeMessageDebug(
			ParticipantMessage(FLF, "Failed to get frequency adjust resolution. "));
	}
	try
	{
		minFrequency = getParticipantServices()->primitiveExecuteGetAsFrequency(
			esif_primitive_type::GET_RFPROFILE_MIN_FREQUENCY, domainIndex);
	}
	catch (...)
	{
		getParticipantServices()->writeMessageDebug(
			ParticipantMessage(FLF, "Failed to get min frequency. "));
	}
	try
	{
		maxFrequency = getParticipantServices()->primitiveExecuteGetAsFrequency(
			esif_primitive_type::GET_RFPROFILE_MAX_FREQUENCY, domainIndex);
	}
	catch (...)
	{
		getParticipantServices()->writeMessageDebug(
			ParticipantMessage(FLF, "Failed to get max frequency. "));
	}
	try
	{
		ssc = getParticipantServices()->primitiveExecuteGetAsPercentage(esif_primitive_type::GET_RFPROFILE_SSC, domainIndex);
	}
	catch (...)
	{
		getParticipantServices()->writeMessageDebug(
			ParticipantMessage(FLF, "Failed to get ssc. "));
	}
	RfProfileCapabilities rfProfileCapabilities(
		defaultCenterFrequency, centerFrequency, frequencyAdjustResolution, minFrequency, maxFrequency, ssc);

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

void DomainRfProfileControl_001::clearCachedData(void)
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