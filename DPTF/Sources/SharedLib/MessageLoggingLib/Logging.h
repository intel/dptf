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

enum class MessageLoggingLevel
{
	Fatal,
	Error,
	Warning,
	Info,
	Debug
};

inline std::string MessageLoggingLevelAsString(MessageLoggingLevel level)
{
	switch (level)
	{
	case MessageLoggingLevel::Fatal:
		return "Fatal";
	case MessageLoggingLevel::Error:
		return "Error";
	case MessageLoggingLevel::Warning:
		return "Warning";
	case MessageLoggingLevel::Info:
		return "Info";
	case MessageLoggingLevel::Debug:
		return "Debug";
	default:
		return "";
	}
}

enum class MessageLoggingCategory
{
	FIRST,
	Default = FIRST,
	Policy,
	Participant,
	Framework,
	LAST
};

inline std::string MessageLoggingCategoryAsString(MessageLoggingCategory category)
{
	switch (category)
	{
	case MessageLoggingCategory::Default:
		return "Default";
	case MessageLoggingCategory::Policy:
		return "Policy";
	case MessageLoggingCategory::Participant:
		return "Participant";
	case MessageLoggingCategory::Framework:
		return "Framework";
	default:
		return "";
	}
}