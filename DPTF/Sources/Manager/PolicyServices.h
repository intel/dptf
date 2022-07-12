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

#pragma once

#include "Dptf.h"
#include "DptfManagerInterface.h"
class PolicyManagerInterface;
class IPolicy;
class ParticipantManagerInterface;
class WorkItemQueueManagerInterface;
class EsifServicesInterface;

class PolicyServices
{
public:
	PolicyServices(DptfManagerInterface* dptfManager, UIntN policyIndex);

	DptfManagerInterface* getDptfManager(void) const;
	UIntN getPolicyIndex(void) const;
	PolicyManagerInterface* getPolicyManager(void) const;
	IPolicy* getPolicy(void) const;
	ParticipantManagerInterface* getParticipantManager(void) const;
	WorkItemQueueManagerInterface* getWorkItemQueueManager(void) const;
	EsifServicesInterface* getEsifServices(void) const;
	void throwIfNotWorkItemThread(void) const;

private:
	DptfManagerInterface* m_dptfManager;
	UIntN m_policyIndex;
	PolicyManagerInterface* m_policyManager;
	IPolicy* m_policy;
	ParticipantManagerInterface* m_participantManager;
	WorkItemQueueManagerInterface* m_workItemQueueManager;
	EsifServicesInterface* m_esifServices;
};
