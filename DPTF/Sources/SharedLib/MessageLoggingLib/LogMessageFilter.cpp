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

#include "LogMessageFilter.h"

#define DEFAULT_LOGGING_LEVEL MessageLoggingLevel::Debug

LogMessageFilter::LogMessageFilter()
	: m_level(DEFAULT_LOGGING_LEVEL)
{
	allowAllCategories();
}

bool LogMessageFilter::shouldLog(const LogMessage& message) const
{
	return levelIsAllowed(message.level()) && categoryIsAllowed(message.category());
}

void LogMessageFilter::setLevel(MessageLoggingLevel level)
{
	m_level = level;
}

void LogMessageFilter::setLevel(eLogType level)
{
	setLevel(static_cast<MessageLoggingLevel>(level));
}

void LogMessageFilter::allowCategory(MessageLoggingCategory category)
{
	m_allowedCategories.insert(category);
}

void LogMessageFilter::allowAllCategories()
{
	constexpr auto first = static_cast<int>(MessageLoggingCategory::FIRST);
	constexpr auto last = static_cast<int>(MessageLoggingCategory::LAST);
	for (auto c = first; c < last; ++c)
	{
		allowCategory(static_cast<MessageLoggingCategory>(c));
	}
}

void LogMessageFilter::allowOnlyCategory(MessageLoggingCategory category)
{
	m_allowedCategories.clear();
	allowCategory(category);
}

void LogMessageFilter::disallowCategory(MessageLoggingCategory category)
{
	m_allowedCategories.erase(category);
}

bool LogMessageFilter::categoryIsAllowed(MessageLoggingCategory category) const
{
	const auto result = m_allowedCategories.find(category);
	return result != m_allowedCategories.end();
}

bool LogMessageFilter::levelIsAllowed(MessageLoggingLevel level) const
{
	return level <= m_level;
}