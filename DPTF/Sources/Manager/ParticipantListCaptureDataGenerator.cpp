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
#include "ParticipantListCaptureDataGenerator.h"
#include "ParticipantManagerInterface.h"
#include "TemperatureThresholds.h"
using namespace std;

ParticipantListCaptureDataGenerator::ParticipantListCaptureDataGenerator(DptfManagerInterface* dptfManager)
	: CaptureDataGenerator(dptfManager)
	, m_participantManager(dptfManager->getParticipantManager())
{
}

/*
Example:
<participants>
	<participant>
		<name>IETM</name>
	</participant>
	...
</participants>
*/
shared_ptr<XmlNode> ParticipantListCaptureDataGenerator::generate() const
{
	auto root = XmlNode::createRoot();
	const auto participantIds = m_participantManager->getParticipantIndexes();
	const auto participantsRoot = XmlNode::createWrapperElement("participants");
	addIetmParticipantName(participantsRoot);
	for (const auto participantId : participantIds)
	{
		addParticipantName(participantId, participantsRoot);
	}
	root->addChild(participantsRoot);
	return root;
}

void ParticipantListCaptureDataGenerator::addIetmParticipantName(const shared_ptr<XmlNode>& root) const
{
	try
	{
		const auto participantRoot = XmlNode::createWrapperElement("participant");
		participantRoot->addChild(XmlNode::createDataElement("name"s, "IETM"s));
		root->addChild(participantRoot);
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

void ParticipantListCaptureDataGenerator::addParticipantName(UIntN participantId, const shared_ptr<XmlNode>& root)
	const
{
	try
	{
		const auto participant = m_participantManager->getParticipantPtr(participantId);
		const auto name = participant->getParticipantName();
		const auto participantRoot = XmlNode::createWrapperElement("participant");
		participantRoot->addChild(XmlNode::createDataElement("name"s, name));
		root->addChild(participantRoot);
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}