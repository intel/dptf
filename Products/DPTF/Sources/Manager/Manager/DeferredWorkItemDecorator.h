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

class DeferredWorkItemDecorator : public WorkItemInterface
{
public:

    DeferredWorkItemDecorator(WorkItemInterface* workItem);
    ~DeferredWorkItemDecorator(void);

    // implement WorkItemInterface
    UInt64 getUniqueId(void) const;
    void execute(void);

    // implement decorated functionality
    UInt64 getTicksForDeferredProcessing(void) const;

private:

    // hide the copy constructor and assignment operator.  implement later if needed.
    DeferredWorkItemDecorator(const DeferredWorkItemDecorator&);
    DeferredWorkItemDecorator& operator=(const DeferredWorkItemDecorator&);

    WorkItemInterface* m_workItem;

    //    LARGE_INTEGER                           m_ticksForDeferredProcessing;   // this should be set if queue processing is deferred until a specific time, or 0 if immediate (in 100's of nanoseconds)
};