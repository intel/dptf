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

#include "DomainPowerStatus_001.h"
#include "XmlNode.h"
#include "esif_ccb.h"

DomainPowerStatus_001::DomainPowerStatus_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPowerStatusBase(participantIndex, domainIndex, participantServicesInterface)
	, m_lastPowerSentToFilter(Power::createInvalid())
	, m_domainPowerFilter(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainPowerStatus_001::~DomainPowerStatus_001()
{
}

PowerStatus DomainPowerStatus_001::getPowerStatus(UIntN participantIndex, UIntN domainIndex)
{
	getPower(domainIndex);
	esif_ccb_sleep_msec(250);
	return PowerStatus(getPower(domainIndex));
}

Power DomainPowerStatus_001::getAveragePower(
	UIntN participantIndex,
	UIntN domainIndex,
	const PowerControlDynamicCaps& capabilities)
{
	m_lastPowerSentToFilter = getPowerStatus(participantIndex, domainIndex).getCurrentPower();
	Power averagePower = m_domainPowerFilter.getAveragePower(capabilities, m_lastPowerSentToFilter);

	return averagePower;
}

Power DomainPowerStatus_001::getPower(UIntN domainIndex)
{
	Power power = Power::createInvalid();

	try
	{
		power = getParticipantServices()->primitiveExecuteGetAsPower(esif_primitive_type::GET_RAPL_POWER, domainIndex);
	}
	catch (primitive_try_again)
	{
		power = getParticipantServices()->primitiveExecuteGetAsPower(esif_primitive_type::GET_RAPL_POWER, domainIndex);
	}

	return power;
}

void DomainPowerStatus_001::clearCachedData(void)
{
	m_domainPowerFilter.clearCachedData();
}

std::shared_ptr<XmlNode> DomainPowerStatus_001::getXml(UIntN domainIndex)
{
	std::shared_ptr<XmlNode> root = XmlNode::createWrapperElement("power_status_set");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(getPowerStatus(getParticipantIndex(), domainIndex).getXml());
	root->addChild(m_domainPowerFilter.getXml());
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	return root;
}

void DomainPowerStatus_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_POWER_STATUS;
			capability.size = sizeof(capability);
			capability.data.powerStatus.powerFilterData.currentPowerSentToFilter = m_lastPowerSentToFilter;
			capability.data.powerStatus.powerFilterData.powerCalculatedByFilter =
				m_domainPowerFilter.getLastPowerUsed(PowerControlType::PL1);

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));

			std::stringstream message;
			message << "Published activity for participant " << getParticipantIndex() << ", "
					<< "domain " << getName() << " "
					<< "("
					<< "Power Status"
					<< ")";
			getParticipantServices()->writeMessageInfo(ParticipantMessage(FLF, message.str()));
		}
	}
	catch (...)
	{
		// skip if there are any issue in sending log data
	}
}

std::string DomainPowerStatus_001::getName(void)
{
	return "Power Status";
}
