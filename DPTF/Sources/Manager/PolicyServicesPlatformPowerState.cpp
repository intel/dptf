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

#include "PolicyServicesPlatformPowerState.h"
#include "EsifServicesInterface.h"
#include "esif_ccb_string.h"
#include "ManagerLogger.h"
#include "ManagerMessage.h"

void* ThreadSleep(void* self);
void* ThreadHibernate(void* self);
void* ThreadShutdown(void* self);

PolicyServicesPlatformPowerState::PolicyServicesPlatformPowerState(DptfManagerInterface* dptfManager, UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
	, m_thermalEvent()
	, m_thread()
{
}

PolicyServicesPlatformPowerState::~PolicyServicesPlatformPowerState()
{
	esif_ccb_thread_join(&m_thread);
}

esif_data_complex_thermal_event* PolicyServicesPlatformPowerState::getThermalEventPtr(void)
{
	return &m_thermalEvent;
}

void PolicyServicesPlatformPowerState::setThermalEvent(
	const Temperature currentTemperature,
	const Temperature tripPointTemperature,
	const std::string participantName)
{
	m_thermalEvent.temperature = currentTemperature;
	m_thermalEvent.tripPointTemperature = tripPointTemperature;
	esif_ccb_strncpy(m_thermalEvent.participantName, participantName.c_str(), sizeof(m_thermalEvent.participantName));
}

void PolicyServicesPlatformPowerState::sleep(void)
{
	throwIfNotWorkItemThread();

	esif_ccb_thread_join(&m_thread);
	eEsifError rc = esif_ccb_thread_create(&m_thread, ThreadSleep, this);
	if (rc != ESIF_OK)
	{
		// TODO: want to pass in MessageCategory::EsifServicesInterface as a parameter
		MANAGER_LOG_MESSAGE_ERROR({
			ManagerMessage message =
				ManagerMessage(getDptfManager(), _file, _line, _function, "Failed to request sleep");
			return message;
		});
	}
}

void PolicyServicesPlatformPowerState::hibernate(
	const Temperature& currentTemperature,
	const Temperature& tripPointTemperature,
	const std::string& participantName)
{
	throwIfNotWorkItemThread();

	setThermalEvent(currentTemperature, tripPointTemperature, participantName);
	esif_ccb_thread_join(&m_thread);
	eEsifError rc = esif_ccb_thread_create(&m_thread, ThreadHibernate, this);
	if (rc != ESIF_OK)
	{
		// TODO: want to pass in MessageCategory::EsifServicesInterface as a parameter
		MANAGER_LOG_MESSAGE_ERROR({
			ManagerMessage message =
				ManagerMessage(getDptfManager(), _file, _line, _function, "Failed to request hibernate");
			return message;
		});
	}
}

void PolicyServicesPlatformPowerState::shutDown(
	const Temperature& currentTemperature,
	const Temperature& tripPointTemperature,
	const std::string& participantName)
{
	throwIfNotWorkItemThread();

	setThermalEvent(currentTemperature, tripPointTemperature, participantName);
	esif_ccb_thread_join(&m_thread);
	eEsifError rc = esif_ccb_thread_create(&m_thread, ThreadShutdown, this);
	if (rc != ESIF_OK)
	{
		// TODO: want to pass in MessageCategory::EsifServicesInterface as a parameter
		MANAGER_LOG_MESSAGE_ERROR({
			ManagerMessage message =
				ManagerMessage(getDptfManager(), _file, _line, _function, "Failed to request shutdown");
			return message;
		});
	}
}

void* ThreadSleep(void* self)
{
	if (self != nullptr)
	{
		try
		{
			PolicyServicesPlatformPowerState* me = (PolicyServicesPlatformPowerState*)self;
			me->getEsifServices()->primitiveExecuteSetAsUInt32(SET_SYSTEM_SLEEP, 1);
		}
		catch (...)
		{
			// do nothing on error
		}
	}
	return nullptr;
}

void* ThreadHibernate(void* self)
{
	if (self != nullptr)
	{
		try
		{
			PolicyServicesPlatformPowerState* me = (PolicyServicesPlatformPowerState*)self;
			me->getEsifServices()->primitiveExecuteSet(
				SET_SYSTEM_HIBERNATE,
				ESIF_DATA_STRUCTURE,
				me->getThermalEventPtr(),
				sizeof(esif_data_complex_thermal_event),
				sizeof(esif_data_complex_thermal_event));
		}
		catch (...)
		{
			// do nothing on error
		}
	}
	return nullptr;
}

void* ThreadShutdown(void* self)
{
	if (self != nullptr)
	{
		try
		{
			PolicyServicesPlatformPowerState* me = (PolicyServicesPlatformPowerState*)self;
			me->getEsifServices()->primitiveExecuteSet(
				SET_SYSTEM_SHUTDOWN,
				ESIF_DATA_STRUCTURE,
				me->getThermalEventPtr(),
				sizeof(esif_data_complex_thermal_event),
				sizeof(esif_data_complex_thermal_event));
		}
		catch (...)
		{
			// do nothing on error
		}
	}
	return nullptr;
}
