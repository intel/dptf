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

#include "DeferredWorkItemDecorator.h"

DeferredWorkItemDecorator::DeferredWorkItemDecorator(WorkItemInterface* workItem) : m_workItem(workItem)
{
    throw implement_me();
}

DeferredWorkItemDecorator::~DeferredWorkItemDecorator(void)
{
    delete m_workItem;
}

UInt64 DeferredWorkItemDecorator::getUniqueId(void) const
{
    return m_workItem->getUniqueId();
}

void DeferredWorkItemDecorator::execute(void)
{
    return m_workItem->execute();
}