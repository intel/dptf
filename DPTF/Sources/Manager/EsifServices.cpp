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

#include "EsifServices.h"
#include "ParticipantManagerInterface.h"
#include "IndexContainer.h"
#include "EsifData.h"
#include "EsifDataPercentage.h"
#include "EsifDataFrequency.h"
#include "EsifDataPower.h"
#include "EsifDataString.h"
#include "EsifDataTemperature.h"
#include "EsifDataUInt8.h"
#include "EsifDataUInt32.h"
#include "EsifDataUInt64.h"
#include "EsifDataVoid.h"
#include "EsifDataGuid.h"
#include "esif_ccb_rc.h"
#include "ManagerMessage.h"
#include "ManagerLogger.h"
#include "EsifDataTime.h"

using namespace std;

EsifServices::EsifServices(
	const DptfManagerInterface* dptfManager,
	const esif_handle_t esifHandle,
	EsifAppServicesInterface* appServices,
	eLogType currentLogVerbosityLevel)
	: m_dptfManager(dptfManager)
	, m_esifHandle(esifHandle)
	, m_appServices(appServices)
	, m_currentLogVerbosityLevel(currentLogVerbosityLevel)
{
}

eLogType EsifServices::getCurrentLogVerbosityLevel(void) const
{
	return m_currentLogVerbosityLevel;
}

void EsifServices::setCurrentLogVerbosityLevel(eLogType currentLogVerbosityLevel)
{
	m_currentLogVerbosityLevel = currentLogVerbosityLevel;
}

EsifServicesInterface* EsifServices::getEsifServices() const
{
	return (EsifServicesInterface*)this;
}

UInt32 EsifServices::readConfigurationUInt32(const string& elementPath)
{
	EsifDataUInt32 esifResult;

	eEsifError rc = m_appServices->getConfigurationValue(
		m_esifHandle,
		(const esif_handle_t)(UInt64)(DptfManagerInterface*)m_dptfManager,
		EsifDataString("dptf"),
		EsifDataString(elementPath),
		esifResult);

	if (rc != ESIF_OK)
	{
		ManagerMessage message =
			ManagerMessage(m_dptfManager, FLF, "Error returned from ESIF services interface function call");
		message.addMessage("Element Path", elementPath);
		message.setEsifErrorCode(rc);

		MANAGER_LOG_MESSAGE_WARNING({ return message; });

		throw dptf_exception(message);
	}

	return esifResult;
}

UInt32 EsifServices::readConfigurationUInt32(const string& nameSpace, const string& elementPath)
{
	EsifDataUInt32 esifResult;

	eEsifError rc = m_appServices->getConfigurationValue(
		m_esifHandle,
		(const esif_handle_t)(UInt64)(DptfManagerInterface*)m_dptfManager,
		EsifDataString(nameSpace),
		EsifDataString(elementPath),
		esifResult);

	throwIfNotSuccessful(FLF, rc, string("Failed to read configuration UInt32 for ") + elementPath + string("."));

	return esifResult;
}

void EsifServices::writeConfigurationUInt32(const string& elementPath, UInt32 elementValue, UInt32 flags)
{
	eEsifError rc = m_appServices->setConfigurationValue(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		EsifDataString("dptf"),
		EsifDataString(elementPath),
		EsifDataUInt32(elementValue),
		flags);

	if (rc != ESIF_OK)
	{
		ManagerMessage message =
			ManagerMessage(m_dptfManager, FLF, "Error returned from ESIF services interface function call");
		message.addMessage("Element Path", elementPath);
		message.addMessage("Element Value", elementValue);
		message.setEsifErrorCode(rc);

		MANAGER_LOG_MESSAGE_WARNING({ return message; });

		throw dptf_exception(message);
	}
}

string EsifServices::readConfigurationString(const string& nameSpace, const string& elementPath)
{
	DptfBuffer buffer(Constants::DefaultBufferSize);
	EsifDataContainer esifData(esif_data_type::ESIF_DATA_STRING, buffer.get(), buffer.size(), 0);
	eEsifError rc = m_appServices->getConfigurationValue(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		EsifDataString(nameSpace),
		EsifDataString(elementPath),
		esifData);
	if (rc == ESIF_E_NEED_LARGER_BUFFER)
	{
		buffer.allocate(esifData.getDataLength());
		EsifDataContainer esifDataTryAgain(esif_data_type::ESIF_DATA_STRING, buffer.get(), buffer.size(), 0);
		rc = m_appServices->getConfigurationValue(
			m_esifHandle,
			(const esif_handle_t)(UInt64)m_dptfManager,
			EsifDataString(nameSpace),
			EsifDataString(elementPath),
			esifDataTryAgain);
	}
	throwIfNotSuccessful(FLF, rc, string("Failed to read configuration string for ") + elementPath + string("."));

	buffer.trim(esifData.getDataLength());
	return buffer.toString();
}

DptfBuffer EsifServices::readConfigurationBinary(const string& nameSpace, const string& elementPath)
{
	DptfBuffer buffer(Constants::DefaultBufferSize);
	EsifDataContainer esifData(esif_data_type::ESIF_DATA_BINARY, buffer.get(), buffer.size(), 0);
	eEsifError rc = m_appServices->getConfigurationValue(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		EsifDataString(nameSpace),
		EsifDataString(elementPath),
		esifData);
	if (rc == ESIF_E_NEED_LARGER_BUFFER)
	{
		buffer.allocate(esifData.getDataLength());
		EsifDataContainer esifDataTryAgain(esif_data_type::ESIF_DATA_BINARY, buffer.get(), buffer.size(), 0);
		rc = m_appServices->getConfigurationValue(
			m_esifHandle,
			(const esif_handle_t)(UInt64)m_dptfManager,
			EsifDataString(nameSpace),
			EsifDataString(elementPath),
			esifDataTryAgain);
	}
	throwIfNotSuccessful(FLF, rc, string("Failed to read configuration binary for ") + elementPath + string("."));

	buffer.trim(esifData.getDataLength());
	return buffer;
}

void EsifServices::writeConfigurationBinary(
	void* bufferPtr,
	UInt32 bufferLength,
	UInt32 dataLength,
	const string& nameSpace,
	const string& key,
	UInt32 flags)
{
	eEsifError rc = m_appServices->setConfigurationValue(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		EsifDataString(nameSpace),
		EsifDataString(key),
		EsifDataContainer(ESIF_DATA_BINARY, bufferPtr, bufferLength, dataLength),
		flags);

	if (rc != ESIF_OK)
	{
		ManagerMessage message =
			ManagerMessage(m_dptfManager, FLF, "Error returned from ESIF services interface function call");
		message.addMessage("Element Path", key);
		message.setEsifErrorCode(rc);

		MANAGER_LOG_MESSAGE_WARNING({ return message; });

		throw dptf_exception(message);
	}
}

void EsifServices::deleteConfigurationBinary(const string& nameSpace, const string& key)
{
	eEsifError rc = m_appServices->setConfigurationValue(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		EsifDataString(nameSpace),
		EsifDataString(key),
		EsifDataContainer(ESIF_DATA_BINARY, NULL, 0, 0),
		ESIF_SERVICE_CONFIG_DELETE);

	if (rc != ESIF_OK)
	{
		ManagerMessage message =
			ManagerMessage(m_dptfManager, FLF, "Error returned from ESIF services interface function call");
		message.addMessage("Element Path", key);
		message.setEsifErrorCode(rc);

		MANAGER_LOG_MESSAGE_WARNING({ return message; });

		throw dptf_exception(message);
	}
}

UInt8 EsifServices::primitiveExecuteGetAsUInt8(
	esif_primitive_type primitive,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	EsifDataUInt8 esifResult;

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataVoid(),
		esifResult,
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

	return esifResult;
}

void EsifServices::primitiveExecuteSetAsUInt8(
	esif_primitive_type primitive,
	UInt8 elementValue,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataUInt8(elementValue),
		EsifDataVoid(),
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

UInt32 EsifServices::primitiveExecuteGetAsUInt32(
	esif_primitive_type primitive,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	EsifDataUInt32 esifResult;

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataVoid(),
		esifResult,
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

	return esifResult;
}

void EsifServices::primitiveExecuteSetAsUInt32(
	esif_primitive_type primitive,
	UInt32 elementValue,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataUInt32(elementValue),
		EsifDataVoid(),
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

UInt64 EsifServices::primitiveExecuteGetAsUInt64(
	esif_primitive_type primitive,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	EsifDataUInt64 esifResult;

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataVoid(),
		esifResult,
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

	return esifResult;
}

void EsifServices::primitiveExecuteSetAsUInt64(
	esif_primitive_type primitive,
	UInt64 elementValue,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataUInt64(elementValue),
		EsifDataVoid(),
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

Temperature EsifServices::primitiveExecuteGetAsTemperatureTenthK(
	esif_primitive_type primitive,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	EsifDataTemperature esifResult;

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataVoid(),
		esifResult,
		primitive,
		instance);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
	// Added to help debug issue with missing temperature threshold events
	if (primitive == esif_primitive_type::GET_TEMPERATURE)
	{
		MANAGER_LOG_MESSAGE_DEBUG({
			ManagerMessage message = ManagerMessage(
				m_dptfManager,
				_file,
				_line,
				_function,
				"Requested participant temperature from ESIF." + MessageCategory::TemperatureThresholds);
			message.addMessage("Temperature", esifResult);
			message.setEsifPrimitive(primitive, instance);
			message.setParticipantAndDomainIndex(participantIndex, domainIndex);
			message.setEsifErrorCode(rc);
			return message;
		});
	}
#endif

	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

	return esifResult;
}

void EsifServices::primitiveExecuteSetAsTemperatureTenthK(
	esif_primitive_type primitive,
	Temperature temperature,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
	// Added to help debug issue with missing temperature threshold events
	if (primitive == esif_primitive_type::SET_TEMPERATURE_THRESHOLDS)
	{
		MANAGER_LOG_MESSAGE_DEBUG({
			ManagerMessage message = ManagerMessage(
				m_dptfManager,
				_file,
				_line,
				_function,
				"Setting new temperature threshold for participant." + MessageCategory::TemperatureThresholds);
			message.addMessage("Temperature", temperature.toString());
			message.setEsifPrimitive(primitive, instance);
			message.setParticipantAndDomainIndex(participantIndex, domainIndex);
			return message;
		});
	}
#endif

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataTemperature(temperature),
		EsifDataVoid(),
		primitive,
		instance);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
	// Added to help debug issue with missing temperature threshold events
	if (primitive == esif_primitive_type::SET_TEMPERATURE_THRESHOLDS && rc != ESIF_OK)
	{
		MANAGER_LOG_MESSAGE_ERROR({
			ManagerMessage message = ManagerMessage(
				m_dptfManager,
				_file,
				_line,
				_function,
				"Failed to set new temperature threshold." + MessageCategory::TemperatureThresholds);
			message.addMessage("Temperature", temperature.toString());
			message.setEsifPrimitive(primitive, instance);
			message.setParticipantAndDomainIndex(participantIndex, domainIndex);
			message.setEsifErrorCode(rc);
			return message;
		});
	}
#endif

	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

Percentage EsifServices::primitiveExecuteGetAsPercentage(
	esif_primitive_type primitive,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	EsifDataPercentage esifResult;

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataVoid(),
		esifResult,
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

	return esifResult;
}

void EsifServices::primitiveExecuteSetAsPercentage(
	esif_primitive_type primitive,
	Percentage percentage,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataPercentage(percentage),
		EsifDataVoid(),
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

Frequency EsifServices::primitiveExecuteGetAsFrequency(
	esif_primitive_type primitive,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	EsifDataFrequency esifResult;

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataVoid(),
		esifResult,
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

	return esifResult;
}

void EsifServices::primitiveExecuteSetAsFrequency(
	esif_primitive_type primitive,
	Frequency frequency,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataFrequency(frequency),
		EsifDataVoid(),
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

Power EsifServices::primitiveExecuteGetAsPower(
	esif_primitive_type primitive,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	EsifDataPower esifResult;

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataVoid(),
		esifResult,
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

	return esifResult;
}

void EsifServices::primitiveExecuteSetAsPower(
	esif_primitive_type primitive,
	Power power,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataPower(power),
		EsifDataVoid(),
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

TimeSpan EsifServices::primitiveExecuteGetAsTimeInMilliseconds(
	esif_primitive_type primitive,
	UIntN participantIndex /*= Constants::Esif::NoParticipant*/,
	UIntN domainIndex /*= Constants::Esif::NoDomain*/,
	UInt8 instance /*= Constants::Esif::NoInstance*/)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	EsifDataTime esifResult;

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataVoid(),
		esifResult,
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

	return esifResult.createTimeSpanFromMilliseconds();
}

void EsifServices::primitiveExecuteSetAsTimeInMilliseconds(
	esif_primitive_type primitive,
	TimeSpan time,
	UIntN participantIndex /*= Constants::Esif::NoParticipant*/,
	UIntN domainIndex /*= Constants::Esif::NoDomain*/,
	UInt8 instance /*= Constants::Esif::NoInstance*/)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataTime(time.asMillisecondsInt()),
		EsifDataVoid(),
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

string EsifServices::primitiveExecuteGetAsString(
	esif_primitive_type primitive,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	EsifDataString esifResult(Constants::DefaultBufferSize);

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		(const esif_handle_t)(UInt64)m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataVoid(),
		esifResult,
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

	return esifResult;
}

void EsifServices::primitiveExecuteSetAsString(
	esif_primitive_type primitive,
	string stringValue,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataString(stringValue),
		EsifDataVoid(),
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

DptfBuffer EsifServices::primitiveExecuteGet(
	esif_primitive_type primitive,
	esif_data_type esifDataType,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	DptfBuffer buffer(Constants::DefaultBufferSize);
	EsifDataContainer esifData(esifDataType, buffer.get(), buffer.size(), 0);
	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		(const esif_handle_t)(UInt64)m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataVoid(),
		esifData,
		primitive,
		instance);
	if (rc == ESIF_E_NEED_LARGER_BUFFER)
	{
		buffer.allocate(esifData.getDataLength());
		EsifDataContainer esifDataTryAgain(esifDataType, buffer.get(), buffer.size(), 0);
		rc = m_appServices->executePrimitive(
			m_esifHandle,
			(const esif_handle_t)(UInt64)m_dptfManager,
			m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
			(const esif_handle_t)(UInt64)m_dptfManager->getIndexContainer()->getDomainHandle(
				participantIndex, domainIndex),
			EsifDataVoid(),
			esifDataTryAgain,
			primitive,
			instance);
	}
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

	buffer.trim(esifData.getDataLength());
	return buffer;
}

DptfBuffer EsifServices::primitiveExecuteGetWithArgument(
	esif_primitive_type primitive,
	DptfBuffer esifData,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	DptfBuffer esifResult = DptfBuffer(esifData.size());
	UInt32 size = esifData.size();
	esifResult = esifData;

	if (esifData.get())
	{
		EsifDataContainer esifRequest(esif_data_type::ESIF_DATA_STRUCTURE, esifData.get(), size, size);
		EsifDataContainer esifResponse(
			esif_data_type::ESIF_DATA_STRUCTURE, esifResult.get(), size, size);

		eEsifError rc = m_appServices->executePrimitive(
			m_esifHandle,
			(const esif_handle_t)(UInt64)m_dptfManager,
			m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
			m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
			esifRequest,
			esifResponse,
			primitive,
			instance);

		esifResult.trim(esifResponse.getDataLength());

		throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
	}

	return esifResult;
}

void EsifServices::primitiveExecuteSet(
	esif_primitive_type primitive,
	esif_data_type esifDataType,
	void* bufferPtr,
	UInt32 bufferLength,
	UInt32 dataLength,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	eEsifError rc = m_appServices->executePrimitive(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		(const esif_handle_t)(UInt64)m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		EsifDataContainer(esifDataType, bufferPtr, bufferLength, dataLength),
		EsifDataVoid(),
		primitive,
		instance);
	throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

void EsifServices::writeMessageFatal(const string& message, MessageCategory::Type messageCategory)
{
	if (eLogType::eLogTypeFatal <= m_currentLogVerbosityLevel)
	{
		writeMessage(eLogType::eLogTypeFatal, messageCategory, message);
	}
}

void EsifServices::writeMessageError(const string& message, MessageCategory::Type messageCategory)
{
	if (eLogType::eLogTypeError <= m_currentLogVerbosityLevel)
	{
		writeMessage(eLogType::eLogTypeError, messageCategory, message);
	}
}

void EsifServices::writeMessageWarning(const string& message, MessageCategory::Type messageCategory)
{
	if (eLogType::eLogTypeWarning <= m_currentLogVerbosityLevel)
	{
		writeMessage(eLogType::eLogTypeWarning, messageCategory, message);
	}
}

void EsifServices::writeMessageInfo(const string& message, MessageCategory::Type messageCategory)
{
	if (eLogType::eLogTypeInfo <= m_currentLogVerbosityLevel)
	{
		writeMessage(eLogType::eLogTypeInfo, messageCategory, message);
	}
}

void EsifServices::writeMessageDebug(const string& message, MessageCategory::Type messageCategory)
{
	if (eLogType::eLogTypeDebug <= m_currentLogVerbosityLevel)
	{
		writeMessage(eLogType::eLogTypeDebug, messageCategory, message);
	}
}

eLogType EsifServices::getLoggingLevel(void)
{
	return m_currentLogVerbosityLevel;
}

void EsifServices::registerEvent(FrameworkEvent::Type frameworkEvent, UIntN participantIndex, UIntN domainIndex)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	auto frameworkEventInfo = FrameworkEventInfo::instance();
	Guid guid = frameworkEventInfo->getGuid(frameworkEvent);

	if (!frameworkEventInfo->usesDummyGuid(frameworkEvent))
	{
		eEsifError rc = m_appServices->registerForEvent(
			m_esifHandle,
			(const esif_handle_t)(UInt64)m_dptfManager,
			m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
			(const esif_handle_t)(UInt64)m_dptfManager->getIndexContainer()->getDomainHandle(
				participantIndex, domainIndex),
			EsifDataGuid(guid));

		// FIXME:  this should throw an exception if we get an unexpected return code.  For now we will just log an
		//         error so we can see a list of items to correct.
		if (rc != ESIF_OK)
		{
			MANAGER_LOG_MESSAGE_WARNING({
				ManagerMessage message = ManagerMessage(
					m_dptfManager, _file, _line, _function, "Error returned from ESIF register event function call");
				message.setFrameworkEvent(frameworkEvent);
				message.addMessage("Guid", guid.toString());
				message.setParticipantAndDomainIndex(participantIndex, domainIndex);
				message.setEsifErrorCode(rc);
				return message;
			});
		}
	}
}

void EsifServices::unregisterEvent(FrameworkEvent::Type frameworkEvent, UIntN participantIndex, UIntN domainIndex)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	auto frameworkEventInfo = FrameworkEventInfo::instance();
	Guid guid = frameworkEventInfo->getGuid(frameworkEvent);

	if (!frameworkEventInfo->usesDummyGuid(frameworkEvent))
	{

		eEsifError rc = m_appServices->unregisterForEvent(
			m_esifHandle,
			(const esif_handle_t)(UInt64)m_dptfManager,
			m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
			(const esif_handle_t)(UInt64)m_dptfManager->getIndexContainer()->getDomainHandle(
				participantIndex, domainIndex),
			EsifDataGuid(guid));

		// FIXME:  this should throw an exception if we get an unexpected return code.  For now we will just log an
		//         error so we can see a list of items to correct.
		if (rc != ESIF_OK)
		{
			MANAGER_LOG_MESSAGE_WARNING({
				ManagerMessage message = ManagerMessage(
					m_dptfManager, _file, _line, _function, "Error returned from ESIF unregister event function call");
				message.setFrameworkEvent(frameworkEvent);
				message.addMessage("Guid", guid.toString());
				message.setParticipantAndDomainIndex(participantIndex, domainIndex);
				message.setEsifErrorCode(rc);
				return message;
			});
		}
	}
}

void EsifServices::writeMessage(eLogType messageLevel, MessageCategory::Type messageCategory, const string& message)
{
	// Do not throw an error here....
	// In general we will write to the log file when an error has been thrown and we don't want to create
	// a recursive mess.  Anyway, if the message isn't written, what can we do about?  We can't write another
	// error message.  So, if this fails, we ignore it.

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
	if ((messageLevel == eLogType::eLogTypeFatal) || (messageLevel == eLogType::eLogTypeError)
		|| (messageCategory == MessageCategory::TemperatureThresholds))
	{
#endif

		m_appServices->writeLog(
			m_esifHandle,
			(const esif_handle_t)(UInt64)m_dptfManager,
			0,
			ESIF_INVALID_HANDLE,
			EsifDataString(message),
			messageLevel);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
	}

#endif
}

void EsifServices::throwIfNotSuccessful(
	const string& fileName,
	UIntN lineNumber,
	const string& executingFunctionName,
	eEsifError returnCode,
	UIntN participantIndex,
	UIntN domainIndex)
{
	if (returnCode == ESIF_OK)
	{
		return;
	}

	ManagerMessage message = ManagerMessage(
		m_dptfManager,
		fileName,
		lineNumber,
		executingFunctionName,
		"Error returned from ESIF services interface function call");
	message.setParticipantAndDomainIndex(participantIndex, domainIndex);
	message.setEsifErrorCode(returnCode);

	MANAGER_LOG_MESSAGE_WARNING({ return message; });

	throw dptf_exception(message);
}

void EsifServices::throwIfNotSuccessful(
	const string& fileName,
	UIntN lineNumber,
	const string& executingFunctionName,
	eEsifError returnCode,
	esif_primitive_type primitive,
	UIntN participantIndex,
	UIntN domainIndex,
	UInt8 instance)
{
	if (returnCode == ESIF_OK)
	{
		return;
	}

	ManagerMessage message = ManagerMessage(
		m_dptfManager,
		fileName,
		lineNumber,
		executingFunctionName,
		"Error returned from ESIF services interface function call");
	message.setEsifPrimitive(primitive, instance);
	message.setParticipantAndDomainIndex(participantIndex, domainIndex);
	message.setEsifErrorCode(returnCode);

	if ((primitive == GET_TRIP_POINT_ACTIVE) && (returnCode == ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT))
	{
		// no message.  we still throw an exception to inform the policy.
	}
	else
	{
		MANAGER_LOG_MESSAGE_WARNING({ return message; });
	}

	switch (returnCode)
	{
	case ESIF_I_AGAIN:
		throw primitive_try_again(message);

	case ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP:
		throw primitive_not_found_in_dsp(message);

	case ESIF_E_PRIMITIVE_DST_UNAVAIL:
		throw primitive_destination_unavailable(message);

	case ESIF_E_IO_OPEN_FAILED:
		throw file_open_create_failure(message);

	case ESIF_E_ACPI_OBJECT_NOT_FOUND:
		throw acpi_object_not_found(message);

	case ESIF_E_UNSUPPORTED_RESULT_TEMP_TYPE:
		throw unsupported_result_temp_type();

	default:
		throw primitive_execution_failed(message);
	}
}

void EsifServices::throwIfNotSuccessful(
	const string& fileName,
	UIntN lineNumber,
	const string& executingFunctionName,
	eEsifError returnCode,
	const string& messageText)
{
	if (returnCode == ESIF_OK)
	{
		return;
	}

	ManagerMessage message = ManagerMessage(
		m_dptfManager,
		fileName,
		lineNumber,
		executingFunctionName,
		"Error returned from ESIF services interface function call");
	message.setEsifErrorCode(returnCode);
	message.addMessage(messageText);

	MANAGER_LOG_MESSAGE_WARNING({ return message; });

	throw primitive_execution_failed(message);
}

void EsifServices::throwIfParticipantDomainCombinationInvalid(
	const string& fileName,
	UIntN lineNumber,
	const string& executingFunctionName,
	UIntN participantIndex,
	UIntN domainIndex)
{
	if ((participantIndex == Constants::Esif::NoParticipant) && (domainIndex != Constants::Esif::NoDomain))
	{
		ManagerMessage message = ManagerMessage(
			m_dptfManager,
			fileName,
			lineNumber,
			executingFunctionName,
			"Domain index not valid without associated participant index");
		message.setParticipantAndDomainIndex(participantIndex, domainIndex);

		MANAGER_LOG_MESSAGE_WARNING({ return message; });

		throw dptf_exception(message);
	}
}

void EsifServices::sendDptfEvent(
	FrameworkEvent::Type frameworkEvent,
	UIntN participantIndex,
	UIntN domainIndex,
	EsifData eventData)
{
	throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

	Guid guid = FrameworkEventInfo::instance()->getGuid(frameworkEvent);

	eEsifError rc = m_appServices->sendEvent(
		m_esifHandle,
		(const esif_handle_t)(UInt64)m_dptfManager,
		m_dptfManager->getIndexContainer()->getParticipantHandle(participantIndex),
		m_dptfManager->getIndexContainer()->getDomainHandle(participantIndex, domainIndex),
		&eventData,
		EsifDataGuid(guid));

	if (rc != ESIF_OK)
	{
		MANAGER_LOG_MESSAGE_WARNING({
			ManagerMessage message = ManagerMessage(
				m_dptfManager, _file, _line, _function, "Error returned from ESIF send event function call");
			message.setFrameworkEvent(frameworkEvent);
			message.addMessage("Guid", guid.toString());
			message.setParticipantAndDomainIndex(participantIndex, domainIndex);
			message.setEsifErrorCode(rc);
			return message;
		});
	}
}

eEsifError EsifServices::sendCommand(UInt32 argc, const string& argv)
{
	DptfBuffer buffer(Constants::DefaultBufferSize);
	EsifDataContainer response(esif_data_type::ESIF_DATA_STRING, buffer.get(), buffer.size(), 0);
	return m_appServices->sendCommand(
		m_esifHandle, (const esif_handle_t)(UInt64)m_dptfManager, argc, EsifDataString(argv), response);
}
