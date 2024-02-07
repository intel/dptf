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

#include "PlatformRequestHandler.h"
#include "PolicyManagerInterface.h"
#include "SystemModeManager.h"
#include "EsifServicesInterface.h"
#include "esif_ccb_memory.h"
#include "esif_sdk_data_misc.h"
#include "esif_sdk_primitive_type.h"
#include "StatusFormat.h"
#include "ManagerLogger.h"
#include "WorkItem.h"
#include "WIDomainFanOperatingModeChanged.h"
#include "WorkItemQueueManagerInterface.h"
#include "WIAll.h"
#include "ExtendedWorkloadPredictionEventPayload.h"
#include "EventPayloadApplicationOptimizationChanged.h"
using namespace StatusFormat;

#define OSC_REQUEST_FAIL 0xE
#define OSC_REQUEST_FAIL_UNABLE_TO_PROCESS 0x2
#define OSC_REQUEST_FAIL_UNRECOGNIZED_UUID 0x4
#define OSC_REQUEST_FAIL_UNRECOGNIZED_REVISON 0x8

const Guid DptfGuid(0x5D, 0xA8, 0x3B, 0xB2, 0xB7, 0xC8, 0x42, 0x35, 0x88, 0xDE, 0x8D, 0xE2, 0xFF, 0xCF, 0xD6, 0x98);

PlatformRequestHandler::PlatformRequestHandler(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
	, m_platformRequestHandlers()
{
	bindRequestHandlers();
}

PlatformRequestHandler::~PlatformRequestHandler()
{
	unbindRequestHandlers();
}

Bool PlatformRequestHandler::canProcessRequest(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto requestType = request.getRequestType();
	auto handler = m_platformRequestHandlers.find(requestType);

	return handler != m_platformRequestHandlers.end();
}

DptfRequestResult PlatformRequestHandler::processRequest(const PolicyRequest& policyRequest)
{
	if (canProcessRequest(policyRequest))
	{
		return callHandler(policyRequest);
	}
	else
	{
		throw dptf_exception("Handler not found for request.");
	}
}

void PlatformRequestHandler::bindRequestHandlers()
{
	bindRequestHandler(DptfRequestType::PlatformNotificationSetOsc,
		[=](const PolicyRequest& policyRequest) { return this->handleSetOsc(policyRequest);
	});
	bindRequestHandler(DptfRequestType::PlatformNotificationSetApplicationAliveResponse,
		[=](const PolicyRequest& policyRequest) { return this->handleSetApplicationAliveResponse(policyRequest);
	});
	bindRequestHandler(DptfRequestType::PlatformNotificationSetPolicySystemMode,
		[=](const PolicyRequest& policyRequest) { return this->handleSetSystemMode(policyRequest);
	});
	bindRequestHandler(DptfRequestType::PlatformNotificationAppBroadcastSend,
		[=](const PolicyRequest& policyRequest) { return this->handleAppBroadcastSend(policyRequest);
	});
	bindRequestHandler(DptfRequestType::PlatformNotificationFanOperatingModeChanged,
		[=](const PolicyRequest& policyRequest) { return this->handleFanOperatingModeChanged(policyRequest);
	});
	bindRequestHandler(DptfRequestType::PlatformNotificationEnableIpAlignment,
		[=](const PolicyRequest& policyRequest) { return this->handleEnableIpAlignment(policyRequest);
	});
	bindRequestHandler(DptfRequestType::PlatformNotificationDisableIpAlignment,
		[=](const PolicyRequest& policyRequest) { return this->handleDisableIpAlignment(policyRequest);
	});
	bindRequestHandler(DptfRequestType::PlatformNotificationStartIpAlignment,
		[=](const PolicyRequest& policyRequest) { return this->handleStartIpAlignment(policyRequest);
	});
	bindRequestHandler(DptfRequestType::PlatformNotificationStopIpAlignment,
		[=](const PolicyRequest& policyRequest) { return this->handleStopIpAlignment(policyRequest);
	});
	bindRequestHandler(DptfRequestType::PublishEvent,
		[=](const PolicyRequest& policyRequest) { return this->handlePublishEvent(policyRequest);
	});
}

void PlatformRequestHandler::bindRequestHandler(
	DptfRequestType::Enum requestType,
	std::function<DptfRequestResult(const PolicyRequest&)> functionObj)
{
	m_platformRequestHandlers[requestType] = functionObj;
	m_dptfManager->getRequestDispatcher()->registerHandler(requestType, this);
}

void PlatformRequestHandler::unbindRequestHandlers()
{
	m_platformRequestHandlers.clear();
}

DptfRequestResult PlatformRequestHandler::callHandler(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto requestType = request.getRequestType();
	auto handler = m_platformRequestHandlers.find(requestType);

	return handler->second(policyRequest);
}

DptfRequestResult PlatformRequestHandler::handleSetOsc(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	PolicyManagerInterface* policyManager = m_dptfManager->getPolicyManager();
	auto policyIndex = policyRequest.getPolicyIndex();
	auto policy = policyManager->getPolicyPtr(policyIndex);
	UInt32 oscInputCapabilitiesDWord = request.getDataAsUInt32();
	Bool isSuccessfulForPolicy = true;
	Bool isSuccessfulForArbitrator = true;
	std::string failureMessage;
	DptfRequestResult result;

	try
	{
		Guid policyGuid = policy->getGuid();
		executeOsc(policyGuid, oscInputCapabilitiesDWord);
	}
	catch (dptf_exception& ex)
	{
		isSuccessfulForPolicy = false;
		failureMessage = "Failure during execution of policy _OSC request: " + std::string(ex.what());
		MANAGER_LOG_MESSAGE_DEBUG_EX({ return failureMessage; });
	}

	try
	{
		m_oscArbitrator.updateRequest(policy->getName(), oscInputCapabilitiesDWord);
		if (m_oscArbitrator.arbitratedOscValueChanged())
		{
			UInt32 arbitratedOscValue = m_oscArbitrator.getArbitratedOscValue();
			MANAGER_LOG_MESSAGE_DEBUG({
				return "Requesting a new arbitrated _OSC value: " + std::bitset<4>(arbitratedOscValue).to_string();
			});
			executeOsc(DptfGuid, arbitratedOscValue);
		}
	}
	catch (dptf_exception& ex)
	{
		isSuccessfulForArbitrator = false;
		failureMessage =
			failureMessage + " Failure during execution of arbitrated _OSC request: " + std::string(ex.what());
		MANAGER_LOG_MESSAGE_DEBUG_EX({ return failureMessage; });
	}

	if (isSuccessfulForPolicy || isSuccessfulForArbitrator)
	{
		result = DptfRequestResult(
			true, "Successfully set _OSC to " + std::bitset<4>(oscInputCapabilitiesDWord).to_string() + ".", request);
	}
	else
	{
		result = DptfRequestResult(false, failureMessage, request);
	}

	return result;
}

DptfRequestResult PlatformRequestHandler::handleSetApplicationAliveResponse(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		auto data = request.getDataAsUInt32();
		EsifServicesInterface* esifServices = getEsifServices();
		esifServices->primitiveExecuteSetAsUInt32(SET_APP_ALIVE_RESPONSE, data);
		m_lastSetAppAliveResponse.set(data);

		return DptfRequestResult(true, "Successfully set app alive response.", request);
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, ex.what(), request);
	}
}

void PlatformRequestHandler::executeOsc(const Guid& guid, UInt32 oscCapabilities)
{
	Bool successful = false;

	struct esif_data_complex_osc osc;
	esif_ccb_memcpy(osc.guid, guid, Guid::GuidSize);
	osc.revision = 1;
	osc.count = 2;
	osc.status = 0;
	osc.capabilities = oscCapabilities;
	std::string failureMessage;

	try
	{
		EsifServicesInterface* esifServices = getEsifServices();
		esifServices->primitiveExecuteSet(
			SET_OPERATING_SYSTEM_CAPABILITIES,
			ESIF_DATA_STRUCTURE,
			&osc,
			sizeof(esif_data_complex_osc),
			sizeof(esif_data_complex_osc));
		successful = true;
	}
	catch (std::exception& ex)
	{
		failureMessage = ex.what();
	}
	catch (...)
	{
		failureMessage = "Unknown error.";
	}

	if (osc.status & OSC_REQUEST_FAIL)
	{
		if (osc.status & OSC_REQUEST_FAIL_UNABLE_TO_PROCESS)
		{
			throw dptf_exception("Platform supports _OSC but unable to process _OSC request.");
		}

		if (osc.status & OSC_REQUEST_FAIL_UNRECOGNIZED_UUID)
		{
			throw dptf_exception("Platform failed _OSC Reason: Unrecognized UUID.");
		}

		if (osc.status & OSC_REQUEST_FAIL_UNRECOGNIZED_REVISON)
		{
			throw dptf_exception("Platform failed _OSC Reason: Unrecognized revision.");
		}
	}

	if (successful == false)
	{
		throw dptf_exception("Failure during execution of _OSC: " + failureMessage);
	}
}

std::shared_ptr<XmlNode> PlatformRequestHandler::getXml()
{
	auto requestHandlerRoot = XmlNode::createWrapperElement("platform_request_handler");

	if (m_lastSetAppAliveResponse.isValid())
	{
		auto request = XmlNode::createWrapperElement("request");
		request->addChild(XmlNode::createDataElement("name", "Notify Platform"));
		request->addChild(XmlNode::createDataElement("value", friendlyValue(m_lastSetAppAliveResponse.get())));
		requestHandlerRoot->addChild(request);
	}

	requestHandlerRoot->addChild(m_oscArbitrator.getOscArbitratorStatus());

	return requestHandlerRoot;
}

DptfRequestResult PlatformRequestHandler::handleSetSystemMode(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		SystemModeManagerInterface* systemModeManager = m_dptfManager->getSystemModeManager();
		auto systemMode = SystemMode::toType(request.getDataAsUInt32());
		if (systemMode != SystemMode::Invalid)
		{
			systemModeManager->setPolicySystemModeValue(systemMode);
		}
		else
		{
			throw dptf_exception("Invalid system mode value.");
		}

		return DptfRequestResult(true, "Successfully set system mode value.", request);
	}
	catch (dptf_exception& ex)
	{
		std::string failureMessage = "Failure during execution of set system mode request: " + std::string(ex.what());
		return DptfRequestResult(false, failureMessage, request);
	}
}

DptfRequestResult PlatformRequestHandler::handleAppBroadcastSend(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		EsifData eventData;
		eventData.buf_ptr = request.getData().get();
		eventData.buf_len = request.getData().size();
		eventData.type = ESIF_DATA_BINARY;
		eventData.data_len = request.getData().size();

		EsifAppBroadcastHeader* broadcastNotificationDataHeader =
			static_cast<EsifAppBroadcastHeader*> (eventData.buf_ptr);
		Guid broadcastGuid(broadcastNotificationDataHeader->UUID);
		if (broadcastGuid == IGCC_SEND_GUID)
		{
			IgccBroadcastData::DttToIgccSendPackage* dttToigccSendData =
				static_cast<IgccBroadcastData::DttToIgccSendPackage*>(eventData.buf_ptr);
			if (dttToigccSendData->enduranceGamingStatus != EnduranceGamingStatus::EnduranceGamingOn)
			{
				SystemModeManagerInterface* systemModeManager = m_dptfManager->getSystemModeManager();
				systemModeManager->arbitrateAndCreateEventSystemModeChanged(); 
			}
		}
		getEsifServices()->sendDptfEvent(
			FrameworkEvent::DptfAppBroadcastPrivileged, Constants::Invalid, Constants::Invalid, eventData);
		return DptfRequestResult(true, "Successfully sent appbroadcast event", request);
	}
	catch (dptf_exception& ex)
	{
		std::string failureMessage = "Failed to send appbroadcast event" + std::string(ex.what());
		return DptfRequestResult(false, failureMessage, request);
	}
}

DptfRequestResult PlatformRequestHandler::handleFanOperatingModeChanged(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		auto operatingMode = FanOperatingMode::toType(request.getDataAsUInt32());
		auto participantIndex = request.getParticipantIndex();
		auto domainIndex = request.getDomainIndex();
		MANAGER_LOG_MESSAGE_DEBUG(
			{ return "Fan Operating Mode changed to: " + FanOperatingMode::toString(operatingMode); });

		std::shared_ptr<WorkItem> wi = std::make_shared<WIDomainFanOperatingModeChanged>(m_dptfManager, participantIndex, domainIndex,
			operatingMode);
		m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(wi);

		return DptfRequestResult(true, "Successfully sent fan operating mode changed event to policies.", request);
	}
	catch (dptf_exception& ex)
	{
		std::string failureMessage =
			"Failure during sending fan operating mode changed event to policies: " + std::string(ex.what());
		return DptfRequestResult(false, failureMessage, request);
	}
}

DptfRequestResult PlatformRequestHandler::handlePublishEvent(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		auto participantIndex = request.getParticipantIndex();
		auto domainIndex = request.getDomainIndex();
		const auto data = request.getData();
		const auto frameworkEvent = static_cast<FrameworkEvent::Type>(request.getFirstUInt32Data());
		std::shared_ptr<WorkItem> wi;

		switch (frameworkEvent)
		{
		case FrameworkEvent::DomainExtendedWorkloadPredictionChanged:
		{
			if (data.size() >= sizeof(ExtendedWorkloadPredictionEventPayload))
			{
				ExtendedWorkloadPredictionEventPayload* extendedWorkloadPredictionEventPayload =
					reinterpret_cast<ExtendedWorkloadPredictionEventPayload*>(data.get());

				wi = std::make_shared<WIDomainExtendedWorkloadPredictionChanged>(
					m_dptfManager,
					participantIndex,
					domainIndex,
					extendedWorkloadPredictionEventPayload->extendedWorkloadPrediction);
			}
			break;
		}
		case FrameworkEvent::PolicyApplicationOptimizationChanged:
		{
			const auto eventPayload = reinterpret_cast<EventPayloadApplicationOptimizationChanged*>(data.get());
			if (eventPayload)
			{
				wi = std::make_shared<WIPolicyApplicationOptimizationChanged>(
					m_dptfManager,
					eventPayload->isActive);
			}
			break;
		}
		default:
			return DptfRequestResult(false, "Unsupported event given in publish request", request);
		}

		if (wi)
		{
			m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(wi);

			return DptfRequestResult(true, "Successfully publish event.", request);
		}

		return DptfRequestResult(false, "Failed to publish event.", request);

	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, ex.what(), request);
	}
}

DptfRequestResult PlatformRequestHandler::handleEnableIpAlignment(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		// Pass in a dummy value (e.g. 1) to prevent nullptr exception for SET primitive
		getEsifServices()->primitiveExecuteSetAsUInt32(SET_IP_ALIGNMENT_ENABLE, 1);
		return DptfRequestResult(true, "Successfully enabled IP Alignment", request);
	}
	catch (dptf_exception& ex)
	{
		std::string failureMessage = "Failed to enable IP Alignment" + std::string(ex.what());
		return DptfRequestResult(false, failureMessage, request);
	}
}

DptfRequestResult PlatformRequestHandler::handleDisableIpAlignment(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		// Pass in a dummy value (e.g. 1) to prevent nullptr exception for SET primitive
		getEsifServices()->primitiveExecuteSetAsUInt32(SET_IP_ALIGNMENT_DISABLE, 1);
		return DptfRequestResult(true, "Successfully disabled IP Alignment", request);
	}
	catch (dptf_exception& ex)
	{
		std::string failureMessage = "Failed to disable IP Alignment" + std::string(ex.what());
		return DptfRequestResult(false, failureMessage, request);
	}
}

DptfRequestResult PlatformRequestHandler::handleStartIpAlignment(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		// Pass in a dummy value (e.g. 1) to prevent nullptr exception for SET primitive
		getEsifServices()->primitiveExecuteSetAsUInt32(SET_IP_ALIGNMENT_START, 1);
		return DptfRequestResult(true, "Successfully started IP Alignment", request);
	}
	catch (dptf_exception& ex)
	{
		std::string failureMessage = "Failed to start IP Alignment" + std::string(ex.what());
		return DptfRequestResult(false, failureMessage, request);
	}
}

DptfRequestResult PlatformRequestHandler::handleStopIpAlignment(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		// Pass in a dummy value (e.g. 1) to prevent nullptr exception for SET primitive
		getEsifServices()->primitiveExecuteSetAsUInt32(SET_IP_ALIGNMENT_STOP, 1);
		return DptfRequestResult(true, "Successfully stopped IP Alignment", request);
	}
	catch (dptf_exception& ex)
	{
		std::string failureMessage = "Failed to stop IP Alignment" + std::string(ex.what());
		return DptfRequestResult(false, failureMessage, request);
	}
}

EsifServicesInterface* PlatformRequestHandler::getEsifServices()
{
	return m_dptfManager->getEsifServices();
}
