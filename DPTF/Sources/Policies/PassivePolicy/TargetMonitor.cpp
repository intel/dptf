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

#include "TargetMonitor.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

TargetMonitor::TargetMonitor()
{
}

TargetMonitor::~TargetMonitor()
{
}

void TargetMonitor::startMonitoring(UIntN target)
{
	m_targetsMonitored.insert(target);
}

void TargetMonitor::stopMonitoring(UIntN target)
{
	if (isMonitoring(target))
	{
		m_targetsMonitored.erase(target);
	}
}

Bool TargetMonitor::isMonitoring(UIntN target)
{
	return (m_targetsMonitored.find(target) != m_targetsMonitored.end());
}

std::set<UIntN> TargetMonitor::getMonitoredTargets() const
{
	return m_targetsMonitored;
}

void TargetMonitor::stopMonitoringAll()
{
	auto targetsCopy = m_targetsMonitored;
	for (auto target = targetsCopy.begin(); target != targetsCopy.end(); target++)
	{
		stopMonitoring(*target);
	}
}
