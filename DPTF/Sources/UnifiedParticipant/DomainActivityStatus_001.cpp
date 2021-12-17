/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "DomainActivityStatus_001.h"
#include "XmlNode.h"
#include "esif_ccb.h"

DomainActivityStatus_001::DomainActivityStatus_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainActivityStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainActivityStatus_001::~DomainActivityStatus_001()
{
}

Percentage DomainActivityStatus_001::getUtilizationThreshold(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsPercentage(
		esif_primitive_type::GET_PARTICIPANT_UTILIZATION_THRESHOLD, domainIndex);
}

Percentage DomainActivityStatus_001::getResidencyUtilization(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsPercentage(
		esif_primitive_type::GET_PARTICIPANT_RESIDENCY_UTILIZATION, domainIndex);
}

UInt64 DomainActivityStatus_001::getCoreActivityCounter(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsUInt64(
		esif_primitive_type::GET_CORE_ACTIVITY_COUNTER, domainIndex);
}

UInt32 DomainActivityStatus_001::getCoreActivityCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_ACTIVITY_COUNTER_WIDTH, domainIndex);
}

UInt64 DomainActivityStatus_001::getTimestampCounter(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsUInt64(
		esif_primitive_type::GET_PROC_TSC, domainIndex);
}

UInt32 DomainActivityStatus_001::getTimestampCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_TSC_WIDTH, domainIndex);
}

CoreActivityInfo DomainActivityStatus_001::getCoreActivityInfo(UIntN participantIndex, UIntN domainIndex)
{
	try 
	{
		UInt64 coreActivityCounter = getCoreActivityCounter(participantIndex, domainIndex);
		UInt64 timestampCounter = getTimestampCounter(participantIndex, domainIndex);
		return CoreActivityInfo(coreActivityCounter, timestampCounter);
	}
	catch (...)
	{
		return CoreActivityInfo();
	}
}

void DomainActivityStatus_001::setPowerShareEffectiveBias(
	UIntN participantIndex,
	UIntN domainIndex,
	UInt32 powerShareEffectiveBias)
{
	try
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_POWER_SHARE_EFFECTIVE_BIAS,
			powerShareEffectiveBias,
			domainIndex,
			Constants::Esif::NoPersistInstance);
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			std::stringstream message;
			message << "Failed to set Power Share Effective Bias for participant index = "
						   + std::to_string(participantIndex) + "and domain Index = " + std::to_string(domainIndex);
			return message.str();
		});
	}
}

void DomainActivityStatus_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	// do nothing
}

void DomainActivityStatus_001::onClearCachedData(void)
{
	// do nothing
}

std::shared_ptr<XmlNode> DomainActivityStatus_001::getXml(UIntN domainIndex)
{
	std::shared_ptr<XmlNode> root = XmlNode::createWrapperElement("activity_status");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	return root;
}

std::string DomainActivityStatus_001::getName(void)
{
	return "Activity Status";
}

