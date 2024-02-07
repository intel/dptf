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

#include "WIDptfNptWwanBandBroadcastReceived.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"
#include "StatusFormat.h"
#include "Participant.h"
#include "ParticipantManagerInterface.h"
#include "WIDomainRfProfileChanged.h"
#include "WorkItemQueueManagerInterface.h"

WIDptfNptWwanBandBroadcastReceived::WIDptfNptWwanBandBroadcastReceived(
	DptfManagerInterface* dptfManager,
	DptfBuffer nptWwanBandNotificationData)
	: WorkItem(dptfManager, FrameworkEvent::DptfAppBroadcastUnprivileged)
	, m_nptWwanBandNotificationData(nptWwanBandNotificationData)
{
}

WIDptfNptWwanBandBroadcastReceived::~WIDptfNptWwanBandBroadcastReceived(void)
{
}

void WIDptfNptWwanBandBroadcastReceived::onExecute(void)
{
	writeWorkItemStartingInfoMessage();
	auto participantManager = getParticipantManager();
	auto participantIndexes = participantManager->getParticipantIndexes();
	std::list<std::pair<UIntN, UIntN>> affectedParticipantDomains;
	for (auto pi = participantIndexes.begin(); pi != participantIndexes.end(); ++pi)
	{
		try
		{
			auto participant = participantManager->getParticipantPtr(*pi);
			const auto numDomains = participant->getDomainCount();
			for (UIntN d = 0; d < numDomains; ++d)
			{
				if (participant->getDomainPropertiesSet().getDomainProperties(d).implementsRfProfileStatusInterface())
				{
					participant->setRfProfileOverride(*pi, d, m_nptWwanBandNotificationData);
					affectedParticipantDomains.emplace_back(std::pair<UIntN, UIntN>{*pi, d});
				}
			}
		}
		catch (participant_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "UnifiedParticipant::setRfProfileOverride", *pi);
		}
	}

	for (const auto& pd : affectedParticipantDomains)
	{
		std::shared_ptr<WIDomainRfProfileChanged> rfProfileChangedWorkItem =
			std::make_shared<WIDomainRfProfileChanged>(getDptfManager(), pd.first, pd.second);
		getDptfManager()->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(rfProfileChangedWorkItem);
	}
}


