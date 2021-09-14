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

#include "WIDptfIgccBroadcastReceived.h"
#include "PolicyManagerInterface.h"
#include "SystemModeManager.h"
#include "EsifServicesInterface.h"
#include "StatusFormat.h"

WIDptfIgccBroadcastReceived::WIDptfIgccBroadcastReceived(
	DptfManagerInterface* dptfManager,
	IgccBroadcastData::IgccToDttNotificationPackage igccNotificationData)
	: WorkItem(dptfManager, FrameworkEvent::DptfAppBroadcastListen)
	, m_igccNotificationData(igccNotificationData)
{
}

WIDptfIgccBroadcastReceived::~WIDptfIgccBroadcastReceived(void)
{
}

void WIDptfIgccBroadcastReceived::onExecute(void)
{
	writeWorkItemStartingInfoMessage();
	Guid broadcastGuid(m_igccNotificationData.uuid);
	if (IGCC_BROADCAST_GUID == broadcastGuid)
	{
		OsPowerSource::Type powerSource = getDptfManager()->getEventCache()->powerSource.get();
		try
		{
			auto systemModeManager = getDptfManager()->getSystemModeManager();
			systemModeManager->executeIgccAppBroadcastReceived(m_igccNotificationData);
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessage(ex, "SystemModeManager::executeIgccAppBroadcastReceived");
		}

		if (m_igccNotificationData.enduranceGamingStatus == 0)
		{
			try
			{
				sendEnduranceGamingOffAppBroadcastEvent(ENDURANCE_GAMING_OFF);
			}
			catch (std::exception& ex)
			{
				writeWorkItemErrorMessage(ex, "SystemModeManager::sendEnduranceGamingOffAppBroadcastEvent");
			}
		}
		else if (powerSource == OsPowerSource::AC)
		{
			sendEnduranceGamingOffAppBroadcastEvent(ENDURANCE_GAMING_ERROR_AC);
		}
		else
		{
			auto policyManager = getPolicyManager();
			auto policyIndexes = policyManager->getPolicyIndexes();

			for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
			{
				try
				{
					getDptfManager()->getEventCache()->appBroadcastNotificationData.set(m_igccNotificationData);
					auto policy = policyManager->getPolicyPtr(*i);
					policy->executeIgccBroadcastReceived(m_igccNotificationData);
				}
				catch (policy_index_invalid&)
				{
					// do nothing.  No item in the policy list at this index.
				}
				catch (std::exception& ex)
				{
					writeWorkItemErrorMessagePolicy(ex, "Policy::executeIgccBroadcastReceived", *i);
				}
			}
		}
	}
}

void WIDptfIgccBroadcastReceived::sendEnduranceGamingOffAppBroadcastEvent(UInt8 egStatus)
{
	try
	{
		IgccBroadcastData::DttToIgccSendPackage igccData;
		IGCC_SEND_GUID.copyToBuffer(igccData.uuid);
		igccData.enduranceGamingStatus = egStatus;
		igccData.currentPpmSettingLevel = INVALID_PPM;
		igccData.totalAvailableLevels = IGCC_TOTAL_AVAILABLE_LEVELS;
		igccData.dataPayloadLength = IGCC_DATA_PAYLOAD_LENGTH;

		EsifData eventData;
		eventData.buf_ptr = &igccData;
		eventData.buf_len = sizeof(igccData);
		eventData.type = ESIF_DATA_BINARY;
		eventData.data_len = sizeof(igccData);

		getEsifServices()->sendDptfEvent(
			FrameworkEvent::DptfAppBroadcastSend, Constants::Invalid, Constants::Invalid, eventData);
		MANAGER_LOG_MESSAGE_DEBUG({
			return "enduranceGamingStatus false, send back IGCC application broadcast with EG status. Struct info: "
				   + StatusFormat::friendlyValue(igccData.enduranceGamingStatus) + ","
				   + StatusFormat::friendlyValue(igccData.currentPpmSettingLevel) + ","
				   + StatusFormat::friendlyValue(igccData.totalAvailableLevels);
		});
	}
	catch (dptf_exception& ex)
	{
		std::string failureMessage = "Failed to send notification to IGCC" + std::string(ex.what());
		MANAGER_LOG_MESSAGE_WARNING_EX({ return failureMessage; });
	}
}
