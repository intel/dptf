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
#include "PolicyRequest.h"
#include "RequestHandlerInterface.h"
#include "DptfRequestResult.h"

class RequestDispatcherInterface
{
public:
	virtual ~RequestDispatcherInterface(){};

	virtual DptfRequestResult dispatch(const PolicyRequest& policyRequest) = 0;
	virtual void dispatchForAllControls(const PolicyRequest& policyRequest) = 0;
	virtual void registerHandler(DptfRequestType::Enum requestType, RequestHandlerInterface* handler) = 0;
	virtual void unregisterHandler(DptfRequestType::Enum requestType, RequestHandlerInterface* handler) = 0;
};

class RequestDispatcher : public RequestDispatcherInterface
{
public:
	RequestDispatcher();
	virtual ~RequestDispatcher();

	virtual DptfRequestResult dispatch(const PolicyRequest& policyRequest) override;
	virtual void dispatchForAllControls(const PolicyRequest& policyRequest) override;
	virtual void registerHandler(DptfRequestType::Enum requestType, RequestHandlerInterface* handler) override;
	virtual void unregisterHandler(DptfRequestType::Enum requestType, RequestHandlerInterface* handler) override;

private:
	std::map<DptfRequestType::Enum, std::set<RequestHandlerInterface*>> m_handlers;
};
