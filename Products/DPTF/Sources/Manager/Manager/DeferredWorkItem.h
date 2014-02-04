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

#include "WorkItemInterface.h"
#include "EsifTime.h"

//
// Adds functionality needed so a work item can go into the deferred queue
//

class DeferredWorkItem : public WorkItemInterface
{
public:

    DeferredWorkItem(WorkItemInterface* workItem, UInt64 numMilliSecUntilExecution);
    virtual ~DeferredWorkItem(void);

    // implement WorkItemInterface
    virtual UInt64 getUniqueId(void) const override;
    virtual FrameworkEvent::Type getFrameworkEventType(void) const override;
    virtual EsifTime getWorkItemCreationTime(void) const override;
    virtual void setWorkItemExecutionStartTime(void) override;
    virtual EsifTime getWorkItemExecutionStartTime(void) const override;
    virtual void signalAtCompletion(EsifSemaphore* semaphore) override;
    virtual Bool matches(const WorkItemMatchCriteria& matchCriteria) const override;
    virtual std::string toXml(void) const override;
    virtual void execute(void) override;

    // implement added functionality
    WorkItemInterface* getWorkItem(void) const;
    EsifTime getDeferredProcessingTime(void) const;

private:

    // hide the copy constructor and assignment operator.
    DeferredWorkItem(const DeferredWorkItem&);
    DeferredWorkItem& operator=(const DeferredWorkItem&);

    WorkItemInterface* m_workItem;
    EsifTime m_deferredProcessingTime;
};