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

#include "DomainPowerStatus_002.h"
#include "XmlNode.h"
#include "esif_ccb.h"

DomainPowerStatus_002::DomainPowerStatus_002(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPowerStatusBase(participantIndex, domainIndex, participantServicesInterface)
	, m_lastCalculatedPower(Power::createInvalid())
{
}

DomainPowerStatus_002::~DomainPowerStatus_002()
{
}

PowerStatus DomainPowerStatus_002::getPowerStatus(UIntN participantIndex, UIntN domainIndex)
{
	return PowerStatus(m_lastCalculatedPower);
}

Power DomainPowerStatus_002::getAveragePower(
	UIntN participantIndex,
	UIntN domainIndex,
	const PowerControlDynamicCaps& capabilities)
{
	return m_lastCalculatedPower;
}

void DomainPowerStatus_002::setCalculatedAveragePower(UIntN participantIndex, UIntN domainIndex, Power powerValue)
{
	m_lastCalculatedPower = powerValue;
	getParticipantServices()->primitiveExecuteSetAsPower(
		esif_primitive_type::SET_RAPL_POWER, powerValue, domainIndex, Constants::Esif::NoInstance);
}

void DomainPowerStatus_002::clearCachedData(void)
{
	// do nothing
}

std::shared_ptr<XmlNode> DomainPowerStatus_002::getXml(UIntN domainIndex)
{
	std::shared_ptr<XmlNode> root = XmlNode::createWrapperElement("power_status_set");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(getPowerStatus(getParticipantIndex(), domainIndex).getXml());
	root->addChild(XmlNode::createDataElement("control_knob_version", "002"));

	return root;
}

void DomainPowerStatus_002::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	// do nothing
}

std::string DomainPowerStatus_002::getName(void)
{
	return "Power Share Status";
}
