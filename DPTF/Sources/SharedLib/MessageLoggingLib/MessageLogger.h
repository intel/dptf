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
#include <functional>
#include "Dptf.h"
#include "LogMessage.h"
#include "LogMessageFilter.h"

class MessageLogger
{
public:
	MessageLogger();
	MessageLogger(std::shared_ptr<LogMessageFilter> filter);
	virtual ~MessageLogger() = default;

	virtual void log(const LogMessage& message) const = 0;
	void logDeferredEvaluation(
		const std::function<std::string(void)>& message,
		const LogMessageLocation& location,
		MessageLoggingLevel level,
		MessageLoggingCategory category) const;

protected:
	std::shared_ptr<LogMessageFilter> m_filter;
};

#define LOG_MESSAGE(logger, logLevel, category, content) \
	{ \
		auto __location = LogMessageLocation{__FILE__, __LINE__, __FUNCTION__}; \
		auto __message = [&]() { content }; \
		logger->logDeferredEvaluation( \
			__message, __location, logLevel, category); \
	}

#define LOG_MESSAGE_FRAMEWORK_FATAL(logger, content) \
	LOG_MESSAGE(logger, MessageLoggingLevel::Fatal, MessageLoggingCategory::Framework, content)
#define LOG_MESSAGE_FRAMEWORK_ERROR(logger, content) \
	LOG_MESSAGE(logger, MessageLoggingLevel::Error, MessageLoggingCategory::Framework, content)
#define LOG_MESSAGE_FRAMEWORK_WARNING(logger, content) \
	LOG_MESSAGE(logger, MessageLoggingLevel::Warning, MessageLoggingCategory::Framework, content)
#define LOG_MESSAGE_FRAMEWORK_INFO(logger, content) \
	LOG_MESSAGE(logger, MessageLoggingLevel::Info, MessageLoggingCategory::Framework, content)
#define LOG_MESSAGE_FRAMEWORK_DEBUG(logger, content) \
	LOG_MESSAGE(logger, MessageLoggingLevel::Debug, MessageLoggingCategory::Framework, content)
