/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

ParticipantManager::ParticipantManager(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
	, m_participants()
{
}

ParticipantManager::~ParticipantManager(void)
{
	try
	{
		destroyAllParticipants();
	}
	catch(...)
	{

	}
}

UIntN ParticipantManager::allocateNextParticipantIndex()
{
	UIntN firstAvailableIndex = Constants::Invalid;

	auto indexesInUse = MapOps<UIntN, std::shared_ptr<Participant>>::getKeys(m_participants);
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
		m_participants[participantIndex] = std::make_shared<Participant>(m_dptfManager);
		m_participants[participantIndex]->createParticipant(participantIndex, participantDataPtr, participantEnabled);
	}
	catch (...)
	{
		throw dptf_exception("Failed to create participant at index " + std::to_string(participantIndex));
	}
}

void ParticipantManager::destroyAllParticipants(void)
{
	auto participantIndexes = MapOps<UIntN, std::shared_ptr<Participant>>::getKeys(m_participants);
	for (auto index = participantIndexes.begin(); index != participantIndexes.end(); ++index)
	{
		try
		{
			// Queue up a work item and wait for the return.
			auto workItem = std::make_shared<WIParticipantDestroy>(m_dptfManager, *index);
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
				message.addMessage("Participant Index", *index);
				return message;
			});
		}
	}
}

void ParticipantManager::destroyParticipant(UIntN participantIndex)
{
	auto requestedParticipant = m_participants.find(participantIndex);
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
	return MapOps<UIntN, std::shared_ptr<Participant>>::getKeys(m_participants);
}

Participant* ParticipantManager::getParticipantPtr(UIntN participantIndex) const
{
	auto requestedParticipant = m_participants.find(participantIndex);
	if ((requestedParticipant == m_participants.end()) || (requestedParticipant->second == nullptr))
	{
		throw participant_index_invalid();
	}

	return requestedParticipant->second.get();
}

void ParticipantManager::clearAllParticipantCachedData()
{
	for (auto p = m_participants.begin(); p != m_participants.end(); p++)
	{
		if (p->second != nullptr)
		{
			p->second->clearParticipantCachedData();
		}
	}
}

std::string ParticipantManager::GetStatusAsXml(void)
{
	throw implement_me();
}

std::shared_ptr<IParticipant> ParticipantManager::getParticipant(const std::string& participantName) const
{
	for (auto p = m_participants.begin(); p != m_participants.end(); p++)
	{
		if (participantName == p->second->getParticipantName())
		{
			return p->second;
		}
	}
	throw dptf_exception(std::string("Participant ") + participantName + std::string(" not found."));
}

Bool ParticipantManager::participantExists(const std::string& participantName) const
{
	for (auto p = m_participants.begin(); p != m_participants.end(); p++)
	{
		if (participantName == p->second->getParticipantName())
		{
			return true;
		}
	}
	return false;
}

EsifServicesInterface* ParticipantManager::getEsifServices()
{
	return m_dptfManager->getEsifServices();
}
