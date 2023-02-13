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

#pragma once
#include "Dptf.h"
#include "Logging.h"
#include "LogMessageLocation.h"

class LogMessage
{
public:
	LogMessage(
		const std::string& message,
		LogMessageLocation location,
		MessageLoggingLevel level = MessageLoggingLevel::Debug,
		MessageLoggingCategory category = MessageLoggingCategory::Default);
	virtual ~LogMessage() = default;
	bool operator==(const LogMessage& other) const;
	std::string toString() const;

	std::string version() const;
	std::string message() const;
	LogMessageLocation location() const;
	MessageLoggingLevel level() const;
	MessageLoggingCategory category() const;

private:

	std::string m_version;
	std::string m_message;
	LogMessageLocation m_location;
	MessageLoggingLevel m_level;
	MessageLoggingCategory m_category;
};