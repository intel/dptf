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

#include "DomainPriority_001.h"
#include "XmlNode.h"

DomainPriority_001::DomainPriority_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainPriorityBase(participantIndex, domainIndex, participantServicesInterface)
{
	onClearCachedData();
}

DomainPriority_001::~DomainPriority_001()
{
}

DomainPriority DomainPriority_001::getDomainPriority(UIntN participantIndex, UIntN domainIndex)
{
	updateCacheIfCleared(participantIndex, domainIndex);
	return m_currentPriority;
}

void DomainPriority_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_DOMAIN_PRIORITY;
			capability.size = sizeof(capability);
			if (m_cacheDataCleared == true)
			{
				capability.data.domainPriority.priority =
					getDomainPriority(getParticipantIndex(), domainIndex).getCurrentPriority();
			}
			else
			{
				capability.data.domainPriority.priority = m_currentPriority.getCurrentPriority();
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
						<< "Domain Priority"
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

void DomainPriority_001::onClearCachedData(void)
{
	m_currentPriority = DomainPriority(0); // set priority to 0 (low)
	m_cacheDataCleared = true;
}

std::shared_ptr<XmlNode> DomainPriority_001::getXml(UIntN domainIndex)
{
	std::shared_ptr<XmlNode> root = XmlNode::createWrapperElement("domain_priority");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(getDomainPriority(getParticipantIndex(), domainIndex).getXml());
	root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

	return root;
}

void DomainPriority_001::updateCacheIfCleared(UIntN participantIndex, UIntN domainIndex)
{
	if (m_cacheDataCleared == true)
	{
		updateCache(participantIndex, domainIndex);
	}
}

void DomainPriority_001::updateCache(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		UInt32 priority = getParticipantServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_DOMAIN_PRIORITY, domainIndex);

		m_currentPriority = DomainPriority(priority);
	}
	catch (...)
	{
		// if the primitive isn't available we keep the priority at 0
		m_currentPriority = DomainPriority(0);
	}

	m_cacheDataCleared = false;
	sendActivityLoggingDataIfEnabled(participantIndex, domainIndex);
}

std::string DomainPriority_001::getName(void)
{
	return "Domain Priority";
}
