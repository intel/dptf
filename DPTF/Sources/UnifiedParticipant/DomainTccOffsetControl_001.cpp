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

#include "DomainTccOffsetControl_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

static const Temperature MinTccOffset = Temperature::fromCelsius(0.0);
static const Temperature MaxTccOffset = Temperature::fromCelsius(63.0);

DomainTccOffsetControl_001::DomainTccOffsetControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainTccOffsetControlBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainTccOffsetControl_001::~DomainTccOffsetControl_001(void)
{
}

Temperature DomainTccOffsetControl_001::getTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex)
{
	m_tccOffset.set(
		getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(esif_primitive_type::GET_TCC_OFFSET, domainIndex));
	return m_tccOffset.get();
}

void DomainTccOffsetControl_001::setTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex, const Temperature& tccOffset)
{
	throwIfInvalidTemperature(tccOffset);
	getParticipantServices()->primitiveExecuteSetAsTemperatureTenthK(
		esif_primitive_type::SET_TCC_OFFSET, tccOffset, domainIndex);
	m_tccOffset.invalidate();
}

Temperature DomainTccOffsetControl_001::getMaxTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex)
{
	return MaxTccOffset;
}

Temperature DomainTccOffsetControl_001::getMinTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex)
{
	return MinTccOffset;
}

void DomainTccOffsetControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_TCC_CONTROL;
			capability.size = sizeof(capability);
			capability.data.tccOffsetStatus.tccOffset = 0;

			try
			{
				capability.data.tccOffsetStatus.tccOffset = (UInt32)getTccOffsetTemperature(participantIndex, domainIndex);
			}
			catch (const std::exception& ex)
			{
				std::stringstream message;
				message << "Failed to get TCC Offset Temperature: " << ex.what();
				getParticipantServices()->writeMessageDebug(ParticipantMessage(FLF, message.str()));
			}
		}
	}
	catch (...)
	{
		// skip if there are any issue in sending log data
	}
}

void DomainTccOffsetControl_001::clearCachedData(void)
{
	m_tccOffset.invalidate();
}

std::string DomainTccOffsetControl_001::getName(void)
{
	return "TCC Offset Control";
}

std::shared_ptr<XmlNode> DomainTccOffsetControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("tcc_offset_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));
	try
	{
		root->addChild(XmlNode::createDataElement("tcc_offset", getTccOffsetTemperature(getParticipantIndex(), domainIndex).toString() + " C"));
	}
	catch (...)
	{
		root->addChild(XmlNode::createDataElement("tcc_offset", "Error"));
	}

	root->addChild(XmlNode::createDataElement("min_tcc_offset", MinTccOffset.toString() + " C"));
	root->addChild(XmlNode::createDataElement("max_tcc_offset", MaxTccOffset.toString() + " C"));

	return root;
}

void DomainTccOffsetControl_001::throwIfInvalidTemperature(const Temperature& temperature)
{
	if (!temperature.isValid() || temperature < MinTccOffset || temperature > MaxTccOffset)
	{
		throw dptf_exception("Attempting to set an invalid Temperature value for TCC Offset Control");
	}
}
