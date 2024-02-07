/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "WorkItem.h"
#include "FrameworkEvent.h"

class XmlNode;

// Stores the statistics for a single work item type.
struct WorkItemTypeExecutionStatistics
{
	UInt64 totalExecuted;

	TimeSpan totalQueueTime;
	TimeSpan minQueueTime;
	TimeSpan maxQueueTime;

	TimeSpan totalExecutionTime;
	TimeSpan minExecutionTime;
	TimeSpan maxExecutionTime;
};

class WorkItemStatistics
{
public:
	WorkItemStatistics(void);
	~WorkItemStatistics(void);

	void incrementImmediateTotals(WorkItemInterface* workItem);
	void incrementDeferredTotals(WorkItemInterface* workItem);

	std::shared_ptr<XmlNode> getXml(void);

private:
	UInt64 m_totalDeferredWorkItemsExecuted;
	UInt64 m_totalImmediateWorkItemsExecuted;

	// Contains one row for each work item type.
	WorkItemTypeExecutionStatistics m_immediateWorkItemStatistics[FrameworkEvent::Max];

	// This is here to prevent the UI work items (WIDptfGetStatus) from skewing the statistics
	TimeSpan m_lastDptfGetStatusWorkItemCreationTime;
	TimeSpan m_lastDptfGetStatusWorkItemExecutionStartTime;
	TimeSpan m_lastDptfGetStatusWorkItemCompletionTime;
	TimeSpan m_lastDptfGetStatusWorkItemExecutionTime;
};
