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

#include "ControlBase.h"
#include "esif_sdk_data_misc.h"
#include "XmlNode.h"
#include <functional>
using namespace std;

ControlBase::ControlBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServices)
	: m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_participantServices(participantServices)
	, m_activityLoggingEnabled(false)
	, m_requestHandlers()
	, m_requestCache()
{
}

ControlBase::~ControlBase()
{
	unbindRequestHandlers();
}

void ControlBase::capture(void)
{
	// do nothing
}

void ControlBase::restore(void)
{
	// do nothing
}

UIntN ControlBase::getParticipantIndex() const
{
	return m_participantIndex;
}

UIntN ControlBase::getDomainIndex() const
{
	return m_domainIndex;
}

Bool ControlBase::isActivityLoggingEnabled(void)
{
	return m_activityLoggingEnabled;
}

void ControlBase::enableActivityLogging(void)
{
	m_activityLoggingEnabled = true;
}

void ControlBase::disableActivityLogging(void)
{
	m_activityLoggingEnabled = false;
}

DptfRequestResult ControlBase::processRequest(const PolicyRequest& policyRequest)
{
	if (canProcessRequest(policyRequest))
	{
		return callHandler(policyRequest);
	}
	else
	{
		throw dptf_exception("Request sent to wrong control.");
	}
}

Bool ControlBase::canProcessRequest(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto participantIndex = request.getParticipantIndex();
	auto domainIndex = request.getDomainIndex();
	auto requestType = request.getRequestType();
	auto handler = m_requestHandlers.find(requestType);
	Bool hasHandlerForRequestType = (handler != m_requestHandlers.end());
	Bool requestTargetsThisControl = isMe(participantIndex, domainIndex);
	return hasHandlerForRequestType && requestTargetsThisControl;
}

DptfRequestResult ControlBase::callHandler(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();
	auto requestType = request.getRequestType();
	auto handler = m_requestHandlers.find(requestType);
	if (handler == m_requestHandlers.end())
	{
		return DptfRequestResult(false, "Handler not found for request.", request);
	}
	else
	{
		try
		{
			return handler->second(policyRequest);
		}
		catch (const std::exception& ex)
		{
			return DptfRequestResult(false, ex.what(), request);
		}
	}
}

void ControlBase::unbindRequestHandlers()
{
	for (auto handler = m_requestHandlers.begin(); handler != m_requestHandlers.end(); ++handler)
	{
		try
		{
			getParticipantServices()->unregisterRequestHandler(handler->first, this);
		}
		catch (...)
		{
			// ignore
		}
	}
	m_requestHandlers.clear();
}

std::shared_ptr<ParticipantServicesInterface> ControlBase::getParticipantServices() const
{
	return m_participantServices;
}

DptfBuffer ControlBase::createResetPrimitiveTupleBinary(esif_primitive_type primitive, UInt8 instance) const
{
	esif_primitive_tuple_parameter tuple;
	tuple.id.integer.type = esif_data_type::ESIF_DATA_UINT16;
	tuple.id.integer.value = primitive;
	tuple.domain.integer.type = esif_data_type::ESIF_DATA_UINT16;
	UInt16 domainIndex = createTupleDomain();
	tuple.domain.integer.value = domainIndex;
	tuple.instance.integer.type = esif_data_type::ESIF_DATA_UINT16;
	tuple.instance.integer.value = instance;

	UInt32 sizeOfTuple = (UInt32)sizeof(tuple);
	DptfBuffer buffer(sizeOfTuple);
	buffer.put(0, (UInt8*)&tuple, sizeOfTuple);
	return buffer;
}

Bool ControlBase::isMe(UIntN particpantIndex, UIntN domainIndex)
{
	return (particpantIndex == getParticipantIndex()) && (domainIndex == getDomainIndex());
}

void ControlBase::bindRequestHandler(
	DptfRequestType::Enum requestType,
	std::function<DptfRequestResult(const PolicyRequest&)> functionObj)
{
	m_requestHandlers[requestType] = functionObj;
	getParticipantServices()->registerRequestHandler(requestType, this);
}

UInt16 ControlBase::createTupleDomain() const
{
	UInt16 tupleDomain;
	tupleDomain = (('0' + (UInt8)m_domainIndex) << 8) + 'D';
	return tupleDomain;
}

void ControlBase::clearCachedData()
{
	clearAllCachedResults();
	onClearCachedData();
}

Bool ControlBase::requestResultIsCached(const DptfRequest& request)
{
	auto cachedItem = m_requestCache.find(tuple<DptfRequestType::Enum, UInt32, UInt32>(
		request.getRequestType(), request.getParticipantIndex(), request.getDomainIndex()));
	return (cachedItem != m_requestCache.end());
}

const DptfRequestResult& ControlBase::getCachedResult(const DptfRequest& request) const
{
	auto cachedItem = m_requestCache.find(tuple<DptfRequestType::Enum, UInt32, UInt32>(
		request.getRequestType(), request.getParticipantIndex(), request.getDomainIndex()));
	if (cachedItem == m_requestCache.end())
	{
		throw dptf_exception("No cached result for request.");
	}
	else
	{
		return cachedItem->second;
	}
}

void ControlBase::clearCachedResult(const DptfRequest& request)
{
	tuple<DptfRequestType::Enum, UInt32, UInt32> key(
		request.getRequestType(), request.getParticipantIndex(), request.getDomainIndex());
	m_requestCache.erase(key);
}

void ControlBase::clearCachedResult(DptfRequestType::Enum requestType, UInt32 participantIndex, UInt32 domainIndex)
{
	tuple<DptfRequestType::Enum, UInt32, UInt32> key(requestType, participantIndex, domainIndex);
	m_requestCache.erase(key);
}

void ControlBase::updateCachedResult(const DptfRequestResult& requestResult)
{
	auto& request = requestResult.getRequest();
	tuple<DptfRequestType::Enum, UInt32, UInt32> key(
		request.getRequestType(), request.getParticipantIndex(), request.getDomainIndex());
	m_requestCache[key] = requestResult;
}

void ControlBase::clearAllCachedResults()
{
	m_requestCache.clear();
}

std::shared_ptr<XmlNode> ControlBase::getArbitratorXml(UIntN policyIndex) const
{
	throw not_implemented();
}
