/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

// clang-format off
#define _POLICY_LOG_MESSAGE(logger, logLevel, logFunction, content) \
	do { \
		if (logger->getLoggingLevel() >= logLevel) \
		{ \
			auto _message = [&]() {content}; \
			logger->logFunction(PolicyMessage(FLF, _message())); \
		} \
	} while (0)

#define _POLICY_LOG_MESSAGE_EX(logger, logLevel, logFunction, content) \
	do { \
		ex; \
		if (logger->getLoggingLevel() >= logLevel) \
		{ \
			auto _message = [&]() {content}; \
			logger->logFunction(PolicyMessage(FLF, _message())); \
		} \
	} while (0)

// Macros with default logger
#define POLICY_LOG_MESSAGE_FATAL(content) \
	_POLICY_LOG_MESSAGE(getPolicyServices().messageLogging, eLogTypeFatal, writeMessageFatal, content)

#define POLICY_LOG_MESSAGE_FATAL_EX(content) \
	_POLICY_LOG_MESSAGE_EX(getPolicyServices().messageLogging, eLogTypeFatal, writeMessageFatal, content)

#define POLICY_LOG_MESSAGE_ERROR(content) \
	_POLICY_LOG_MESSAGE(getPolicyServices().messageLogging, eLogTypeError, writeMessageError, content)

#define POLICY_LOG_MESSAGE_ERROR_EX(content) \
	_POLICY_LOG_MESSAGE_EX(getPolicyServices().messageLogging, eLogTypeError, writeMessageError, content)

#define POLICY_LOG_MESSAGE_WARNING(content) \
	_POLICY_LOG_MESSAGE(getPolicyServices().messageLogging, eLogTypeWarning, writeMessageWarning, content)

#define POLICY_LOG_MESSAGE_WARNING_EX(content) \
	_POLICY_LOG_MESSAGE_EX(getPolicyServices().messageLogging, eLogTypeWarning, writeMessageWarning, content)

#define POLICY_LOG_MESSAGE_INFO(content) \
	_POLICY_LOG_MESSAGE(getPolicyServices().messageLogging, eLogTypeInfo, writeMessageInfo, content)

#define POLICY_LOG_MESSAGE_INFO_EX(content) \
	_POLICY_LOG_MESSAGE_EX(getPolicyServices().messageLogging, eLogTypeInfo, writeMessageInfo, content)

#define POLICY_LOG_MESSAGE_DEBUG(content) \
	_POLICY_LOG_MESSAGE(getPolicyServices().messageLogging, eLogTypeDebug, writeMessageDebug, content)

#define POLICY_LOG_MESSAGE_DEBUG_EX(content) \
	_POLICY_LOG_MESSAGE_EX(getPolicyServices().messageLogging, eLogTypeDebug, writeMessageDebug, content)

// Macros with custom logger provided
#define POLICY_LOG_MESSAGE_FATAL_L(logger, content) \
	_POLICY_LOG_MESSAGE(logger, eLogTypeFatal, writeMessageFatal, content)

#define POLICY_LOG_MESSAGE_FATAL_EX_L(logger, content) \
	_POLICY_LOG_MESSAGE_EX(logger, eLogTypeFatal, writeMessageFatal, content)

#define POLICY_LOG_MESSAGE_ERROR_L(logger, content) \
	_POLICY_LOG_MESSAGE(logger, eLogTypeError, writeMessageError, content)

#define POLICY_LOG_MESSAGE_ERROR_EX_L(logger, content) \
	_POLICY_LOG_MESSAGE_EX(logger, eLogTypeError, writeMessageError, content)

#define POLICY_LOG_MESSAGE_WARNING_L(logger, content) \
	_POLICY_LOG_MESSAGE(logger, eLogTypeWarning, writeMessageWarning, content)

#define POLICY_LOG_MESSAGE_WARNING_EX_L(logger, content) \
	_POLICY_LOG_MESSAGE_EX(logger, eLogTypeWarning, writeMessageWarning, content)

#define POLICY_LOG_MESSAGE_INFO_L(logger, content) \
	_POLICY_LOG_MESSAGE(logger, eLogTypeInfo, writeMessageInfo, content)

#define POLICY_LOG_MESSAGE_INFO_EX_L(logger, content) \
	_POLICY_LOG_MESSAGE_EX(logger, eLogTypeInfo, writeMessageInfo, content)

#define POLICY_LOG_MESSAGE_DEBUG_L(logger, content) \
	_POLICY_LOG_MESSAGE(logger, eLogTypeDebug, writeMessageDebug, content)

#define POLICY_LOG_MESSAGE_DEBUG_EX_L(logger, content) \
	_POLICY_LOG_MESSAGE_EX(logger, eLogTypeDebug, writeMessageDebug, content)

// clang-format on
