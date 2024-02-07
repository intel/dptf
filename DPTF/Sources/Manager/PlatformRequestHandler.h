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

#pragma once

#include "DptfManager.h"
#include "RequestHandlerInterface.h"
#include "PolicyRequest.h"
#include "ArbitratorOscRequest.h"
#include <functional>

class dptf_export PlatformRequestHandlerInterface : public RequestHandlerInterface
{
public:
	PlatformRequestHandlerInterface() = default;
	virtual std::shared_ptr<XmlNode> getXml() = 0;
};

class dptf_export PlatformRequestHandler : public PlatformRequestHandlerInterface
{
public:
	PlatformRequestHandler(DptfManagerInterface* dptfManager);
	virtual ~PlatformRequestHandler();

	virtual Bool canProcessRequest(const PolicyRequest& policyRequest) override;
	virtual DptfRequestResult processRequest(const PolicyRequest& policyRequest) override;
	virtual std::shared_ptr<XmlNode> getXml() override;

private:
	DptfManagerInterface* m_dptfManager;
	std::map<DptfRequestType::Enum, std::function<DptfRequestResult(const PolicyRequest&)>> m_platformRequestHandlers;
	CachedValue<UInt32> m_lastSetAppAliveResponse;
	ArbitratorOscRequest m_oscArbitrator;

	void bindRequestHandlers();
	void bindRequestHandler(
		DptfRequestType::Enum requestType,
		std::function<DptfRequestResult(const PolicyRequest&)> functionObj);
	void unbindRequestHandlers();
	DptfRequestResult callHandler(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetOsc(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetApplicationAliveResponse(const PolicyRequest& request);
	DptfRequestResult handleSetSystemMode(const PolicyRequest& request);
	DptfRequestResult handleAppBroadcastSend(const PolicyRequest& request);
	DptfRequestResult handleFanOperatingModeChanged(const PolicyRequest& policyRequest);
	DptfRequestResult handlePublishEvent(const PolicyRequest& policyRequest);
	DptfRequestResult handleEnableIpAlignment(const PolicyRequest& policyRequest);
	DptfRequestResult handleDisableIpAlignment(const PolicyRequest& policyRequest);
	DptfRequestResult handleStartIpAlignment(const PolicyRequest& policyRequest);
	DptfRequestResult handleStopIpAlignment(const PolicyRequest& policyRequest);

	void executeOsc(const Guid& guid, UInt32 oscCapabilities);
	EsifServicesInterface* getEsifServices();
};
