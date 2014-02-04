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

        m_immediateWorkItemStatistics[i].totalQueueTimeMilliseconds = 0;
        m_immediateWorkItemStatistics[i].minQueueTimeMilliseconds = 0;
        m_immediateWorkItemStatistics[i].maxQueueTimeMilliseconds = 0;

        m_immediateWorkItemStatistics[i].totalExecutionTimeMilliseconds = 0;
        m_immediateWorkItemStatistics[i].minExecutionTimeMilliseconds = 0;
        m_immediateWorkItemStatistics[i].maxExecutionTimeMilliseconds = 0;
    }

    m_lastDptfGetStatusWorkItemCreationTime = EsifTime(0);
    m_lastDptfGetStatusWorkItemExecutionStartTime = EsifTime(0);
    m_lastDptfGetStatusWorkItemCompletionTime = EsifTime(0);
    m_lastDptfGetStatusWorkItemExecutionTimeMilliseconds = 0;
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

    EsifTime currentTime = EsifTime();

    m_totalImmediateWorkItemsExecuted += 1;
    m_immediateWorkItemStatistics[eventType].totalExecuted += 1;

    UInt64 queueTimeMilliseconds = workItem->getWorkItemExecutionStartTime() - workItem->getWorkItemCreationTime();
    UInt64 executionTimeMilliseconds = currentTime - workItem->getWorkItemExecutionStartTime();

    // See if a DptfGetStatus work item executed while this work item was sitting in the queue.  If so we need to
    // adjust the queueTimeMilliseconds.
    if ((eventType != FrameworkEvent::DptfGetStatus) &&
        (m_lastDptfGetStatusWorkItemCompletionTime > workItem->getWorkItemCreationTime()))
    {
        if (m_lastDptfGetStatusWorkItemExecutionStartTime >= workItem->getWorkItemCreationTime())
        {
            // deduct all of the DptfGetStatus execution time
            queueTimeMilliseconds -= m_lastDptfGetStatusWorkItemExecutionTimeMilliseconds;
        }
        else
        {
            // only deduct part of the DptfGetStatus execution time
            UInt64 numMillisecondsToDeduct = m_lastDptfGetStatusWorkItemCompletionTime - workItem->getWorkItemCreationTime();
            queueTimeMilliseconds -= numMillisecondsToDeduct;
        }
    }

    m_immediateWorkItemStatistics[eventType].totalQueueTimeMilliseconds += queueTimeMilliseconds;
    m_immediateWorkItemStatistics[eventType].totalExecutionTimeMilliseconds += executionTimeMilliseconds;

    if (m_immediateWorkItemStatistics[eventType].totalExecuted == 1)
    {
        m_immediateWorkItemStatistics[eventType].minQueueTimeMilliseconds = queueTimeMilliseconds;
        m_immediateWorkItemStatistics[eventType].maxQueueTimeMilliseconds = queueTimeMilliseconds;

        m_immediateWorkItemStatistics[eventType].minExecutionTimeMilliseconds = executionTimeMilliseconds;
        m_immediateWorkItemStatistics[eventType].maxExecutionTimeMilliseconds = executionTimeMilliseconds;
    }
    else
    {
        if (queueTimeMilliseconds < m_immediateWorkItemStatistics[eventType].minQueueTimeMilliseconds)
        {
            m_immediateWorkItemStatistics[eventType].minQueueTimeMilliseconds = queueTimeMilliseconds;
        }

        if (queueTimeMilliseconds > m_immediateWorkItemStatistics[eventType].maxQueueTimeMilliseconds)
        {
            m_immediateWorkItemStatistics[eventType].maxQueueTimeMilliseconds = queueTimeMilliseconds;
        }

        if (executionTimeMilliseconds < m_immediateWorkItemStatistics[eventType].minExecutionTimeMilliseconds)
        {
            m_immediateWorkItemStatistics[eventType].minExecutionTimeMilliseconds = executionTimeMilliseconds;
        }

        if (executionTimeMilliseconds > m_immediateWorkItemStatistics[eventType].maxExecutionTimeMilliseconds)
        {
            m_immediateWorkItemStatistics[eventType].maxExecutionTimeMilliseconds = executionTimeMilliseconds;
        }
    }

    if (eventType == FrameworkEvent::DptfGetStatus)
    {
        m_lastDptfGetStatusWorkItemCreationTime = workItem->getWorkItemCreationTime();
        m_lastDptfGetStatusWorkItemExecutionStartTime = workItem->getWorkItemExecutionStartTime();
        m_lastDptfGetStatusWorkItemCompletionTime = currentTime;
        m_lastDptfGetStatusWorkItemExecutionTimeMilliseconds = executionTimeMilliseconds;
    }
}

void WorkItemStatistics::incrementDeferredTotals(WorkItemInterface* workItem)
{
    m_totalDeferredWorkItemsExecuted += 1;
}

XmlNode* WorkItemStatistics::getXml(void)
{
    XmlNode* workItemStatistics = XmlNode::createWrapperElement("work_item_statistics");

    // print the total number that have executed in each queue

    workItemStatistics->addChild(XmlNode::createDataElement("total_deferred_work_items_executed",
        std::to_string(m_totalDeferredWorkItemsExecuted)));
    workItemStatistics->addChild(XmlNode::createDataElement("total_immediate_work_items_executed",
        std::to_string(m_totalImmediateWorkItemsExecuted)));

    // create a table containing one row for each work item type.  this is for immediate work items only.

    XmlNode* immediateWorkItemStatistics = XmlNode::createWrapperElement("immediate_work_item_statistics");
    workItemStatistics->addChild(immediateWorkItemStatistics);

    for (UIntN i = 0; i < FrameworkEvent::Max; i++)
    {
        const FrameworkEventData event = (*FrameworkEventInfo::instance())[(FrameworkEvent::Type)i];
        UInt64 totalExecuted = m_immediateWorkItemStatistics[i].totalExecuted;

        UInt64 averageQueueTime = 0;
        UInt64 averageExecutionTime = 0;

        if (totalExecuted > 0)
        {
            averageQueueTime = m_immediateWorkItemStatistics[i].totalQueueTimeMilliseconds / totalExecuted;
            averageExecutionTime = m_immediateWorkItemStatistics[i].totalExecutionTimeMilliseconds / totalExecuted;
        }

        XmlNode* workItem = XmlNode::createWrapperElement("work_item");
        immediateWorkItemStatistics->addChild(workItem);

        workItem->addChild(XmlNode::createDataElement("work_item_type", event.name));
        workItem->addChild(XmlNode::createDataElement("total_executed", std::to_string(totalExecuted)));

        workItem->addChild(XmlNode::createDataElement("average_queue_time", std::to_string(averageQueueTime)));
        workItem->addChild(XmlNode::createDataElement("min_queue_time",
            std::to_string(m_immediateWorkItemStatistics[i].minQueueTimeMilliseconds)));
        workItem->addChild(XmlNode::createDataElement("max_queue_time",
            std::to_string(m_immediateWorkItemStatistics[i].maxQueueTimeMilliseconds)));

        workItem->addChild(XmlNode::createDataElement("average_execution_time", std::to_string(averageExecutionTime)));
        workItem->addChild(XmlNode::createDataElement("min_execution_time",
            std::to_string(m_immediateWorkItemStatistics[i].minExecutionTimeMilliseconds)));
        workItem->addChild(XmlNode::createDataElement("max_execution_time",
            std::to_string(m_immediateWorkItemStatistics[i].maxExecutionTimeMilliseconds)));
    }

    return workItemStatistics;
}