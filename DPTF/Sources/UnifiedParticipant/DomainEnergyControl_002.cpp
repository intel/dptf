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

#include "DomainEnergyControl_002.h"
#include "XmlNode.h"
#include "StatusFormat.h"
#include <cmath>
#include "EsifTime.h"
using namespace StatusFormat;

DomainEnergyControl_002::DomainEnergyControl_002(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainEnergyControlBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainEnergyControl_002::~DomainEnergyControl_002(void)
{
}

UInt32 DomainEnergyControl_002::getRaplEnergyCounter(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

EnergyCounterInfo DomainEnergyControl_002::getRaplEnergyCounterInfo(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

double DomainEnergyControl_002::getRaplEnergyUnit(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainEnergyControl_002::getRaplEnergyCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

Power DomainEnergyControl_002::getInstantaneousPower(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainEnergyControl_002::getEnergyThreshold(UIntN participantIndex, UIntN domainIndex)
{
	auto energyThreshold = 0;

	try
	{
		energyThreshold = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_PARTICIPANT_ENERGY_THRESHOLD, domainIndex);
	}
	catch (primitive_not_found_in_dsp&)
	{
		PARTICIPANT_LOG_MESSAGE_INFO({ return "Participant does not support the get energy threshold primitive"; });
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_INFO({ return "Failed to get energy threshold interrupt"; });
	}

	return energyThreshold;
}

void DomainEnergyControl_002::setEnergyThreshold(UIntN participantIndex, UIntN domainIndex, UInt32 energyThreshold)
{
	try
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_ENERGY_THRESHOLD_COUNT, energyThreshold, domainIndex);
	}
	catch (primitive_not_found_in_dsp&)
	{
		PARTICIPANT_LOG_MESSAGE_INFO({ return "Participant does not support the set energy threshold primitive"; });
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_INFO({ return "Failed to set energy threshold"; });
	}
}

void DomainEnergyControl_002::setEnergyThresholdInterruptDisable(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			esif_primitive_type::SET_ENERGY_THRESHOLD_COUNT, 0, domainIndex);
	}
	catch (primitive_not_found_in_dsp&)
	{
		PARTICIPANT_LOG_MESSAGE_INFO(
			{ return "Participant does not support the set energy threshold interrupt enable primitive"; });
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_INFO({ return "Failed to set energy threshold interrupt flag"; });
	}
}

void DomainEnergyControl_002::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainEnergyControl_002::onClearCachedData(void)
{
	// do nothing
}

std::shared_ptr<XmlNode> DomainEnergyControl_002::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("energy_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "002"));
	return root;
}

std::string DomainEnergyControl_002::getName(void)
{
	return "Energy Control";
}
