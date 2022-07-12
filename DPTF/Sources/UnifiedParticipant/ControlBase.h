/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "ParticipantServicesInterface.h"
#include "PolicyRequest.h"
#include "ParticipantLogger.h"
#include <functional>

class XmlNode;

class ControlBase : public RequestHandlerInterface
{
public:
	ControlBase(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServices);
	virtual ~ControlBase();

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) = 0;
	virtual std::string getName(void) = 0;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) = 0;
	virtual std::shared_ptr<XmlNode> getArbitratorXml(UIntN policyIndex) const;

	Bool isActivityLoggingEnabled(void);
	void enableActivityLogging(void);
	void disableActivityLogging(void);

	void clearCachedData(void);
	void clearAllCachedResults(void);
	virtual DptfRequestResult processRequest(const PolicyRequest& policyRequest) override;
	virtual Bool canProcessRequest(const PolicyRequest& policyRequest) override;

protected:
	virtual void capture(void);
	virtual void restore(void);
	UIntN getParticipantIndex() const;
	UIntN getDomainIndex() const;
	std::shared_ptr<ParticipantServicesInterface> getParticipantServices() const;
	DptfBuffer createResetPrimitiveTupleBinary(esif_primitive_type primitive, UInt8 instance) const;
	Bool isMe(UIntN particpantIndex, UIntN domainIndex);
	void bindRequestHandler(
		DptfRequestType::Enum requestType,
		std::function<DptfRequestResult(const PolicyRequest&)> functionObj);

	// request cache.  TODO: put this in its own class
	Bool requestResultIsCached(const DptfRequest& request);
	const DptfRequestResult& getCachedResult(const DptfRequest& request) const;
	void clearCachedResult(const DptfRequest& request);
	void updateCachedResult(const DptfRequestResult& requestResult);
	void clearCachedResult(DptfRequestType::Enum requestType, UInt32 participantIndex, UInt32 domainIndex);

private:
	UIntN m_participantIndex;
	UIntN m_domainIndex;
	std::shared_ptr<ParticipantServicesInterface> m_participantServices;
	Bool m_activityLoggingEnabled;
	UInt16 createTupleDomain() const;
	DptfRequestResult callHandler(const PolicyRequest& policyRequest);
	void unbindRequestHandlers();
	std::map<DptfRequestType::Enum, std::function<DptfRequestResult(const PolicyRequest&)>> m_requestHandlers;
	std::map<std::tuple<DptfRequestType::Enum, UInt32, UInt32>, DptfRequestResult> m_requestCache;
};
