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

#include "RequestDispatcher.h"
using namespace std;

RequestDispatcher::RequestDispatcher()
	: m_handlers()
{
}

RequestDispatcher::~RequestDispatcher()
{
}

void RequestDispatcher::dispatchForAllControls(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto handlers = m_handlers[request.getRequestType()];
	for (auto handler = handlers.begin(); handler != handlers.end(); ++handler)
	{
		if ((*handler)->canProcessRequest(policyRequest))
		{
			(*handler)->processRequest(policyRequest);
		}
	}
}

DptfRequestResult RequestDispatcher::dispatch(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto handlers = m_handlers[request.getRequestType()];
	for (auto handler = handlers.begin(); handler != handlers.end(); ++handler)
	{
		if ((*handler)->canProcessRequest(policyRequest))
		{
			return (*handler)->processRequest(policyRequest);
		}
	}
	return DptfRequestResult(false, "No handler for request.", request);
}

void RequestDispatcher::registerHandler(DptfRequestType::Enum requestType, RequestHandlerInterface* handler)
{
	m_handlers[requestType].insert(handler);
}

void RequestDispatcher::unregisterHandler(DptfRequestType::Enum requestType, RequestHandlerInterface* handler)
{
	auto request = m_handlers.find(requestType);
	if (request != m_handlers.end())
	{
		auto handlerIterator = request->second.find(handler);
		if (handlerIterator != request->second.end())
		{
			request->second.erase(handlerIterator);
		}
	}
}
