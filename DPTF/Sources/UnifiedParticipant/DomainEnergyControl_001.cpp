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

#include "DomainEnergyControl_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"
#include <cmath>
#include "EsifTime.h"
using namespace StatusFormat;

DomainEnergyControl_001::DomainEnergyControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainEnergyControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_raplEnergyUnit(0.0)
{
}

DomainEnergyControl_001::~DomainEnergyControl_001(void)
{
}

UInt32 DomainEnergyControl_001::getRaplEnergyCounter(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_RAPL_ENERGY, domainIndex, Constants::Esif::NoInstance);
}

EnergyCounterInfo DomainEnergyControl_001::getRaplEnergyCounterInfo(UIntN participantIndex, UIntN domainIndex)
{
	if (m_raplEnergyUnit == 0.0)
	{
		getRaplEnergyUnit(participantIndex, domainIndex);
	}

	try
	{
		double raplEnergyCounter = static_cast<double>(getRaplEnergyCounter(participantIndex, domainIndex));
		raplEnergyCounter = raplEnergyCounter * m_raplEnergyUnit;

		TimeSpan timestamp = EsifTime().getTimeStamp();

		return EnergyCounterInfo(raplEnergyCounter, timestamp.asMicroseconds());
	}
	catch (...)
	{
		return EnergyCounterInfo();
	}
}

double DomainEnergyControl_001::getRaplEnergyUnit(UIntN participantIndex, UIntN domainIndex)
{
	auto raplEnergyUnit = getParticipantServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_RAPL_ENERGY_UNIT, domainIndex, Constants::Esif::NoInstance);
	
	m_raplEnergyUnit = (1 / pow(2, raplEnergyUnit));
	return m_raplEnergyUnit;
}

UInt32 DomainEnergyControl_001::getRaplEnergyCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_RAPL_ENERGY_COUNTER_WIDTH, domainIndex, Constants::Esif::NoInstance);
}

Power DomainEnergyControl_001::getInstantaneousPower(UIntN participantIndex, UIntN domainIndex)
{
	auto instantaneousPower = Power::createInvalid();
	try
	{
		instantaneousPower = getParticipantServices()->primitiveExecuteGetAsPower(
			esif_primitive_type::GET_INSTANTANEOUS_POWER, domainIndex);
	}
	catch (primitive_not_found_in_dsp&)
	{
		PARTICIPANT_LOG_MESSAGE_INFO({ return "Participant does not support the get instantaneous power primitive"; });
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_INFO({ return "Failed to get instantaneous power"; });
	}

	return instantaneousPower;
}

UInt32 DomainEnergyControl_001::getEnergyThreshold(UIntN participantIndex, UIntN domainIndex)
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

void DomainEnergyControl_001::setEnergyThreshold(UIntN participantIndex, UIntN domainIndex, UInt32 energyThreshold)
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

void DomainEnergyControl_001::setEnergyThresholdInterruptDisable(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		getParticipantServices()->primitiveExecuteSet(
			esif_primitive_type::SET_ENERGY_THRESHOLD_INT_DISABLE,
			esif_data_type::ESIF_DATA_VOID,
			nullptr,
			0,
			0,
			domainIndex,
			Constants::Esif::NoInstance);
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

void DomainEnergyControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_ENERGY_CONTROL;
			capability.size = sizeof(capability);

			try
			{
				capability.data.energyControl.energyCounter = getRaplEnergyCounter(participantIndex, domainIndex);
				capability.data.energyControl.instantaneousPower =
					(UInt32)getInstantaneousPower(participantIndex, domainIndex);
			}
			catch (dptf_exception& ex)
			{
				PARTICIPANT_LOG_MESSAGE_DEBUG_EX({ return ex.getDescription(); });

				capability.data.energyControl.energyCounter = 0;
				capability.data.energyControl.instantaneousPower = 0;
			}

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));

			PARTICIPANT_LOG_MESSAGE_INFO({
				std::stringstream message;
				message << "Published activity for participant " << getParticipantIndex() << ", "
						<< "domain " << getName() << " "
						<< "("
						<< "Energy Control"
						<< ")";
				return message.str();
			});
		}
	}
	catch (...)
	{
		// skip if there are any issue in sending log data
	}
}

void DomainEnergyControl_001::onClearCachedData(void)
{
	// do nothing
}

std::shared_ptr<XmlNode> DomainEnergyControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("energy_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));
	return root;
}

std::string DomainEnergyControl_001::getName(void)
{
	return "Energy Control";
}
