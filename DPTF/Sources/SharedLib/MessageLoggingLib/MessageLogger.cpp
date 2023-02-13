/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "MessageLogger.h"
#include <string>

using namespace std;

MessageLogger::MessageLogger()
	: m_filter(nullptr)
{
}

MessageLogger::MessageLogger(shared_ptr<LogMessageFilter> filter)
	: m_filter(filter)
{
}

void MessageLogger::logDeferredEvaluation(
	const function<string(void)>& message,
	const LogMessageLocation& location,
	MessageLoggingLevel level,
	MessageLoggingCategory category) const
{
	if (m_filter.get() && m_filter->shouldLog(LogMessage(""s, location, level, category)))
	{
		log(LogMessage(message(), location, level, category));
	}
}