/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "DomainPlatformPowerControl_001.h"
#include "XmlNode.h"
#include <esif_sdk_data_misc.h>

DomainPlatformPowerControl_001::DomainPlatformPowerControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPlatformPowerControlBase(participantIndex, domainIndex, participantServicesInterface)
{
	onClearCachedData();
	capture();
}

DomainPlatformPowerControl_001::~DomainPlatformPowerControl_001(void)
{
	restore();
}

void DomainPlatformPowerControl_001::setPortPowerLimit(const UInt32 portNumber, const Power& powerLimit)
{
	esif_data_complex_usbc_power_limit usbcPowerLimitStructure;
	usbcPowerLimitStructure.port = portNumber;
	// if powerLimit is invalid, it means that all policies removed their requests,
	// we still want to set the power to 0 (when Power is invalid the value is 0) which allows PPM to set what it wants
	usbcPowerLimitStructure.power_limit = (UInt32)powerLimit;

	getParticipantServices()->primitiveExecuteSet(
		esif_primitive_type::SET_USBC_POWER_LIMIT,
		ESIF_DATA_STRUCTURE,
		&usbcPowerLimitStructure,
		sizeof(esif_data_complex_usbc_power_limit),
		sizeof(esif_data_complex_usbc_power_limit),
		getDomainIndex(),
		Constants::Esif::NoInstance);

	m_lastSetPowerLimits[portNumber] = powerLimit;
}

void DomainPlatformPowerControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_PLAT_POWER_CONTROL;
			capability.size = sizeof(capability);

			for (UInt32 portNumber = 0; portNumber < MAX_PORT_NUMBER; portNumber++)
			{
				try
				{
					capability.data.platformPowerControl.portDataSet[portNumber] =
						(UInt32)getPortPowerLimit(portNumber + 1);
				}
				catch (dptf_exception& ex)
				{
					// Make the variable appear as used for Chromium Klocwork.
					PARTICIPANT_LOG_MESSAGE_DEBUG_EX({ return ex.getDescription(); });

					capability.data.platformPowerControl.portDataSet[portNumber] = 0;
				}
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
						<< "Platform Power Control"
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

void DomainPlatformPowerControl_001::onClearCachedData(void)
{
	// do not clear last set values since we need to be able to restore the ones that were modified
}

std::shared_ptr<XmlNode> DomainPlatformPowerControl_001::getXml(UIntN domainIndex)
{
	auto root = XmlNode::createWrapperElement("platform_power_control");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));
	root->addChild(getXmlForAllPorts());
	return root;
}

Power DomainPlatformPowerControl_001::getPortPowerLimit(UInt32 portNumber)
{
	auto port = m_lastSetPowerLimits.find(portNumber);
	if (port != m_lastSetPowerLimits.end())
	{
		return m_lastSetPowerLimits[portNumber];
	}

	return Power::createInvalid();
}

std::shared_ptr<XmlNode> DomainPlatformPowerControl_001::getXmlForAllPorts()
{
	auto set = XmlNode::createWrapperElement("port_status_set");
	for (auto portNumber = 1; portNumber <= MAX_PORT_NUMBER; ++portNumber)
	{
		auto portStatus = XmlNode::createWrapperElement("port_status");
		portStatus->addChild(XmlNode::createDataElement("port_number", std::to_string(portNumber)));
		if (m_lastSetPowerLimits.find(portNumber) != m_lastSetPowerLimits.end())
		{
			portStatus->addChild(
				XmlNode::createDataElement("limit_value", m_lastSetPowerLimits[portNumber].toString()));
		}
		else
		{
			portStatus->addChild(XmlNode::createDataElement("limit_value", Constants::InvalidString));
		}
		set->addChild(portStatus);
	}
	return set;
}

std::shared_ptr<XmlNode> DomainPlatformPowerControl_001::getArbitratorXml(UIntN policyIndex) const
{
	return m_arbitrator.getStatusForPolicy(policyIndex);
}

void DomainPlatformPowerControl_001::capture(void)
{
	// do nothing since there is no GET primitive
}

void DomainPlatformPowerControl_001::restore(void)
{
	for (auto portNumber = m_lastSetPowerLimits.begin(); portNumber != m_lastSetPowerLimits.end(); ++portNumber)
	{
		setPortPowerLimit(portNumber->first, Power::createInvalid());
	}
}

std::string DomainPlatformPowerControl_001::getName(void)
{
	return "Platform Power Control";
}
