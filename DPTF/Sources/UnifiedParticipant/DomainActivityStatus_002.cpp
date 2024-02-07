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

#include "DomainActivityStatus_002.h"
#include "XmlNode.h"
#include "esif_ccb.h"

DomainActivityStatus_002::DomainActivityStatus_002(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainActivityStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainActivityStatus_002::~DomainActivityStatus_002()
{
}

Percentage DomainActivityStatus_002::getUtilizationThreshold(UIntN participantIndex, UIntN domainIndex)
{
	return getParticipantServices()->primitiveExecuteGetAsPercentage(
		esif_primitive_type::GET_PARTICIPANT_UTILIZATION_THRESHOLD, domainIndex);
}

Percentage DomainActivityStatus_002::getResidencyUtilization(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt64 DomainActivityStatus_002::getCoreActivityCounter(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainActivityStatus_002::getCoreActivityCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt64 DomainActivityStatus_002::getTimestampCounter(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

UInt32 DomainActivityStatus_002::getTimestampCounterWidth(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

CoreActivityInfo DomainActivityStatus_002::getCoreActivityInfo(UIntN participantIndex, UIntN domainIndex)
{
	throw not_implemented();
}

void DomainActivityStatus_002::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	// do nothing
}

void DomainActivityStatus_002::onClearCachedData(void)
{
	// do nothing
}

std::shared_ptr<XmlNode> DomainActivityStatus_002::getXml(UIntN domainIndex)
{
	std::shared_ptr<XmlNode> root = XmlNode::createWrapperElement("activity_status");
	root->addChild(XmlNode::createDataElement("control_name", getName()));
	root->addChild(XmlNode::createDataElement("control_knob_version", "002"));

	return root;
}

std::string DomainActivityStatus_002::getName(void)
{
	return "Activity Status";
}

