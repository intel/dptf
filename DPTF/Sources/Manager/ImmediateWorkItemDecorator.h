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
#include "WorkItemInterface.h"

class ImmediateWorkItemDecorator : public WorkItemInterface
{
public:

    ImmediateWorkItemDecorator(WorkItemInterface* workItem, UIntN priority);
    ~ImmediateWorkItemDecorator(void);

    // implement WorkItemInterface
    UInt64 getUniqueId(void) const;
    void execute(void);

    // implement decorated functionality
    UIntN getPriority(void) const;

private:

    // hide the copy constructor and assignment operator.  implement later if needed.
    ImmediateWorkItemDecorator(const ImmediateWorkItemDecorator&);
    ImmediateWorkItemDecorator& operator=(const ImmediateWorkItemDecorator&);

    WorkItemInterface* m_workItem;
    const UIntN m_priority;

    // for the immediate queue we track how long the work item stayed in the queue
    //
    //    LARGE_INTEGER                           m_ticksAtWorkItemCreation;      // the number of ticks when the work item was created (from KeQuerySystemTime(), in 100's of nanoseconds)
    //    BOOL                                    m_signalEventAtCompletion;      // If the creator of the work item would like to know when it has been completed,
    //    PRKEVENT                                m_completionEvent;              // set SignalEventAtCompletion = TRUE and fill in the pCompletionEvent;
};