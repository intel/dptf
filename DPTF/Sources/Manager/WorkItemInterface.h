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
#include "FrameworkEvent.h"
#include "WorkItemMatchCriteria.h"
#include "EsifSemaphore.h"
#include "EsifTime.h"

class WorkItemInterface
{
public:

    virtual ~WorkItemInterface()
    {
    };

    virtual UInt64 getUniqueId(void) const = 0;
    virtual FrameworkEvent::Type getFrameworkEventType(void) const = 0;
    virtual EsifTime getWorkItemCreationTime(void) const = 0;
    virtual void setWorkItemExecutionStartTime(void) = 0;
    virtual EsifTime getWorkItemExecutionStartTime(void) const = 0;
    virtual void signalAtCompletion(EsifSemaphore* semaphore) = 0;
    virtual Bool matches(const WorkItemMatchCriteria& matchCriteria) const = 0;
    virtual std::string toXml(void) const = 0;
    virtual void execute(void) = 0;
};