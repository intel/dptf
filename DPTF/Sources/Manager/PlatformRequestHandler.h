#pragma once

#include "DptfManager.h"
#include "RequestHandlerInterface.h"
#include "PolicyRequest.h"
#include "ArbitratorOscRequest.h"
#include <functional>

class dptf_export PlatformRequestHandler : public RequestHandlerInterface
{
public:
	PlatformRequestHandler(DptfManagerInterface* dptfManager);
	virtual ~PlatformRequestHandler();

	virtual Bool canProcessRequest(const PolicyRequest& policyRequest) override;
	virtual DptfRequestResult processRequest(const PolicyRequest& policyRequest) override;
	std::shared_ptr<XmlNode> getXml();

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

	void executeOsc(const Guid& guid, UInt32 oscCapabilities);
	EsifServicesInterface* getEsifServices();
};
