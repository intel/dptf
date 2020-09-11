/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "DomainPeakPowerControl_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

DomainPeakPowerControl_001::DomainPeakPowerControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPeakPowerControlBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainPeakPowerControl_001::~DomainPeakPowerControl_001(void)
{
}

Power DomainPeakPowerControl_001::getACPeakPower(UIntN participantIndex, UIntN domainIndex)
{
	m_acPeakPower.set(
		getParticipantServices()->primitiveExecuteGetAsPower(esif_primitive_type::GET_AC_PEAK_POWER, domainIndex));
	return m_acPeakPower.get();
}

void DomainPeakPowerControl_001::setACPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& acPeakPower)
{
	throwIfInvalidPower(acPeakPower);
	getParticipantServices()->primitiveExecuteSetAsPower(
		esif_primitive_type::SET_AC_PEAK_POWER, acPeakPower, domainIndex);
	m_acPeakPower.invalidate();
}

Power DomainPeakPowerControl_001::getDCPeakPower(UIntN participantIndex, UIntN domainIndex)
{
	m_dcPeakPower.set(
		getParticipantServices()->primitiveExecuteGetAsPower(esif_primitive_type::GET_DC_PEAK_POWER, domainIndex));
	return m_dcPeakPower.get();
}

void DomainPeakPowerControl_001::setDCPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& dcPeakPower)
{
	throwIfInvalidPower(dcPeakPower);
	getParticipantServices()->primitiveExecuteSetAsPower(
		esif_primitive_type::SET_DC_PEAK_POWER, dcPeakPower, domainIndex);
	m_dcPeakPower.invalidate();
}

void DomainPeakPowerControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_PEAK_POWER_CONTROL;
			capability.size = sizeof(capability);
			capability.data.peakPowerControl.acPeakPower = 0;
			capability.data.peakPowerControl.dcPeakPower = 0;

			try
			{
				capability.data.peakPowerControl.acPeakPower = (UInt32)getACPeakPower(participantIndex, domainIndex);
			}
			catch (const std::exception& ex)
			{
				PARTICIPANT_LOG_MESSAGE_DEBUG_EX({
					std::stringstream message;
					message << "Failed to get PL4 AC Power: " << ex.what();
					return message.str();
				});
			}

			try
			{
				capability.data.peakPowerControl.dcPeakPower = (UInt32)getDCPeakPower(participantIndex, domainIndex);
			}
			catch (const std::exception& ex)
			{
				PARTICIPANT_LOG_MESSAGE_DEBUG_EX({
					std::stringstream message;
					message << "Failed to get PL4 DC Power: " << ex.what();
					return message.str();
				});
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
						<< "Peak Power Control"
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

void DomainPeakPowerControl_001::onClearCachedData(void)
{
	m_acPeakPower.invalidate();
	m_dcPeakPower.invalidate();
}

std::string DomainPeakPowerControl_001::getName(void)
{
	return "Peak Power Control";
}

std::shared_ptr<XmlNode> DomainPeakPowerControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("peak_power_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	auto acPeakPower = XmlNode::createWrapperElement("peak_power_control_object");
	acPeakPower->addChild(XmlNode::createDataElement("name", "PL4 AC Power"));
	try
	{
		acPeakPower->addChild(
			XmlNode::createDataElement("value", getACPeakPower(getParticipantIndex(), domainIndex).toString() + " mW"));
	}
	catch (...)
	{
		acPeakPower->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(acPeakPower);

	auto dcPeakPower = XmlNode::createWrapperElement("peak_power_control_object");
	dcPeakPower->addChild(XmlNode::createDataElement("name", "PL4 DC Power"));
	try
	{
		dcPeakPower->addChild(
			XmlNode::createDataElement("value", getDCPeakPower(getParticipantIndex(), domainIndex).toString() + " mW"));
	}
	catch (...)
	{
		dcPeakPower->addChild(XmlNode::createDataElement("value", "Error"));
	}
	root->addChild(dcPeakPower);

	return root;
}

void DomainPeakPowerControl_001::throwIfInvalidPower(const Power& power)
{
	if (!power.isValid())
	{
		throw dptf_exception("Attempting to set an invalid power value for Peak Power Control");
	}
}
