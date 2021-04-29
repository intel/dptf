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

#include "PlatformRequestHandler.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"
#include "esif_ccb_memory.h"
#include "esif_sdk_data_misc.h"
#include "esif_sdk_primitive_type.h"
#include "StatusFormat.h"
#include "ManagerLogger.h"
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
	bindRequestHandler(DptfRequestType::PlatformNotificationSetOsc, [=](const PolicyRequest& policyRequest) {
		return this->handleSetOsc(policyRequest);
	});
	bindRequestHandler(
		DptfRequestType::PlatformNotificationSetApplicationAliveResponse,
		[=](const PolicyRequest& policyRequest) { return this->handleSetApplicationAliveResponse(policyRequest); });
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

EsifServicesInterface* PlatformRequestHandler::getEsifServices()
{
	return m_dptfManager->getEsifServices();
}
