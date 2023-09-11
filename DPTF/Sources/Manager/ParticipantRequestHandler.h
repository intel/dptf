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

#include "DptfManager.h"
#include "RequestHandlerInterface.h"
#include "PolicyRequest.h"
#include <functional>

#include "DptfRequestCreateParticipant.h"
#include "DptfRequestDeleteParticipant.h"

class dptf_export ParticipantRequestHandlerInterface : public RequestHandlerInterface
{
public:
	ParticipantRequestHandlerInterface() = default;
};

class dptf_export ParticipantRequestHandler : public ParticipantRequestHandlerInterface
{
public:
	ParticipantRequestHandler(DptfManagerInterface* dptfManager);
	virtual ~ParticipantRequestHandler();

	virtual Bool canProcessRequest(const PolicyRequest& policyRequest) override;
	virtual DptfRequestResult processRequest(const PolicyRequest& policyRequest) override;
	

private:
	DptfManagerInterface* m_dptfManager;
	std::shared_ptr<MessageLogger> m_messageLogger;
	std::map<DptfRequestType::Enum, std::function<DptfRequestResult(const PolicyRequest&)>> m_participantRequestHandlers;

	void bindRequestHandlers();
	void bindRequestHandler(
		DptfRequestType::Enum requestType,
		const std::function<DptfRequestResult(const PolicyRequest&)>& functionObj);
	void unbindRequestHandlers();
	DptfRequestResult callHandler(const PolicyRequest& policyRequest) const;
	static void throwIfDataIsEmpty(const DptfRequestCreateParticipant::RequestData* data);
	static void throwIfDataIsEmpty(const DptfRequestDeleteParticipant::RequestData* data);
	DptfRequestResult handleCreateParticipant(const PolicyRequest& policyRequest) const;
	DptfRequestResult handleDeleteParticipant(const PolicyRequest& policyRequest) const;

	EsifServicesInterface* getEsifServices() const;
};