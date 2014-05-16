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

#pragma once

#include "Dptf.h"

class DptfManager;
class PolicyManager;
class Policy;
class ParticipantManager;
class WorkItemQueueManager;
class EsifServices;

class PolicyServices
{
public:

    PolicyServices(DptfManager* dptfManager, UIntN policyIndex);

    DptfManager* getDptfManager(void) const;
    UIntN getPolicyIndex(void) const;
    PolicyManager* getPolicyManager(void) const;
    Policy* getPolicy(void) const;
    ParticipantManager* getParticipantManager(void) const;
    WorkItemQueueManager* getWorkItemQueueManager(void) const;
    EsifServices* getEsifServices(void) const;
    void throwIfNotWorkItemThread(void);

private:

    DptfManager* m_dptfManager;
    UIntN m_policyIndex;
    PolicyManager* m_policyManager;
    Policy* m_policy;
    ParticipantManager* m_participantManager;
    WorkItemQueueManager* m_workItemQueueManager;
    EsifServices* m_esifServices;
};