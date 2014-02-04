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
#include "WorkItemMatchCriteria.h"
#include "XmlNode.h"

class WorkItemQueueInterface
{
public:

    virtual ~WorkItemQueueInterface()
    {
    };

    // Remove each item in the queue and call its destructor.  The
    // execute method will not get called.
    virtual void makeEmtpy(void) = 0;

    // Returns the current number of items in the queue
    virtual UInt64 getCount(void) const = 0;

    // Returns the maximum number of items in the queue at any one time
    virtual UInt64 getMaxCount(void) const = 0;

    virtual UIntN removeIfMatches(const WorkItemMatchCriteria& matchCriteria) = 0;
    virtual XmlNode* getXml(void) const = 0;
};