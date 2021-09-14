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

#include "SystemModeManager.h"
#include "ManagerLogger.h"
#include "WorkItem.h"
#include "WorkItemQueueManagerInterface.h"
#include "WIPolicySystemModeChanged.h"
#include "EsifServicesInterface.h"
#include "StatusFormat.h"

SystemModeManager::SystemModeManager(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
	, m_policySystemModeValue(SystemMode::Invalid)
	, m_arbitratedValue(SystemMode::Invalid)
	, m_arbitratedValueChangedSinceLastSet(false)
	, m_registeredEvents()
	, m_isEnduranceGamingActive(false)
	, m_powerSource(OsPowerSource::Invalid)
{
	registerEvent(FrameworkEvent::PolicyOperatingSystemPowerSliderChanged);
	registerEvent(FrameworkEvent::PolicyOperatingSystemPowerSchemePersonalityChanged);
	registerEvent(FrameworkEvent::DptfAppBroadcastListen);
	registerEvent(FrameworkEvent::DptfAppBroadcastSend);
	registerEvent(FrameworkEvent::PolicyOperatingSystemPowerSourceChanged);
}

SystemModeManager::~SystemModeManager(void)
{
}

void SystemModeManager::arbitrateAndCreateEventSystemModeChanged()
{
	try
	{
		auto powerSlider = OsPowerSlider::Invalid;
		try
		{
			MANAGER_LOG_MESSAGE_DEBUG({ return "get powerSlider "; });
			powerSlider = m_dptfManager->getEventCache()
							  ->powerSlider.get(); // temperary solution to unblock EG, need review later
		}
		catch (dptf_exception& ex)
		{
			MANAGER_LOG_MESSAGE_WARNING_EX({ return "Failure during get powerSlider: " + std::string(ex.what()) + "."; });
		}

		auto powerSchemePersonality = OsPowerSchemePersonality::Invalid;
		try
		{
			MANAGER_LOG_MESSAGE_DEBUG({ return "get powerSchemePersonality "; });
			powerSchemePersonality =
				m_dptfManager->getEventCache()
					->powerSchemePersonality.get(); // temperary solution to unblock EG, need review later
		}
		catch (dptf_exception& ex)
		{
			MANAGER_LOG_MESSAGE_WARNING_EX(
				{ return "Failure during get powerSchemePersonality: " + std::string(ex.what()) + "."; });
		}
		arbitrateSystemMode(powerSlider, powerSchemePersonality);

		MANAGER_LOG_MESSAGE_INFO({
			return "Power Slider: " + toString(powerSlider) + ", Power Scheme Personality: "
				   + toString(powerSchemePersonality) + ", Policy System Mode: " + toString(m_policySystemModeValue)
				   + ", Arbitrated System Mode: " + toString(m_arbitratedValue);
		});

		MANAGER_LOG_MESSAGE_DEBUG({
			return " Arbitrated system mode changed or not from last arbitration: m_arbitratedValueChangedSinceLastSet: "
				   + StatusFormat::friendlyValue(m_arbitratedValueChangedSinceLastSet);
		});

		if (m_arbitratedValueChangedSinceLastSet == true)
		{
			std::shared_ptr<WorkItem> wi =
				std::make_shared<WIPolicySystemModeChanged>(m_dptfManager, m_arbitratedValue);
			m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(wi);
		}
	}
	catch (dptf_exception& ex)
	{
		MANAGER_LOG_MESSAGE_WARNING_EX(
			{ return "Failure during arbitration of system mode: " + std::string(ex.what()) + "."; });
	}
}

void SystemModeManager::arbitrateSystemMode(
	const OsPowerSlider::Type osPowerSlider,
	const OsPowerSchemePersonality::Type osPowerSchemePersonality)
{
	auto oldArbitratedValue = m_arbitratedValue;

	if (m_isEnduranceGamingActive && m_powerSource == OsPowerSource::DC)
	{
		m_arbitratedValue = SystemMode::EnduranceGaming;
	}
	else if (m_policySystemModeValue != SystemMode::Invalid)
	{
		m_arbitratedValue = m_policySystemModeValue;
	}
	else if (osPowerSlider != OsPowerSlider::Invalid)
	{
		if (osPowerSlider == OsPowerSlider::BestPerformance)
		{
			m_arbitratedValue = SystemMode::Performance;
		}
		else if (osPowerSlider == OsPowerSlider::BetterPerformance)
		{
			m_arbitratedValue = SystemMode::Balanced;
		}
		else if (osPowerSlider == OsPowerSlider::BetterBattery)
		{
			m_arbitratedValue = SystemMode::Quiet;
		}
		else if (osPowerSlider == OsPowerSlider::BatterySaver)
		{
			m_arbitratedValue = SystemMode::Invalid;
		}
	}
	else if (osPowerSchemePersonality != OsPowerSchemePersonality::Invalid)
	{
		if (osPowerSchemePersonality == OsPowerSchemePersonality::HighPerformance)
		{
			m_arbitratedValue = SystemMode::Performance;
		}
		else if (osPowerSchemePersonality == OsPowerSchemePersonality::Balanced)
		{
			m_arbitratedValue = SystemMode::Balanced;
		}
		else if (osPowerSchemePersonality == OsPowerSchemePersonality::PowerSaver)
		{
			m_arbitratedValue = SystemMode::Quiet;
		}
	}

	m_arbitratedValueChangedSinceLastSet = (oldArbitratedValue != m_arbitratedValue);
}

void SystemModeManager::setPolicySystemModeValue(const SystemMode::Type systemMode)
{
	m_policySystemModeValue = systemMode;
	arbitrateAndCreateEventSystemModeChanged();
}

void SystemModeManager::executeOperatingSystemPowerSliderChanged()
{
	if (m_registeredEvents.test(FrameworkEvent::PolicyOperatingSystemPowerSliderChanged))
	{
		arbitrateAndCreateEventSystemModeChanged();
	}
}

void SystemModeManager::executeOperatingSystemPowerSchemePersonalityChanged()
{
	if (m_registeredEvents.test(FrameworkEvent::PolicyOperatingSystemPowerSchemePersonalityChanged))
	{
		arbitrateAndCreateEventSystemModeChanged();
	}
}

void SystemModeManager::executeOperatingSystemPowerSourceChanged(OsPowerSource::Type powerSource)
{
	if (m_registeredEvents.test(FrameworkEvent::PolicyOperatingSystemPowerSourceChanged))
	{
		m_powerSource = powerSource;
		arbitrateAndCreateEventSystemModeChanged();
	}
}

void SystemModeManager::executeIgccAppBroadcastReceived(
	IgccBroadcastData::IgccToDttNotificationPackage broadcastNotificationData)
{
	if (m_registeredEvents.test(FrameworkEvent::DptfAppBroadcastListen))
	{
		Guid broadcastGuid(broadcastNotificationData.uuid);

		MANAGER_LOG_MESSAGE_DEBUG(
			{ return "Application broadcast recieved. Broadcast header GUID: " + broadcastGuid.toString(); });
		if (IGCC_BROADCAST_GUID == broadcastGuid)
		{
			MANAGER_LOG_MESSAGE_DEBUG({
				return "IGCC application broadcast recieved. Struct info: "
					   + StatusFormat::friendlyValue(broadcastNotificationData.enduranceGamingStatus) + ","
					   + StatusFormat::friendlyValue(broadcastNotificationData.ppmSettingDirection) + ","
					   + StatusFormat::friendlyValue(broadcastNotificationData.ppmSettingLevel);
			});
			if (m_powerSource == OsPowerSource::DC)
			{
				m_isEnduranceGamingActive = broadcastNotificationData.enduranceGamingStatus;
			}
			else
			{
				m_isEnduranceGamingActive = false;
			}	
			arbitrateAndCreateEventSystemModeChanged();
		}
	}
}

void SystemModeManager::unregisterFrameworkEvents()
{
	unregisterEvent(FrameworkEvent::PolicyOperatingSystemPowerSliderChanged);
	unregisterEvent(FrameworkEvent::PolicyOperatingSystemPowerSchemePersonalityChanged);
	unregisterEvent(FrameworkEvent::DptfAppBroadcastListen);
	unregisterEvent(FrameworkEvent::DptfAppBroadcastSend);
	unregisterEvent(FrameworkEvent::PolicyOperatingSystemPowerSourceChanged);
}

void SystemModeManager::registerEvent(FrameworkEvent::Type frameworkEvent)
{
	try
	{
		if (m_registeredEvents.test(frameworkEvent) == false)
		{
			getEsifServices()->registerEvent(frameworkEvent);
		}
		m_registeredEvents.set(frameworkEvent);
	}
	catch (...)
	{
	}
}

void SystemModeManager::unregisterEvent(FrameworkEvent::Type frameworkEvent)
{
	try
	{
		m_registeredEvents.set(frameworkEvent, false);
		getEsifServices()->unregisterEvent(frameworkEvent);
	}
	catch (...)
	{
	}
}

EsifServicesInterface* SystemModeManager::getEsifServices()
{
	return m_dptfManager->getEsifServices();
}
