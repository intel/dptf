/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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
#include "WorkItemQueueManager.h"
#include "WIParticipantDestroy.h"
#include "DptfManager.h"
#include "Utility.h"

ParticipantManager::ParticipantManager(DptfManager* dptfManager) : m_dptfManager(dptfManager)
{
}

ParticipantManager::~ParticipantManager(void)
{
    destroyAllParticipants();
}

void ParticipantManager::allocateParticipant(UIntN* newParticipantIndex)
{
    UIntN firstAvailableIndex = Constants::Invalid;
    Participant* participant = nullptr;

    try
    {
        // create an instance of the participant class and save at the first available index.
        participant = new Participant(m_dptfManager);
        firstAvailableIndex = getFirstNonNullIndex(m_participant);
        m_participant[firstAvailableIndex] = participant;
    }
    catch (...)
    {
        if (firstAvailableIndex != Constants::Invalid)
        {
            m_participant[firstAvailableIndex] = nullptr;
        }
        delete participant;
        throw;
    }

    *newParticipantIndex = firstAvailableIndex;
}

void ParticipantManager::createParticipant(UIntN participantIndex, const AppParticipantDataPtr participantDataPtr,
    Bool participantEnabled)
{
    if (participantIndex >= m_participant.size() || m_participant[participantIndex] == nullptr)
    {
        throw dptf_exception("Participant index is invalid.");
    }

    // When this completes the actual participant will be instantiated and the functionality will
    // be available through the interface function pointers.
    m_participant[participantIndex]->createParticipant(participantIndex, participantDataPtr, participantEnabled);
}

void ParticipantManager::destroyAllParticipants(void)
{
    for (UIntN i = 0; i < m_participant.size(); i++)
    {
        if (m_participant[i] != nullptr)
        {
            try
            {
                // Queue up a work item and wait for the return.
                WorkItem* workItem = new WIParticipantDestroy(m_dptfManager, i);
                m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
            }
            catch (...)
            {
            }
        }
    }
}

void ParticipantManager::destroyParticipant(UIntN participantIndex)
{
    if ((participantIndex < m_participant.size()) && (m_participant[participantIndex] != nullptr))
    {
        try
        {
            m_participant[participantIndex]->destroyParticipant();
        }
        catch (...)
        {
        }

        DELETE_MEMORY_TC(m_participant[participantIndex]);
    }
}

UIntN ParticipantManager::getParticipantListCount(void) const
{
    return static_cast<UIntN>(m_participant.size());
}

Participant* ParticipantManager::getParticipantPtr(UIntN participantIndex)
{
    if ((participantIndex >= m_participant.size()) ||
        (m_participant[participantIndex] == nullptr))
    {
        throw participant_index_invalid();
    }

    return m_participant[participantIndex];
}

void ParticipantManager::clearAllParticipantCachedData()
{
    for (UIntN i = 0; i < m_participant.size(); i++)
    {
        if (m_participant[i] != nullptr)
        {
            m_participant[i]->clearParticipantCachedData();
        }
    }
}

std::string ParticipantManager::GetStatusAsXml(void)
{
    throw implement_me();
}