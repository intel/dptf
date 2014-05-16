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
#include "esif_data.h"

class EsifServices;
class WorkItemQueueManager;
class PolicyManager;
class ParticipantManager;
class IndexContainer;
class DptfStatus;
enum _t_eAppStatusCommand : unsigned int;
enum _t_eLogType : unsigned int;

struct _t_EsifInterface;
typedef struct _t_EsifInterface* EsifInterfacePtr;
typedef enum _t_eAppStatusCommand eAppStatusCommand;
typedef enum _t_eLogType eLogType;

//
// DPTF starts here!!!
//

class DptfManager
{
public:

    // This is in place so we can create the handle for esif before it calls back to createDptfManager()
    DptfManager(void);

    // This will shut down DPTF and clean up resources.
    ~DptfManager(void);

    // This will create all of the DPTF subsystems.
    void createDptfManager(const void* esifHandle, EsifInterfacePtr esifInterfacePtr,
        const std::string& dptfHomeDirectoryPath, eLogType currentLogVerbosityLevel, Bool dptfEnabled);

    Bool isDptfManagerCreated(void) const;
    Bool isDptfShuttingDown(void) const;
    Bool isWorkItemQueueManagerCreated(void) const;

    EsifServices* getEsifServices(void) const;
    WorkItemQueueManager* getWorkItemQueueManager(void) const;
    PolicyManager* getPolicyManager(void) const;
    ParticipantManager* getParticipantManager(void) const;
    DptfStatus* getDptfStatus(void) const;
    IndexContainer* getIndexContainer(void) const;

    std::string getDptfHomeDirectoryPath(void) const;

private:

    // hide the copy constructor and assignment operator.
    DptfManager(const DptfManager& rhs);
    DptfManager& operator=(const DptfManager& rhs);

    Bool m_dptfManagerCreateStarted;
    Bool m_dptfManagerCreateFinished;
    Bool m_dptfShuttingDown;
    Bool m_workItemQueueManagerCreated;

    Bool m_dptfEnabled;

    // EsifServices is the only way to make calls back to ESIF.
    EsifServices* m_esifServices;

    // All work item threads, enqueueing, dequeuing, and work item dispatch is handled by the WorkItemQueueManager.
    WorkItemQueueManager* m_workItemQueueManager;

    // Manages all of the polices and events that are registered for each policy.  When PolicyManager is instantiated
    // each policy is also created.
    PolicyManager* m_policyManager;

    // Manages all of the participants.  When ParticipantManager is first created there are no participants.  When
    // ESIF calls AppParticipantCreate() and AppDomainCreate() the participants will get created and the policies
    // will be notified as they come in to the DPTF framework.
    ParticipantManager* m_participantManager;

    // Creates XML needed for requests from the UI
    DptfStatus* m_dptfStatus;

    IndexContainer* m_indexContainer;

    std::string m_dptfHomeDirectoryPath;

    void shutDown(void);
    void disableAndEmptyAllQueues(void);
    void deleteDptfStatus(void);
    void destroyAllPolicies(void);
    void destroyAllParticipants(void);
    void deleteWorkItemQueueManager(void);
    void deletePolicyManager(void);
    void deleteParticipantManager(void);
    void deleteEsifServices(void);
    void deleteIndexContainer(void);
    void destroyUniqueIdGenerator(void);
    void destroyFrameworkEventInfo(void);

    void registerDptfFrameworkEvents(void);
    void unregisterDptfFrameworkEvents(void);
};