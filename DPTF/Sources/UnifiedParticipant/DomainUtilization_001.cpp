/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "DomainUtilization_001.h"
#include "XmlNode.h"

DomainUtilization_001::DomainUtilization_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainUtilizationBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainUtilization_001::~DomainUtilization_001()
{
}

UtilizationStatus DomainUtilization_001::getUtilizationStatus(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		Percentage utilization = getParticipantServices()->primitiveExecuteGetAsPercentage(
			esif_primitive_type::GET_PARTICIPANT_UTILIZATION, domainIndex);
		return UtilizationStatus(utilization);
	}
	catch (...)
	{
		return UtilizationStatus(Percentage(1.0));
	}
}

Percentage DomainUtilization_001::getMaxCoreUtilization(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsPercentage(
		esif_primitive_type::GET_PARTICIPANT_MAX_CORE_UTILIZATION, domainIndex);
}

void DomainUtilization_001::onClearCachedData(void)
{
	// Do nothing.  We don't cache domain utilization related data.
}

std::shared_ptr<XmlNode> DomainUtilization_001::getXml(UIntN domainIndex)
{
	auto root = getUtilizationStatus(getParticipantIndex(), domainIndex).getXml("utilization");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));
	return root;
}

std::string DomainUtilization_001::getName(void)
{
	return "Utilization Status";
}
