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

#include "ParticipantManager.h"
#include "WorkItemQueueManagerInterface.h"
#include "WIParticipantDestroy.h"
#include "EsifServicesInterface.h"
#include "MapOps.h"
#include "Utility.h"
#include "ManagerLogger.h"
#include "ManagerMessage.h"

using namespace std;

ParticipantManager::ParticipantManager(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
	, m_participants()
{
}

ParticipantManager::~ParticipantManager()
{
	try
	{
		ParticipantManager::destroyAllParticipants();
	}
	catch (...)
	{
		// best effort	
	}
}

UIntN ParticipantManager::allocateNextParticipantIndex()
{
	UIntN firstAvailableIndex = Constants::Invalid;

	auto indexesInUse = MapOps<UIntN, shared_ptr<Participant>>::getKeys(m_participants);
	firstAvailableIndex = getFirstAvailableIndex(indexesInUse);

	return firstAvailableIndex;
}

void ParticipantManager::createParticipant(
	UIntN participantIndex,
	const AppParticipantDataPtr participantDataPtr,
	Bool participantEnabled)
{
	if (participantIndex == Constants::Invalid || participantIndex == Constants::Esif::NoParticipant)
	{
		throw dptf_exception("Participant index is invalid.");
	}

	try
	{
		// create an instance of the participant class and save at the first available index.
		// When this completes the actual participant will be instantiated and the functionality will
		// be available through the interface function pointers.
		m_participants[participantIndex] = make_shared<Participant>(m_dptfManager);
		m_participants[participantIndex]->createParticipant(participantIndex, participantDataPtr, participantEnabled);
	}
	catch (...)
	{
		throw dptf_exception("Failed to create participant at index " + to_string(participantIndex));
	}
}

void ParticipantManager::destroyAllParticipants()
{
	const auto participantIndexes = MapOps<UIntN, shared_ptr<Participant>>::getKeys(m_participants);
	for (const auto participantIndex : participantIndexes)
	{
		try
		{
			// Queue up a work item and wait for the return.
			auto workItem = make_shared<WIParticipantDestroy>(m_dptfManager, participantIndex);
			m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
		}
		catch (...)
		{
			MANAGER_LOG_MESSAGE_ERROR({
				ManagerMessage message = ManagerMessage(
					m_dptfManager,
					_file,
					_line,
					_function,
					"Failed while trying to enqueue and wait for WIParticipantDestroy.");
					message.addMessage("Participant Index", participantIndex);
				return message;
			});
		}
	}
}

void ParticipantManager::destroyParticipant(UIntN participantIndex)
{
	const auto requestedParticipant = m_participants.find(participantIndex);
	if (requestedParticipant != m_participants.end())
	{
		if (requestedParticipant->second != nullptr)
		{
			try
			{
				requestedParticipant->second->destroyParticipant();
			}
			catch (...)
			{
				MANAGER_LOG_MESSAGE_ERROR({
					ManagerMessage message = ManagerMessage(
						m_dptfManager, _file, _line, _function, "Failed while trying to destroy participant.");
					message.addMessage("Participant Index", participantIndex);
					return message;
				});
			}
		}

		m_participants.erase(requestedParticipant);
	}
}

std::set<UIntN> ParticipantManager::getParticipantIndexes(void) const
{
	return MapOps<UIntN, shared_ptr<Participant>>::getKeys(m_participants);
}

Participant* ParticipantManager::getParticipantPtr(UIntN participantIndex) const
{
	const auto requestedParticipant = m_participants.find(participantIndex);
	if ((requestedParticipant == m_participants.end()) || (requestedParticipant->second == nullptr))
	{
		throw participant_index_invalid();
	}

	return requestedParticipant->second.get();
}

void ParticipantManager::clearAllParticipantCachedData()
{
	for (auto& [index, participant] : m_participants)
	{
		if (participant)
		{
			participant->clearParticipantCachedData();
		}
	}
}

string ParticipantManager::GetStatusAsXml(void)
{
	throw implement_me();
}

shared_ptr<IParticipant> ParticipantManager::getParticipant(const std::string& participantName) const
{
	for (const auto& [index, participant] : m_participants)
	{
		if (participant && participantName == participant->getParticipantName())
		{
			return participant;
		}
	}
	throw dptf_exception("Participant "s + participantName + " not found."s);
}

Bool ParticipantManager::participantExists(const std::string& participantName) const
{
	for (const auto& [index, participant] : m_participants)
	{
		if (participant && participantName == participant->getParticipantName())
		{
			return true;
		}
	}
	return false;
}

EsifServicesInterface* ParticipantManager::getEsifServices() const
{
	return m_dptfManager->getEsifServices();
}
