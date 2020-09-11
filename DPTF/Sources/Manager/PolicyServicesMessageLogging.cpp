/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "PolicyServicesMessageLogging.h"
#include "EsifServicesInterface.h"
#include "ManagerMessage.h"
#include "ManagerLogger.h"

PolicyServicesMessageLogging::PolicyServicesMessageLogging(DptfManagerInterface* dptfManager, UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
{
}

void PolicyServicesMessageLogging::writeMessageFatal(const DptfMessage& message)
{
	throwIfNotWorkItemThread();

	MANAGER_LOG_MESSAGE_FATAL({
		ManagerMessage updatedMessage = ManagerMessage(getDptfManager(), message);
		updatedMessage.setPolicyIndex(getPolicyIndex());
		return updatedMessage;
	});
}

void PolicyServicesMessageLogging::writeMessageError(const DptfMessage& message)
{
	throwIfNotWorkItemThread();

	MANAGER_LOG_MESSAGE_ERROR({
		ManagerMessage updatedMessage = ManagerMessage(getDptfManager(), message);
		updatedMessage.setPolicyIndex(getPolicyIndex());
		return updatedMessage;
	});
}

void PolicyServicesMessageLogging::writeMessageWarning(const DptfMessage& message)
{
	throwIfNotWorkItemThread();

	MANAGER_LOG_MESSAGE_WARNING({
		ManagerMessage updatedMessage = ManagerMessage(getDptfManager(), message);
		updatedMessage.setPolicyIndex(getPolicyIndex());
		return updatedMessage;
	});
}

void PolicyServicesMessageLogging::writeMessageInfo(const DptfMessage& message)
{
	throwIfNotWorkItemThread();

	MANAGER_LOG_MESSAGE_INFO({
		ManagerMessage updatedMessage = ManagerMessage(getDptfManager(), message);
		updatedMessage.setPolicyIndex(getPolicyIndex());
		return updatedMessage;
	});
}

void PolicyServicesMessageLogging::writeMessageDebug(const DptfMessage& message)
{
	throwIfNotWorkItemThread();

	MANAGER_LOG_MESSAGE_DEBUG({
		ManagerMessage updatedMessage = ManagerMessage(getDptfManager(), message);
		updatedMessage.setPolicyIndex(getPolicyIndex());
		return updatedMessage;
	});
}

eLogType PolicyServicesMessageLogging::getLoggingLevel()
{
	throwIfNotWorkItemThread();
	return getEsifServices()->getCurrentLogVerbosityLevel();
}
