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

#include "ApplicationTimerSettings.h"

using namespace std;

ApplicationTimerSettings::ApplicationTimerSettings(const std::shared_ptr<MessageLogger>& messageLogger)
	: IApplicationTimerSettings()
	, m_messageLogger(messageLogger)
{
	throwIfInvalidMessageLogger();
}

ApplicationTimerSettings::~ApplicationTimerSettings()
{
	ApplicationTimerSettings::clearMinimumResolutionForTimers();
}

// TODO move implementation into CCB once available

void ApplicationTimerSettings::setMinimumResolutionForTimers(const TimeSpan& resolution)
{
	// TODO once linux implementation is in CCB then can remove this code
}

void ApplicationTimerSettings::clearMinimumResolutionForTimers()
{
	// TODO once linux implementation is in CCB then can remove this code
}


void ApplicationTimerSettings::updateResolution(const TimeSpan& resolution)
{
	clearMinimumResolutionForTimers();
	m_resolution = resolution;
}

void ApplicationTimerSettings::throwIfInvalidMessageLogger() const
{
	if (!m_messageLogger)
	{
		throw dptf_exception("Message logger is invalid."s);
	}
}
