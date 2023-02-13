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

#include "EsifMessageLogger.h"
#include "EsifServicesInterface.h"
using namespace std;

EsifMessageLogger::EsifMessageLogger(
	std::shared_ptr<LogMessageFilter> filter,
	EsifServicesInterface* services)
	: MessageLogger(filter)
	, m_services(services)
{
}

void EsifMessageLogger::log(const LogMessage& message) const
{
	if (m_filter->shouldLog(message))
	{
		switch (message.level())
		{
		case MessageLoggingLevel::Fatal:
			m_services->writeMessageFatal(message.toString());
			break;
		case MessageLoggingLevel::Error:
			m_services->writeMessageError(message.toString());
			break;
		case MessageLoggingLevel::Warning:
			m_services->writeMessageWarning(message.toString());
			break;
		case MessageLoggingLevel::Info:
			m_services->writeMessageInfo(message.toString());
			break;
		case MessageLoggingLevel::Debug:
			m_services->writeMessageDebug(message.toString());
			break;
		}
	}
}
