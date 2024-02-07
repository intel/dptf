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

#include "PassiveControlStatus.h"
using namespace std;

PassiveControlStatus::PassiveControlStatus(
	std::shared_ptr<ThermalRelationshipTable> trt,
	std::shared_ptr<ParticipantTrackerInterface> trackedParticipants)
{
	vector<UIntN> trackedParticipantIndicies = trackedParticipants->getAllTrackedIndexes();
	for (auto participant = trackedParticipantIndicies.begin(); participant != trackedParticipantIndicies.end();
		 participant++)
	{
		if (trt->isParticipantSourceDevice(*participant))
		{
			m_participantStatus.push_back(
				PassiveParticipantControlStatus(trackedParticipants->getParticipant(*participant)));
		}
	}
}

std::shared_ptr<XmlNode> PassiveControlStatus::getXml()
{
	auto controlStatus = XmlNode::createWrapperElement("passive_control_status");
	for (auto status = m_participantStatus.begin(); status != m_participantStatus.end(); status++)
	{
		controlStatus->addChild(status->getXml());
	}
	return controlStatus;
}
