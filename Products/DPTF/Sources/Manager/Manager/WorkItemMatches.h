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
#include "ImmediateWorkItem.h"
#include "DeferredWorkItem.h"
#include "WorkItemMatchCriteria.h"

// This is used as a predicate for removing work items from the immediate or deferred queue

class WorkItemMatches final
{
public:

    WorkItemMatches(const WorkItemMatchCriteria& matchCriteria);
    Bool operator()(ImmediateWorkItem* workItem);
    Bool operator()(DeferredWorkItem* workItem);

private:

    WorkItemMatchCriteria m_matchCriteria;
};