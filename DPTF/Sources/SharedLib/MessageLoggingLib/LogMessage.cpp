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

#include "LogMessage.h"
#include "DptfVer.h"
using namespace std;

#define APPLICATION_NAME "DTT"s
#define SEPARATOR ','

LogMessage::LogMessage(
	const string& message,
	LogMessageLocation location,
	MessageLoggingLevel level,
	MessageLoggingCategory category)
	: m_version(VERSION_STR)
	, m_message(message)
	, m_location(location)
	, m_level(level)
	, m_category(category)
{

}

bool LogMessage::operator==(const LogMessage& other) const
{
	return (m_version == other.m_version)
		&& (m_message == other.m_message)
		&& (m_location == other.m_location)
		&& (m_level == other.m_level)
		&& (m_category == other.m_category);
}

string LogMessage::toString() const
{
	// DTT,version,level,category,location,message
	const list<string> messageComponents{
		APPLICATION_NAME,
		m_version,
		MessageLoggingLevelAsString(m_level),
		MessageLoggingCategoryAsString(m_category),
		m_location.toStringCompact(),
		m_message};
	stringstream stream;
	for (const auto& c : messageComponents)
	{
		stream << c << SEPARATOR;
	}
	string result = stream.str();
	result.pop_back(); // remove last separator
	return result;
}

string LogMessage::version() const
{
	return m_version;
}

string LogMessage::message() const
{
	return m_message;
}

LogMessageLocation LogMessage::location() const
{
	return m_location;
}

MessageLoggingLevel LogMessage::level() const
{
	return m_level;
}

MessageLoggingCategory LogMessage::category() const
{
	return m_category;
}
