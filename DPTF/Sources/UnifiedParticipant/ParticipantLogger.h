/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#define _PARTICIPANT_LOG_MESSAGE(logLevel, logFunction, content) \
	do { \
		if (getParticipantServices()->getLoggingLevel() >= logLevel) \
		{ \
			auto _message = [&]() {content}; \
			getParticipantServices()->logFunction(ParticipantMessage(FLF, _message())); \
		} \
	} while (0)

#define _PARTICIPANT_LOG_MESSAGE_EX(logLevel, logFunction, content) \
	do { \
		ex; \
		if (getParticipantServices()->getLoggingLevel() >= logLevel) \
		{ \
			auto _message = [&]() {content}; \
			getParticipantServices()->logFunction(ParticipantMessage(FLF, _message())); \
		} \
	} while (0)

#define PARTICIPANT_LOG_MESSAGE_FATAL(content) \
	_PARTICIPANT_LOG_MESSAGE(eLogTypeFatal, writeMessageFatal, content)

#define PARTICIPANT_LOG_MESSAGE_FATAL_EX(content) \
	_PARTICIPANT_LOG_MESSAGE_EX(eLogTypeFatal, writeMessageFatal, content)

#define PARTICIPANT_LOG_MESSAGE_ERROR(content) \
	_PARTICIPANT_LOG_MESSAGE(eLogTypeError, writeMessageError, content)

#define PARTICIPANT_LOG_MESSAGE_ERROR_EX(content) \
	_PARTICIPANT_LOG_MESSAGE_EX(eLogTypeError, writeMessageError, content)

#define PARTICIPANT_LOG_MESSAGE_WARNING(content) \
	_PARTICIPANT_LOG_MESSAGE(eLogTypeWarning, writeMessageWarning, content)

#define PARTICIPANT_LOG_MESSAGE_WARNING_EX(content) \
	_PARTICIPANT_LOG_MESSAGE_EX(eLogTypeWarning, writeMessageWarning, content)

#define PARTICIPANT_LOG_MESSAGE_INFO(content) \
	_PARTICIPANT_LOG_MESSAGE(eLogTypeInfo, writeMessageInfo, content)

#define PARTICIPANT_LOG_MESSAGE_INFO_EX(content) \
	_PARTICIPANT_LOG_MESSAGE_EX(eLogTypeInfo, writeMessageInfo, content)

#define PARTICIPANT_LOG_MESSAGE_DEBUG(content) \
	_PARTICIPANT_LOG_MESSAGE(eLogTypeDebug, writeMessageDebug, content)

#define PARTICIPANT_LOG_MESSAGE_DEBUG_EX(content) \
	_PARTICIPANT_LOG_MESSAGE_EX(eLogTypeDebug, writeMessageDebug, content)

// clang-format on
