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
#include "PolicyManagerInterface.h"
#include "UiSubCommand.h"

#include "ParticipantManagerInterface.h"

using namespace std;

UiSubCommand::UiSubCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

std::vector<std::pair<UIntN, UIntN>> UiSubCommand::buildParticipantDomainsList() const
{
	std::vector<std::pair<UIntN, UIntN>> participantDomainsList;
	const auto participantManager = m_dptfManager->getParticipantManager();
	const auto participantIndexes = participantManager->getParticipantIndexes();
	for (auto participantIndex = participantIndexes.begin(); participantIndex != participantIndexes.end();
		 ++participantIndex)
	{
		try
		{
			const auto participant = participantManager->getParticipantPtr(*participantIndex);
			const UIntN domainCount = participant->getDomainCount();
			for (UIntN domainIndex = 0; domainIndex < domainCount; domainIndex++)
			{
				participantDomainsList.push_back(std::make_pair(*participantIndex, domainIndex));
			}
		}
		catch (...)
		{
			// participant not available, don't add it to the list
		}
	}
	return participantDomainsList;
}