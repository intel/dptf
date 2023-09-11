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

#include "ParticipantRequestHandler.h"

#include "DptfRequestCreateParticipant.h"
#include "DptfRequestDeleteParticipant.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"
#include "esif_sdk_iface_conjure.h"
#include "esif_sdk_primitive_type.h"
#include "ManagerLogger.h"
#include "EsifDataCreateParticipant.h"
using namespace std;

ParticipantRequestHandler::ParticipantRequestHandler(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
	, m_participantRequestHandlers()
{
	bindRequestHandlers();
}

ParticipantRequestHandler::~ParticipantRequestHandler()
{
	unbindRequestHandlers();
}

Bool ParticipantRequestHandler::canProcessRequest(const PolicyRequest& policyRequest)
{
	const auto& request = policyRequest.getRequest();
	const auto requestType = request.getRequestType();
	const auto handler = m_participantRequestHandlers.find(requestType);
	return handler != m_participantRequestHandlers.end();
}

DptfRequestResult ParticipantRequestHandler::processRequest(const PolicyRequest& policyRequest)
{
	if (canProcessRequest(policyRequest))
	{
		return callHandler(policyRequest);
	}
	throw dptf_exception("Handler not found for request.");
}

void ParticipantRequestHandler::bindRequestHandlers()
{
	bindRequestHandler(
		DptfRequestType::CreateParticipant,
		[=](const PolicyRequest& policyRequest) { return this->handleCreateParticipant(policyRequest); });
	bindRequestHandler(
		DptfRequestType::DeleteParticipant,
		[=](const PolicyRequest& policyRequest) { return this->handleDeleteParticipant(policyRequest); });
}

void ParticipantRequestHandler::bindRequestHandler(
	DptfRequestType::Enum requestType,
	const std::function<DptfRequestResult(const PolicyRequest&)>& functionObj)
{
	m_participantRequestHandlers[requestType] = functionObj;
	m_dptfManager->getRequestDispatcher()->registerHandler(requestType, this);
}

void ParticipantRequestHandler::unbindRequestHandlers()
{
	m_participantRequestHandlers.clear();
}

DptfRequestResult ParticipantRequestHandler::callHandler(const PolicyRequest& policyRequest) const
{
	const auto& request = policyRequest.getRequest();
	const auto requestType = request.getRequestType();
	const auto handler = m_participantRequestHandlers.find(requestType);
	if (handler != m_participantRequestHandlers.end())
	{
		return handler->second(policyRequest);
	}
	throw dptf_exception("Handler not found for request.");
}

EsifServicesInterface* ParticipantRequestHandler::getEsifServices() const
{
	return m_dptfManager->getEsifServices();
}

DptfRequestResult ParticipantRequestHandler::handleCreateParticipant(const PolicyRequest& policyRequest) const
{
	try
	{
		const auto& request = policyRequest.getRequest();
		const auto data = reinterpret_cast<const DptfRequestCreateParticipant::RequestData*>(
			request.getData().get());
		throwIfDataIsEmpty(data);

		const EsifDataCreateParticipant eventData(*data);
		getEsifServices()->sendDptfEvent(
			FrameworkEvent::DptfSendParticipantCreate, 
			Constants::Esif::NoParticipant, 
			Constants::Esif::NoDomain, 
			eventData.getEsifData());
		return {true, "Successfully sent event for create participant", policyRequest.getRequest()};
	}
	catch (dptf_exception& ex)
	{
		std::string failureMessage = "Failed to send event for create participant" + std::string(ex.what());
		return {false, failureMessage, policyRequest.getRequest()};
	}
}

DptfRequestResult ParticipantRequestHandler::handleDeleteParticipant(const PolicyRequest& policyRequest) const
{
	try
	{
		const auto& request = policyRequest.getRequest();
		const auto data = reinterpret_cast<const DptfRequestDeleteParticipant::RequestData*>(request.getData().get());
		throwIfDataIsEmpty(data);
		return {false, "Delete participant not supported yet", policyRequest.getRequest()};
	}
	catch (dptf_exception& ex)
	{
		std::string failureMessage = "Failed to send event for delete participant" + std::string(ex.what());
		return {false, failureMessage, policyRequest.getRequest()};
	}
}

void ParticipantRequestHandler::throwIfDataIsEmpty(const DptfRequestCreateParticipant::RequestData* data)
{
	if (!data)
	{
		throw dptf_exception("Request data is empty"s);
	}
}

void ParticipantRequestHandler::throwIfDataIsEmpty(const DptfRequestDeleteParticipant::RequestData* data)
{
	if (!data)
	{
		throw dptf_exception("Request data is empty"s);
	}
}