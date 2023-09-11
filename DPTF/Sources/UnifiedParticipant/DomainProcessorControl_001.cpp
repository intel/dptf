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

#include "DomainProcessorControl_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

static const Temperature MinTccOffset = Temperature::fromCelsius(0.0);
static const Temperature MaxTccOffset = Temperature::fromCelsius(63.0);

DomainProcessorControl_001::DomainProcessorControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainProcessorControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	m_pcieThrottleRequestState.invalidate();
}

DomainProcessorControl_001::~DomainProcessorControl_001(void)
{
}

void DomainProcessorControl_001::updatePcieThrottleRequestState(UInt32 pcieThrottleRequestState)
{
	m_pcieThrottleRequestState.set(pcieThrottleRequestState);
}

Temperature DomainProcessorControl_001::getTccOffsetTemperature()
{
	return getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
		esif_primitive_type::GET_TCC_OFFSET, getDomainIndex());
}

void DomainProcessorControl_001::setTccOffsetTemperature(const Temperature& tccOffset)
{
	throwIfInvalidTemperature(tccOffset);
	getParticipantServices()->primitiveExecuteSetAsTemperatureTenthK(
		esif_primitive_type::SET_TCC_OFFSET, tccOffset, getDomainIndex());
}

void DomainProcessorControl_001::setPerfPreferenceMax(const Percentage& cpuMaxRatio)
{
	getParticipantServices()->primitiveExecuteSetAsPercentage(
		esif_primitive_type::SET_PERF_PREFERENCE_MAX, cpuMaxRatio, getDomainIndex());
	PARTICIPANT_LOG_MESSAGE_DEBUG(
		{
			std::stringstream message;
			message << "Successfully set max CPU frequency to " << std::to_string(cpuMaxRatio);
			return message.str();
		});
}

void DomainProcessorControl_001::setPerfPreferenceMin(const Percentage& cpuMinRatio)
{
	getParticipantServices()->primitiveExecuteSetAsPercentage(
		esif_primitive_type::SET_PERF_PREFERENCE_MIN, cpuMinRatio, getDomainIndex());
	PARTICIPANT_LOG_MESSAGE_DEBUG(
		{
			std::stringstream message;
			message << "Successfully set min CPU frequency to " << std::to_string(cpuMinRatio);
			return message.str();
		});
}

UInt32 DomainProcessorControl_001::getPcieThrottleRequestState()
{
	if (m_pcieThrottleRequestState.isInvalid())
	{
		try
		{
			UInt32 pcieThrottleRequestState = getParticipantServices()->primitiveExecuteGetAsUInt32(
				esif_primitive_type::GET_PCIE_THROTTLING_OPTION, getDomainIndex());
			m_pcieThrottleRequestState.set(pcieThrottleRequestState);
		}
		catch (...)
		{
			PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to get PCIe Throttle Request State"; });
			m_pcieThrottleRequestState.set((UInt32)0);
		}
	}

	return m_pcieThrottleRequestState.get();
}

Temperature DomainProcessorControl_001::getMaxTccOffsetTemperature()
{
	return MaxTccOffset;
}

Temperature DomainProcessorControl_001::getMinTccOffsetTemperature()
{
	return MinTccOffset;
}

void DomainProcessorControl_001::setUnderVoltageThreshold(const UInt32 voltageThreshold)
{
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_UNDER_VOLTAGE_THRESHOLD, voltageThreshold, getDomainIndex());
	m_uvth.set(voltageThreshold);
}

void DomainProcessorControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_PROCESSOR_CONTROL;
			capability.size = sizeof(capability);
			// tcc offset gets handled by ESIF
			if (m_uvth.isValid())
			{
				capability.data.processorControlStatus.uvth = m_uvth.get();
			}
			else
			{
				capability.data.processorControlStatus.uvth = 0;
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
						<< "Processor Control"
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

void DomainProcessorControl_001::onClearCachedData(void)
{
	m_uvth.invalidate();
}

std::string DomainProcessorControl_001::getName(void)
{
	return "Processor Control";
}

std::shared_ptr<XmlNode> DomainProcessorControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("processor_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));
	try
	{
		root->addChild(XmlNode::createDataElement("tcc_offset", getTccOffsetTemperature().toString() + " C"));
	}
	catch (...)
	{
		root->addChild(XmlNode::createDataElement("tcc_offset", "Error"));
	}

	root->addChild(XmlNode::createDataElement("min_tcc_offset", MinTccOffset.toString() + " C"));
	root->addChild(XmlNode::createDataElement("max_tcc_offset", MaxTccOffset.toString() + " C"));

	if (m_uvth.isValid())
	{
		root->addChild(XmlNode::createDataElement("last_set_uvth", friendlyValue(m_uvth.get()) + " mV"));
	}
	else
	{
		root->addChild(XmlNode::createDataElement("last_set_uvth", Constants::InvalidString + " mV"));
	}

	return root;
}

std::shared_ptr<XmlNode> DomainProcessorControl_001::getArbitratorXml(UIntN policyIndex) const
{
	return m_arbitrator.getStatusForPolicy(policyIndex);
}

void DomainProcessorControl_001::throwIfInvalidTemperature(const Temperature& temperature)
{
	if (!temperature.isValid() || temperature < MinTccOffset || temperature > MaxTccOffset)
	{
		throw dptf_exception("Attempting to set an invalid Temperature value for TCC Offset Control");
	}
}

