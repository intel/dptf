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

#include "DomainSocWorkloadClassification_001.h"
#include "XmlNode.h"
#include "SocWorkloadClassification.h"

DomainSocWorkloadClassification_001::DomainSocWorkloadClassification_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainSocWorkloadClassificationBase(participantIndex, domainIndex, participantServicesInterface)
	, m_socWorkload(Constants::Invalid)
{
}

DomainSocWorkloadClassification_001::~DomainSocWorkloadClassification_001()
{
}

UInt32 DomainSocWorkloadClassification_001::getSocWorkloadClassification()
{
	m_socWorkload = Constants::Invalid;

	try
	{
		m_socWorkload = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_SOC_WORKLOAD, getDomainIndex());
	}
	catch (...)
	{
	}

	return m_socWorkload;
}

void DomainSocWorkloadClassification_001::updateSocWorkloadClassification(UInt32 socWorkloadClassification)
{
	m_socWorkload = socWorkloadClassification;
}

std::string DomainSocWorkloadClassification_001::getName(void)
{
	return "Soc Workload Classification";
}

std::shared_ptr<XmlNode> DomainSocWorkloadClassification_001::getXml(UIntN domainIndex)
{
	std::shared_ptr<XmlNode> root = XmlNode::createRoot();
	std::shared_ptr<XmlNode> node = XmlNode::createWrapperElement("soc_workload");
	node->addChild(XmlNode::createDataElement("control_name", getName()));
	node->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	try
	{
		node->addChild(XmlNode::createDataElement(
			"value",
			SocWorkloadClassification::toString((SocWorkloadClassification::Type)m_socWorkload)));
	}
	catch (...)
	{
		node->addChild(XmlNode::createDataElement("value", Constants::InvalidString));
	}

	root->addChild(node);

	return root;
}