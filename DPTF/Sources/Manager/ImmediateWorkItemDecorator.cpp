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

#include "ImmediateWorkItemDecorator.h"

ImmediateWorkItemDecorator::ImmediateWorkItemDecorator(WorkItemInterface* workItem, UIntN priority) :
    m_workItem(workItem), m_priority(priority)
{
}

ImmediateWorkItemDecorator::~ImmediateWorkItemDecorator(void)
{
    delete m_workItem;
}

UInt64 ImmediateWorkItemDecorator::getUniqueId(void) const
{
    return m_workItem->getUniqueId();
}

void ImmediateWorkItemDecorator::execute(void)
{
    return m_workItem->execute();
}

UIntN ImmediateWorkItemDecorator::getPriority(void) const
{
    return m_priority;
}