/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "WorkItemStatistics.h"
#include "XmlNode.h"
#include "FrameworkEvent.h"

WorkItemStatistics::WorkItemStatistics(void)
{
    m_totalDeferredWorkItemsExecuted = 0;
    m_totalImmediateWorkItemsExecuted = 0;

    for (UIntN i = 0; i < FrameworkEvent::Max; i++)
    {
        m_immediateWorkItemStatistics[i].totalExecuted = 0;

        m_immediateWorkItemStatistics[i].totalQueueTime = TimeSpan::createFromSeconds(0);
        m_immediateWorkItemStatistics[i].minQueueTime = TimeSpan::createFromSeconds(0);
        m_immediateWorkItemStatistics[i].maxQueueTime = TimeSpan::createFromSeconds(0);

        m_immediateWorkItemStatistics[i].totalExecutionTime = TimeSpan::createFromSeconds(0);
        m_immediateWorkItemStatistics[i].minExecutionTime = TimeSpan::createFromSeconds(0);
        m_immediateWorkItemStatistics[i].maxExecutionTime = TimeSpan::createFromSeconds(0);
    }

    m_lastDptfGetStatusWorkItemCreationTime = TimeSpan::createFromSeconds(0);
    m_lastDptfGetStatusWorkItemExecutionStartTime = TimeSpan::createFromSeconds(0);
    m_lastDptfGetStatusWorkItemCompletionTime = TimeSpan::createFromSeconds(0);
    m_lastDptfGetStatusWorkItemExecutionTime = TimeSpan::createFromSeconds(0);
}

WorkItemStatistics::~WorkItemStatistics(void)
{
}

void WorkItemStatistics::incrementImmediateTotals(WorkItemInterface* workItem)
{
    FrameworkEvent::Type eventType = workItem->getFrameworkEventType();
    if (eventType >= FrameworkEvent::Max)
    {
        return;
    }

    auto currentTime = EsifTime().getTimeStamp();

    m_totalImmediateWorkItemsExecuted += 1;
    m_immediateWorkItemStatistics[eventType].totalExecuted += 1;

    auto queueTime = workItem->getWorkItemExecutionStartTime() - workItem->getWorkItemCreationTime();
    auto executionTime = currentTime - workItem->getWorkItemExecutionStartTime();

    // See if a DptfGetStatus work item executed while this work item was sitting in the queue.  If so we need to
    // adjust the queueTimeMilliseconds.
    if ((eventType != FrameworkEvent::DptfGetStatus) &&
        (m_lastDptfGetStatusWorkItemCompletionTime > workItem->getWorkItemCreationTime()))
    {
        if (m_lastDptfGetStatusWorkItemExecutionStartTime >= workItem->getWorkItemCreationTime())
        {
            // deduct all of the DptfGetStatus execution time
            queueTime -= m_lastDptfGetStatusWorkItemExecutionTime;
        }
        else
        {
            // only deduct part of the DptfGetStatus execution time
            auto timeToDeduct = m_lastDptfGetStatusWorkItemCompletionTime - workItem->getWorkItemCreationTime();
            queueTime -= timeToDeduct;
        }
    }

    m_immediateWorkItemStatistics[eventType].totalQueueTime += queueTime;
    m_immediateWorkItemStatistics[eventType].totalExecutionTime += executionTime;

    if (m_immediateWorkItemStatistics[eventType].totalExecuted == 1)
    {
        m_immediateWorkItemStatistics[eventType].minQueueTime = queueTime;
        m_immediateWorkItemStatistics[eventType].maxQueueTime = queueTime;

        m_immediateWorkItemStatistics[eventType].minExecutionTime = executionTime;
        m_immediateWorkItemStatistics[eventType].maxExecutionTime = executionTime;
    }
    else
    {
        if (queueTime < m_immediateWorkItemStatistics[eventType].minQueueTime)
        {
            m_immediateWorkItemStatistics[eventType].minQueueTime = queueTime;
        }

        if (queueTime > m_immediateWorkItemStatistics[eventType].maxQueueTime)
        {
            m_immediateWorkItemStatistics[eventType].maxQueueTime = queueTime;
        }

        if (executionTime < m_immediateWorkItemStatistics[eventType].minExecutionTime)
        {
            m_immediateWorkItemStatistics[eventType].minExecutionTime = executionTime;
        }

        if (executionTime > m_immediateWorkItemStatistics[eventType].maxExecutionTime)
        {
            m_immediateWorkItemStatistics[eventType].maxExecutionTime = executionTime;
        }
    }

    if (eventType == FrameworkEvent::DptfGetStatus)
    {
        m_lastDptfGetStatusWorkItemCreationTime = workItem->getWorkItemCreationTime();
        m_lastDptfGetStatusWorkItemExecutionStartTime = workItem->getWorkItemExecutionStartTime();
        m_lastDptfGetStatusWorkItemCompletionTime = currentTime;
        m_lastDptfGetStatusWorkItemExecutionTime = executionTime;
    }
}

void WorkItemStatistics::incrementDeferredTotals(WorkItemInterface* workItem)
{
    m_totalDeferredWorkItemsExecuted += 1;
}

std::shared_ptr<XmlNode> WorkItemStatistics::getXml(void)
{
    auto workItemStatistics = XmlNode::createWrapperElement("work_item_statistics");

    // print the total number that have executed in each queue

    workItemStatistics->addChild(XmlNode::createDataElement("total_deferred_work_items_executed",
        StlOverride::to_string(m_totalDeferredWorkItemsExecuted)));
    workItemStatistics->addChild(XmlNode::createDataElement("total_immediate_work_items_executed",
        StlOverride::to_string(m_totalImmediateWorkItemsExecuted)));

    // create a table containing one row for each work item type.  this is for immediate work items only.

    auto immediateWorkItemStatistics = XmlNode::createWrapperElement("immediate_work_item_statistics");
    workItemStatistics->addChild(immediateWorkItemStatistics);

    for (UIntN i = 0; i < FrameworkEvent::Max; i++)
    {
        const FrameworkEventData event = (*FrameworkEventInfo::instance())[(FrameworkEvent::Type)i];
        UInt64 totalExecuted = m_immediateWorkItemStatistics[i].totalExecuted;

        auto averageQueueTime = TimeSpan::createFromSeconds(0);
        auto averageExecutionTime = TimeSpan::createFromSeconds(0);

        if (totalExecuted > 0)
        {
            averageQueueTime = m_immediateWorkItemStatistics[i].totalQueueTime / totalExecuted;
            averageExecutionTime = m_immediateWorkItemStatistics[i].totalExecutionTime / totalExecuted;
        }

        auto workItem = XmlNode::createWrapperElement("work_item");
        immediateWorkItemStatistics->addChild(workItem);

        workItem->addChild(XmlNode::createDataElement("work_item_type", event.name));
        workItem->addChild(XmlNode::createDataElement("total_executed", StlOverride::to_string(totalExecuted)));

        workItem->addChild(XmlNode::createDataElement("average_queue_time", averageQueueTime.toStringMilliseconds()));
        workItem->addChild(XmlNode::createDataElement("min_queue_time", m_immediateWorkItemStatistics[i].minQueueTime.toStringMilliseconds()));
        workItem->addChild(XmlNode::createDataElement("max_queue_time", m_immediateWorkItemStatistics[i].maxQueueTime.toStringMilliseconds()));

        workItem->addChild(XmlNode::createDataElement("average_execution_time", averageExecutionTime.toStringMilliseconds()));
        workItem->addChild(XmlNode::createDataElement("min_execution_time", m_immediateWorkItemStatistics[i].minExecutionTime.toStringMilliseconds()));
        workItem->addChild(XmlNode::createDataElement("max_execution_time", m_immediateWorkItemStatistics[i].maxExecutionTime.toStringMilliseconds()));
    }

    return workItemStatistics;
}