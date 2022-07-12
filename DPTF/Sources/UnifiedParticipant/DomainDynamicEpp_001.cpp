/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "DomainDynamicEpp_001.h"
#include "XmlNode.h"
#include "MbtHint.h"

DomainDynamicEpp_001::DomainDynamicEpp_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainDynamicEppBase(participantIndex, domainIndex, participantServicesInterface)
	, m_eppHint(Constants::Invalid)
{
	checkDynamicEppSupport();
}

DomainDynamicEpp_001::~DomainDynamicEpp_001()
{
}

void DomainDynamicEpp_001::checkDynamicEppSupport()
{
	try
	{
		m_isDynamicEppSupported = (getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_SUPPORT_EPP_HINT, getDomainIndex())
			== 1)
			? true
			: false;
	}
	catch (dptf_exception& ex)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING_EX({ return "Failed to get Dynamic EPP support. " + ex.getDescription(); });
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_DEBUG({ return "Failed to get Dynamic EPP support."; });
	}
}

UInt32 DomainDynamicEpp_001::getEppSensitivityHint()
{
	m_eppHint = Constants::Invalid;

	try
	{
		m_eppHint = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_EPP_SENSITIVITY_HINT_MODEL, getDomainIndex());
	}
	catch (...)
	{
	}

	return m_eppHint;
}

void DomainDynamicEpp_001::updateEppSensitivityHint(UInt32 eppSensitivityHint)
{
	m_eppHint = eppSensitivityHint;
}

void DomainDynamicEpp_001::setDynamicEppSupport(UInt32 dynamicEppSupport)
{
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_SUPPORT_EPP_HINT, dynamicEppSupport, getDomainIndex());
	m_isDynamicEppSupported = dynamicEppSupport;
}

std::string DomainDynamicEpp_001::getName(void)
{
	return "Dynamic EPP";
}

std::shared_ptr<XmlNode> DomainDynamicEpp_001::getXml(UIntN domainIndex)
{
	std::shared_ptr<XmlNode> root = XmlNode::createRoot();

	if (m_isDynamicEppSupported)
	{
		std::shared_ptr<XmlNode> node = XmlNode::createWrapperElement("dynamic_epp");
		node->addChild(XmlNode::createDataElement("control_name", getName()));
		node->addChild(XmlNode::createDataElement("control_knob_version", "001"));

		try
		{
			node->addChild(XmlNode::createDataElement(
				"value",
				MbtHint::toString((MbtHint::Type)m_eppHint)));
		}
		catch (...)
		{
			node->addChild(XmlNode::createDataElement("value", Constants::InvalidString));
		}

		root->addChild(node);
	}

	return root;
}