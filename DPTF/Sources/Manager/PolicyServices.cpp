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

#include "PolicyServices.h"
#include "DptfManager.h"
#include "EsifServices.h"
#include "WorkItemQueueManager.h"
#include "PolicyManager.h"
#include "ParticipantManager.h"


PolicyServices::PolicyServices(DptfManager* dptfManager, UIntN policyIndex) :
    m_dptfManager(dptfManager), m_policyIndex(policyIndex)
{
    m_policyManager = m_dptfManager->getPolicyManager();
    m_policy = m_policyManager->getPolicyPtr(m_policyIndex);
    m_participantManager = m_dptfManager->getParticipantManager();
    m_workItemQueueManager = m_dptfManager->getWorkItemQueueManager();
    m_esifServices = m_dptfManager->getEsifServices();
}

DptfManager* PolicyServices::getDptfManager(void) const
{
    return m_dptfManager;
}

UIntN PolicyServices::getPolicyIndex(void) const
{
    return m_policyIndex;
}

PolicyManager* PolicyServices::getPolicyManager(void) const
{
    return m_policyManager;
}

Policy* PolicyServices::getPolicy(void) const
{
    return m_policy;
}

ParticipantManager* PolicyServices::getParticipantManager(void) const
{
    return m_participantManager;
}

WorkItemQueueManager* PolicyServices::getWorkItemQueueManager(void) const
{
    return m_workItemQueueManager;
}

EsifServices* PolicyServices::getEsifServices(void) const
{
    return m_esifServices;
}

void PolicyServices::throwIfNotWorkItemThread(void)
{
    if (m_workItemQueueManager->isWorkItemThread() == false)
    {
        throw dptf_exception("Policy Services functionality called from an unknown thread.");
    }
}