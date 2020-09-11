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

#include "DomainTemperatureBase.h"

DomainTemperatureBase::DomainTemperatureBase(
	UIntN participantIndex,
	UIntN domainIndex,
	Bool areTemperatureThresholdsSupported,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_areTemperatureThresholdsSupported(areTemperatureThresholdsSupported)
	, m_arbitratorTemperatureThresholds(participantServicesInterface)
{
	bindRequestHandlers();
}

DomainTemperatureBase::~DomainTemperatureBase()
{
	clearCachedData();
}

void DomainTemperatureBase::bindRequestHandlers()
{
	bindRequestHandler(DptfRequestType::ClearCachedData, [=](const PolicyRequest& policyRequest) {
		return this->handleClearCachedResults(policyRequest);
	});
	bindRequestHandler(DptfRequestType::ClearPolicyRequestsForAllControls, [=](const PolicyRequest& policyRequest) {
		return this->handleRemovePolicyRequests(policyRequest);
	});
	bindRequestHandler(
		DptfRequestType::TemperatureControlGetTemperatureStatus,
		[=](const PolicyRequest& policyRequest) { return this->handleGetTemperatureStatus(policyRequest); });
	bindRequestHandler(
		DptfRequestType::TemperatureControlGetTemperatureThresholds,
		[=](const PolicyRequest& policyRequest) { return this->handleGetTemperatureThresholds(policyRequest); });
	bindRequestHandler(
		DptfRequestType::TemperatureControlSetTemperatureThresholds,
		[=](const PolicyRequest& policyRequest) { return this->handleSetTemperatureThresholds(policyRequest); });
	bindRequestHandler(
		DptfRequestType::TemperatureControlGetPowerShareTemperatureThreshold, [=](const PolicyRequest& policyRequest) {
			return this->handleGetPowerShareTemperatureThreshold(policyRequest);
		});
	bindRequestHandler(DptfRequestType::TemperatureControlGetCalibrationTable, [=](const PolicyRequest& policyRequest) {
		return this->handleGetCalibrationTable(policyRequest);
	});
	bindRequestHandler(DptfRequestType::TemperatureControlGetPollingTable, [=](const PolicyRequest& policyRequest) {
		return this->handleGetPollingTable(policyRequest);
	});
	bindRequestHandler(
		DptfRequestType::TemperatureControlIsVirtualTemperatureControl,
		[=](const PolicyRequest& policyRequest) { return this->handleIsVirtualTemperatureControl(policyRequest); });
	bindRequestHandler(
		DptfRequestType::TemperatureControlSetVirtualTemperature,
		[=](const PolicyRequest& policyRequest) { return this->handleSetVirtualTemperature(policyRequest); });
}

DptfRequestResult DomainTemperatureBase::handleClearCachedResults(const PolicyRequest& policyRequest)
{
	clearAllCachedResults();
	auto& request = policyRequest.getRequest();
	DptfRequestResult result(true, "Successfully cleared all cached results.", request);

	return result;
}

DptfRequestResult DomainTemperatureBase::handleRemovePolicyRequests(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		auto policyIndex = policyRequest.getPolicyIndex();
		auto currentTemperature = getTemperatureStatus().getCurrentTemperature();

		auto lastArbitratedTemperatureThresholds =
			m_arbitratorTemperatureThresholds.getArbitratedTemperatureThresholds();
		m_arbitratorTemperatureThresholds.removeTemperatureThresholdsRequest(policyIndex, currentTemperature);

		if (m_arbitratorTemperatureThresholds.arbitratedTemperatureThresholdsChangedSinceLastSet())
		{
			auto newArbitratedTemperatureThresholds =
				m_arbitratorTemperatureThresholds.getArbitratedTemperatureThresholds();
			setTemperatureThresholds(newArbitratedTemperatureThresholds, lastArbitratedTemperatureThresholds);
			sendActivityLoggingDataIfEnabled(getParticipantIndex(), getDomainIndex());
		}

		return DptfRequestResult(true, "Successfully removed policy request from temperature control.", request);
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(
			false, "Failed to remove policy request from temperature control: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to remove policy request from temperature control.", request);
	}
}

DptfRequestResult DomainTemperatureBase::handleGetTemperatureStatus(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		if (requestResultIsCached(request))
		{
			return getCachedResult(request);
		}
		else
		{
			auto temperatureStatus = getTemperatureStatus();
			DptfRequestResult result(true, "Successfully retrieved temperature status.", request);
			result.setData(temperatureStatus.toDptfBuffer());
			updateCachedResult(result);

			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, "Failed to retrieve temperature status: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to retrieve temperature status.", request);
	}
}

DptfRequestResult DomainTemperatureBase::handleGetTemperatureThresholds(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		if (requestResultIsCached(request))
		{
			return getCachedResult(request);
		}
		else
		{
			auto temperatureThresholds = getTemperatureThresholds();
			DptfRequestResult result(true, "Successfully retrieved temperature thresholds.", request);
			result.setData(temperatureThresholds.toDptfBuffer());
			updateCachedResult(result);

			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, "Failed to retrieve temperature thresholds: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to retrieve temperature thresholds.", request);
	}
}

DptfRequestResult DomainTemperatureBase::handleSetTemperatureThresholds(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		auto policyIndex = policyRequest.getPolicyIndex();
		auto temperatureThresholds = TemperatureThresholds::createFromDptfBuffer(request.getData());

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
		// TODO: wanted to use MessageCategory::TemperatureThresholds as a function parameter
		PARTICIPANT_LOG_MESSAGE_DEBUG({
			std::string message;
			message += "Policy called handler SetTemperatureThresholds.\n";
			message += "Aux0 = " + temperatureThresholds.getAux0().toString() + "\n";
			message += "Aux1 = " + temperatureThresholds.getAux1().toString() + "\n";
			message += "Participant Index = " + std::to_string(request.getParticipantIndex()) + "\n";
			message += "Domain Index = " + std::to_string(request.getDomainIndex()) + "\n";
			message += "Policy Index = " + std::to_string(policyIndex) + "\n";
			return message;
		});
#endif
		auto currentTemperature = getCurrentTemperatureStatusForPolicy(policyIndex).getCurrentTemperature();
		auto currentHysteresis = getCurrentTemperatureThresholdsForPolicy(policyIndex).getHysteresis();

		auto currentArbitratedTemperatureThresholds =
			m_arbitratorTemperatureThresholds.getArbitratedTemperatureThresholds();
		auto newArbitratedTemperatureThresholds =
			m_arbitratorTemperatureThresholds.arbitrateTemperatureThresholdsRequest(
				policyIndex, temperatureThresholds, currentTemperature, currentHysteresis);

		// always set the thresholds even if no change
		setTemperatureThresholds(newArbitratedTemperatureThresholds, currentArbitratedTemperatureThresholds);

		m_arbitratorTemperatureThresholds.commitTemperatureThresholdsRequest(
			policyIndex, temperatureThresholds, currentTemperature, currentHysteresis);

		DptfRequest requestGetTemperatureThresholds(
			DptfRequestType::TemperatureControlGetTemperatureThresholds, getParticipantIndex(), getDomainIndex());
		if (requestResultIsCached(requestGetTemperatureThresholds))
		{
			clearCachedResult(requestGetTemperatureThresholds);
		}

		sendActivityLoggingDataIfEnabled(getParticipantIndex(), getDomainIndex());

		return DptfRequestResult(true, "Successfully set temperature thresholds.", request);
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, "Failed to set temperature thresholds: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to set temperature thresholds.", request);
	}
}

DptfRequestResult DomainTemperatureBase::handleGetPowerShareTemperatureThreshold(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		if (requestResultIsCached(request))
		{
			return getCachedResult(request);
		}
		else
		{
			auto temperatureThreshold = getPowerShareTemperatureThreshold();
			DptfRequestResult result(true, "Successfully retrieved power share temperature threshold.", request);
			result.setData(temperatureThreshold.toDptfBuffer());
			updateCachedResult(result);

			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(
			false, "Failed to retrieve power share temperature threshold: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to retrieve power share temperature threshold.", request);
	}
}

DptfRequestResult DomainTemperatureBase::handleGetCalibrationTable(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		if (requestResultIsCached(request))
		{
			return getCachedResult(request);
		}
		else
		{
			auto calibrationTable = getCalibrationTable();
			DptfRequestResult result(true, "Successfully retrieved calibration table.", request);
			result.setData(calibrationTable);
			updateCachedResult(result);

			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, "Failed to retrieve calibration table: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to retrieve calibration table.", request);
	}
}

DptfRequestResult DomainTemperatureBase::handleGetPollingTable(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		if (requestResultIsCached(request))
		{
			return getCachedResult(request);
		}
		else
		{
			auto pollingTable = getPollingTable();
			DptfRequestResult result(true, "Successfully retrieved polling table.", request);
			result.setData(pollingTable);
			updateCachedResult(result);

			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, "Failed to retrieve polling table: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to retrieve polling table.", request);
	}
}

DptfRequestResult DomainTemperatureBase::handleIsVirtualTemperatureControl(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		if (requestResultIsCached(request))
		{
			return getCachedResult(request);
		}
		else
		{
			auto isVirtualTemperatureControl = isVirtualTemperature();
			DptfRequestResult result(true, "Successfully retrieved if it is virtual temperature control.", request);
			result.setDataFromBool(isVirtualTemperatureControl);
			updateCachedResult(result);

			return result;
		}
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(
			false, "Failed to retrieve if it is virtual temperature control: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to retrieve if it is virtual temperature control.", request);
	}
}

DptfRequestResult DomainTemperatureBase::handleSetVirtualTemperature(const PolicyRequest& policyRequest)
{
	auto& request = policyRequest.getRequest();

	try
	{
		auto virtualTemperature = Temperature::createFromDptfBuffer(request.getData());
		setVirtualTemperature(virtualTemperature);

		return DptfRequestResult(true, "Successfully set virtual temperature.", request);
	}
	catch (dptf_exception& ex)
	{
		return DptfRequestResult(false, "Failed to set virtual temperature: " + ex.getDescription(), request);
	}
	catch (...)
	{
		return DptfRequestResult(false, "Failed to set virtual temperature.", request);
	}
}

TemperatureThresholds DomainTemperatureBase::getTemperatureThresholds()
{
	if (m_areTemperatureThresholdsSupported)
	{
		UIntN domainIndex = getDomainIndex();
		Temperature aux0 = getAuxTemperatureThreshold(domainIndex, 0);
		Temperature aux1 = getAuxTemperatureThreshold(domainIndex, 1);
		Temperature hysteresis = getHysteresis(domainIndex);

		return TemperatureThresholds(aux0, aux1, hysteresis);
	}
	else
	{
		return TemperatureThresholds(
			Temperature::createInvalid(), Temperature::createInvalid(), Temperature::createInvalid());
	}
}

void DomainTemperatureBase::setTemperatureThresholds(
	const TemperatureThresholds& temperatureThresholds,
	const TemperatureThresholds& lastSetTemperatureThresholds)
{
	if (!m_areTemperatureThresholdsSupported)
	{
		throw dptf_exception("Temperature thresholds are not supported.");
	}

	UIntN domainIndex = getDomainIndex();
	Bool isAux0Successful = false;
	Bool isAux1Successful = false;
	auto aux0 = temperatureThresholds.getAux0();
	auto aux1 = temperatureThresholds.getAux1();
	auto lastSetAux0 = lastSetTemperatureThresholds.getAux0();
	auto lastSetAux1 = lastSetTemperatureThresholds.getAux1();

	if ((!lastSetAux0.isValid() && !lastSetAux1.isValid()) || (aux0 < lastSetAux0) || (aux1 < lastSetAux1))
	{
		isAux0Successful = setAux0(aux0, domainIndex);
		isAux1Successful = setAux1(aux1, domainIndex);
	}
	else
	{
		isAux1Successful = setAux1(aux1, domainIndex);
		isAux0Successful = setAux0(aux0, domainIndex);
	}

	if (!isAux0Successful || !isAux1Successful)
	{
		throw dptf_exception("Could not set one or both temperature thresholds.");
	}
}

void DomainTemperatureBase::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
	try
	{
		if (isActivityLoggingEnabled() == true)
		{
			TemperatureThresholds tempthreshold = getTemperatureThresholds();

			EsifCapabilityData capability;
			capability.type = ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD;
			capability.size = sizeof(capability);
			capability.data.temperatureThresholdControl.aux0 = tempthreshold.getAux0();
			capability.data.temperatureThresholdControl.aux1 = tempthreshold.getAux1();
			capability.data.temperatureThresholdControl.hysteresis = tempthreshold.getHysteresis();

			getParticipantServices()->sendDptfEvent(
				ParticipantEvent::DptfParticipantControlAction,
				domainIndex,
				Capability::getEsifDataFromCapabilityData(&capability));

			PARTICIPANT_LOG_MESSAGE_INFO({
				std::stringstream message;
				message << "Published activity for participant " << getParticipantIndex() << ", "
						<< "domain " << getName() << " "
						<< "("
						<< "Temperature Status"
						<< ")";
				return message.str();
			});
		}
	}
	catch (...)
	{
		// skip if there are any issues in sending log data
	}
}

std::shared_ptr<XmlNode> DomainTemperatureBase::getArbitratorXml(UIntN policyIndex) const
{
	return m_arbitratorTemperatureThresholds.getArbitrationXmlForPolicy(policyIndex);
}

void DomainTemperatureBase::onClearCachedData(void)
{
	// do nothing.
}

Temperature DomainTemperatureBase::getAuxTemperatureThreshold(UIntN domainIndex, UInt8 auxNumber)
{
	try
	{
		auto aux = getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
			esif_primitive_type::GET_TEMPERATURE_THRESHOLDS, domainIndex, auxNumber);

		return Temperature::snapWithinAllowableTripPointRange(aux);
	}
	catch (...)
	{
		return Temperature::fromCelsius(0);
	}
}

Temperature DomainTemperatureBase::getHysteresis(UIntN domainIndex) const
{
	try
	{
		return getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
			esif_primitive_type::GET_TEMPERATURE_THRESHOLD_HYSTERESIS, domainIndex);
	}
	catch (...)
	{
		return Temperature::fromCelsius(0);
	}
}

Bool DomainTemperatureBase::setAux0(Temperature& aux0, UIntN domainIndex)
{
	Bool isSuccessful = false;
	try
	{
		getParticipantServices()->primitiveExecuteSetAsTemperatureTenthK(
			esif_primitive_type::SET_TEMPERATURE_THRESHOLDS, aux0, domainIndex, 0);
		isSuccessful = true;
	}
	catch (dptf_exception& ex)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING_EX({ return ex.getDescription(); });
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING(
			{ return "Something went wrong when setting temperature thresholds for AUX0"; });
	}

	return isSuccessful;
}

Bool DomainTemperatureBase::setAux1(Temperature& aux1, UIntN domainIndex)
{
	Bool isSuccessful = false;
	try
	{
		getParticipantServices()->primitiveExecuteSetAsTemperatureTenthK(
			esif_primitive_type::SET_TEMPERATURE_THRESHOLDS, aux1, domainIndex, 1);
		isSuccessful = true;
	}
	catch (dptf_exception& ex)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING_EX({ return ex.getDescription(); });
	}
	catch (...)
	{
		PARTICIPANT_LOG_MESSAGE_WARNING(
			{ return "Something went wrong when setting temperature thresholds for AUX1"; });
	}

	return isSuccessful;
}

TemperatureStatus DomainTemperatureBase::getCurrentTemperatureStatusForPolicy(UIntN policyIndex)
{
	DptfRequest request(
		DptfRequestType::TemperatureControlGetTemperatureStatus, getParticipantIndex(), getDomainIndex());
	DptfRequestResult result = handleGetTemperatureStatus(PolicyRequest(policyIndex, request));
	result.throwIfFailure();

	return TemperatureStatus::createFromDptfBuffer(result.getData());
}

TemperatureThresholds DomainTemperatureBase::getCurrentTemperatureThresholdsForPolicy(UIntN policyIndex)
{
	DptfRequest request(
		DptfRequestType::TemperatureControlGetTemperatureThresholds, getParticipantIndex(), getDomainIndex());
	DptfRequestResult result = handleGetTemperatureThresholds(PolicyRequest(policyIndex, request));
	result.throwIfFailure();

	return TemperatureThresholds::createFromDptfBuffer(result.getData());
}
