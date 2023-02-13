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
#include "esif_sdk_iface.h"
#include "Logging.h"
#include "LogMessage.h"

class LogMessageFilter
{
public:
	LogMessageFilter();

	bool shouldLog(const LogMessage& message) const;
	void setLevel(MessageLoggingLevel level);
	void setLevel(eLogType level);
	void allowCategory(MessageLoggingCategory category);
	void allowAllCategories();
	void allowOnlyCategory(MessageLoggingCategory category);
	void disallowCategory(MessageLoggingCategory category);

private:
	MessageLoggingLevel m_level;
	std::set<MessageLoggingCategory> m_allowedCategories;

	bool categoryIsAllowed(MessageLoggingCategory category) const;
	bool levelIsAllowed(MessageLoggingLevel level) const;
};
