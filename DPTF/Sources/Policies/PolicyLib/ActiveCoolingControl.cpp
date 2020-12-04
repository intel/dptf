/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "ActiveCoolingControl.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

ActiveCoolingControl::ActiveCoolingControl(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const ParticipantProperties& participantProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: m_policyServices(policyServices)
	, m_domainProperties(domainProperties)
	, m_participantProperties(participantProperties)
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_staticCaps(participantIndex, domainIndex, domainProperties, policyServices)
	, m_dynamicCaps(participantIndex, domainIndex, domainProperties, policyServices)
	, m_fanSpeedRequestTable(std::map<UIntN, Percentage>())
	, m_lastFanSpeedRequest(Percentage::createInvalid())
{
}

ActiveCoolingControl::~ActiveCoolingControl(void)
{
}

Bool ActiveCoolingControl::supportsActiveCoolingControls()
{
	return m_domainProperties.implementsActiveControlInterface();
}

Bool ActiveCoolingControl::supportsFineGrainControl()
{
	if (supportsActiveCoolingControls())
	{
		return m_staticCaps.getCapabilities().supportsFineGrainedControl();
	}
	return false;
}

void ActiveCoolingControl::requestFanSpeedPercentage(UIntN requestorIndex, const Percentage& fanSpeed)
{
	if (supportsFineGrainControl())
	{
		updateFanSpeedRequestTable(requestorIndex, snapToCapabilitiesBounds(fanSpeed));
		setHighestFanSpeedPercentage();
	}
}

void ActiveCoolingControl::updateFanSpeedRequestTable(UIntN requestorIndex, const Percentage& fanSpeed)
{
	if (fanSpeed != Percentage::createInvalid())
	{
		if (m_fanSpeedRequestTable.count(requestorIndex))
		{
			m_fanSpeedRequestTable.at(requestorIndex) = fanSpeed;
		}
		else
		{
			m_fanSpeedRequestTable.insert(pair<UIntN, Percentage>(requestorIndex, fanSpeed));
		}
	}
	else
	{
		clearFanSpeedRequestForTarget(requestorIndex);
	}
}

void ActiveCoolingControl::clearFanSpeedRequestForTarget(UIntN requestorIndex)
{
	if (m_fanSpeedRequestTable.count(requestorIndex))
	{
		m_fanSpeedRequestTable.erase(requestorIndex);
	}
}

void ActiveCoolingControl::setHighestFanSpeedPercentage()
{
	Percentage highestFanSpeed = chooseHighestFanSpeedRequest();
	if (highestFanSpeed != m_lastFanSpeedRequest)
	{
		if (m_fanSpeedRequestTable.empty())
		{
			m_lastFanSpeedRequest = Percentage::createInvalid();
		}
		else
		{
			m_lastFanSpeedRequest = highestFanSpeed;
		}
		setFanSpeed(highestFanSpeed);
	}
}

Percentage ActiveCoolingControl::chooseHighestFanSpeedRequest()
{
	Percentage highestFanSpeed(0.0);
	for (auto request = m_fanSpeedRequestTable.begin(); request != m_fanSpeedRequestTable.end(); request++)
	{
		if (request->second > highestFanSpeed)
		{
			highestFanSpeed = request->second;
		}
	}
	return highestFanSpeed;
}

void ActiveCoolingControl::forceFanOff(void)
{
	if (supportsFineGrainControl())
	{
		Percentage minFanSpeed(0.0);
		setFanSpeed(minFanSpeed);
		m_lastFanSpeedRequest = Percentage::createInvalid();
	}
}

std::shared_ptr<XmlNode> ActiveCoolingControl::getXml()
{
	auto status = XmlNode::createWrapperElement("active_cooling_control");
	status->addChild(XmlNode::createDataElement("participant_index", friendlyValue(m_participantIndex)));
	status->addChild(XmlNode::createDataElement("domain_index", friendlyValue(m_domainIndex)));
	status->addChild(XmlNode::createDataElement("name", m_participantProperties.getAcpiInfo().getAcpiScope()));
	status->addChild(XmlNode::createDataElement("max", m_dynamicCaps.getCapabilities().getMaxFanSpeed().toString()));
	if (supportsFineGrainControl())
	{
		status->addChild(XmlNode::createDataElement("speed", m_lastFanSpeedRequest.toString()));
	}
	status->addChild(XmlNode::createDataElement("min", m_dynamicCaps.getCapabilities().getMinFanSpeed().toString()));
	status->addChild(XmlNode::createDataElement("fine_grain", friendlyValue(supportsFineGrainControl())));
	return status;
}

void ActiveCoolingControl::setControl(Percentage activeCoolingControlFanSpeed)
{
	if (supportsActiveCoolingControls())
	{
		setFanSpeed(activeCoolingControlFanSpeed);
	}
	else
	{
		throw dptf_exception("Domain does not support the active control fan interface.");
	}
}

void ActiveCoolingControl::refreshCapabilities()
{
	m_staticCaps.refresh();
	m_dynamicCaps.refresh();
}

const ActiveControlStaticCaps& ActiveCoolingControl::getCapabilities()
{
	return m_staticCaps.getCapabilities();
}

const ActiveControlDynamicCaps& ActiveCoolingControl::getDynamicCapabilities()
{
	return m_dynamicCaps.getCapabilities();
}

ActiveControlStatus ActiveCoolingControl::getStatus()
{
	if (supportsActiveCoolingControls())
	{
		DptfRequest request(DptfRequestType::ActiveControlGetStatus, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();
		return ActiveControlStatus::createFromFst(result.getData());
	}
	else
	{
		throw dptf_exception("Domain does not support the active control fan interface.");
	}
}

UIntN ActiveCoolingControl::getSmallestNonZeroFanSpeed()
{
	auto controlSet = getActiveControlSet();
	return controlSet.getSmallestNonZeroFanSpeed();
}

Bool ActiveCoolingControl::hasValidActiveControlSet()
{
	// active control set is empty or has only one entry of value 0
	auto controlSet = getActiveControlSet();
	if ((controlSet.getCount() == 0) || (controlSet.getSmallestNonZeroFanSpeed() == 0))
	{
		return false;
	}
	return true;
}

void ActiveCoolingControl::setActiveControlDynamicCaps(ActiveControlDynamicCaps newCapabilities)
{
	if (supportsActiveCoolingControls())
	{
		DptfRequest request(
			DptfRequestType::ActiveControlSetDynamicCaps,
			newCapabilities.toFcdcBinary(),
			m_participantIndex,
			m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();
		refreshCapabilities();
	}
	else
	{
		throw dptf_exception("Domain does not support the active control interface.");
	}
}

void ActiveCoolingControl::lockCapabilities()
{
	if (supportsActiveCoolingControls())
	{
		DptfRequest request(
			DptfRequestType::ActiveControlSetFanCapsLock,
			DptfBuffer::fromBool(true),
			m_participantIndex,
			m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();
	}
	else
	{
		throw dptf_exception("Domain does not support the active control fan interface.");
	}
}

void ActiveCoolingControl::unlockCapabilities()
{
	if (supportsActiveCoolingControls())
	{
		DptfRequest request(
			DptfRequestType::ActiveControlSetFanCapsLock,
			DptfBuffer::fromBool(false),
			m_participantIndex,
			m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();
	}
	else
	{
		throw dptf_exception("Domain does not support the active control fan interface.");
	}
}

void ActiveCoolingControl::setFanSpeed(const Percentage& fanSpeed)
{
	DptfRequest request(
		DptfRequestType::ActiveControlSetFanSpeed, fanSpeed.toDptfBuffer(), m_participantIndex, m_domainIndex);
	auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
}

Bool ActiveCoolingControl::setFanDirection(const UInt32 fanDirection)
{
	if (supportsActiveCoolingControls())
	{
		DptfRequest request(DptfRequestType::ActiveControlSetFanDirection, m_participantIndex, m_domainIndex);
		request.setDataFromUInt32(fanDirection);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		return result.isSuccessful();
	}
	else
	{
		throw dptf_exception("Domain does not support the active control interface.");
	}
} 

ActiveControlSet ActiveCoolingControl::getActiveControlSet()
{
	DptfRequest request(DptfRequestType::ActiveControlGetControlSet, m_participantIndex, m_domainIndex);
	auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();
	return ActiveControlSet::createFromFps(result.getData());
}

void ActiveCoolingControl::setValueWithinCapabilities()
{
	auto status = getStatus();
	auto currentSpeed = Percentage::fromWholeNumber(status.getCurrentControlId());
	currentSpeed = snapToCapabilitiesBounds(currentSpeed);
	setControl(currentSpeed);
}

Percentage ActiveCoolingControl::snapToCapabilitiesBounds(Percentage fanSpeed)
{
	if (fanSpeed != Percentage::createInvalid())
	{
		Percentage minFanSpeed = getDynamicCapabilities().getMinFanSpeed();
		Percentage maxFanSpeed = getDynamicCapabilities().getMaxFanSpeed();

		if (fanSpeed < minFanSpeed)
		{
			fanSpeed = minFanSpeed;
		}
		else if (fanSpeed > maxFanSpeed)
		{
			fanSpeed = maxFanSpeed;
		}
	}
	return fanSpeed;
}
