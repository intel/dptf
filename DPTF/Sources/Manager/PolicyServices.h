/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#pragma once

#include "Dptf.h"
#include "DptfManagerInterface.h"
class PolicyManager;
class Policy;
class ParticipantManagerInterface;
class WorkItemQueueManagerInterface;
class EsifServices;

class PolicyServices
{
public:

    PolicyServices(DptfManagerInterface* dptfManager, UIntN policyIndex);

    DptfManagerInterface* getDptfManager(void) const;
    UIntN getPolicyIndex(void) const;
    PolicyManager* getPolicyManager(void) const;
    Policy* getPolicy(void) const;
    ParticipantManagerInterface* getParticipantManager(void) const;
    WorkItemQueueManagerInterface* getWorkItemQueueManager(void) const;
    EsifServices* getEsifServices(void) const;
    void throwIfNotWorkItemThread(void) const;

private:

    DptfManagerInterface* m_dptfManager;
    UIntN m_policyIndex;
    PolicyManager* m_policyManager;
    Policy* m_policy;
    ParticipantManagerInterface* m_participantManager;
    WorkItemQueueManagerInterface* m_workItemQueueManager;
    EsifServices* m_esifServices;
};