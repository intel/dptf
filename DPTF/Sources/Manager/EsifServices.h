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

#include "EsifServicesInterface.h"

class dptf_export EsifServices : public EsifServicesInterface
{
public:
	EsifServices(
		const DptfManagerInterface* dptfManager,
		const esif_handle_t esifHandle,
		EsifAppServicesInterface* appServices,
		eLogType currentLogVerbosityLevel);

	virtual eLogType getCurrentLogVerbosityLevel(void) const override;
	virtual void setCurrentLogVerbosityLevel(eLogType currentLogVerbosityLevel) override;

	// Read/write configuration data.

	virtual UInt32 readConfigurationUInt32(const std::string& elementPath) override;
	virtual UInt32 readConfigurationUInt32(const std::string& nameSpace, const std::string& elementPath) override;
	virtual void writeConfigurationUInt32(const std::string& elementPath, UInt32 elementValue) override;
	virtual std::string readConfigurationString(const std::string& nameSpace, const std::string& elementPath) override;
	virtual DptfBuffer readConfigurationBinary(const std::string& nameSpace, const std::string& elementPath) override;
	virtual void writeConfigurationBinary(
		void* bufferPtr,
		UInt32 bufferLength,
		UInt32 dataLength,
		const std::string& nameSpace,
		const std::string& elementPath) override;
	virtual void deleteConfigurationBinary(
		const std::string& nameSpace,
		const std::string& elementPath) override;

	// Primitives

	virtual UInt8 primitiveExecuteGetAsUInt8(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual void primitiveExecuteSetAsUInt8(
		esif_primitive_type primitive,
		UInt8 elementValue,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual UInt32 primitiveExecuteGetAsUInt32(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual void primitiveExecuteSetAsUInt32(
		esif_primitive_type primitive,
		UInt32 elementValue,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual UInt64 primitiveExecuteGetAsUInt64(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual void primitiveExecuteSetAsUInt64(
		esif_primitive_type primitive,
		UInt64 elementValue,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual Temperature primitiveExecuteGetAsTemperatureTenthK(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual void primitiveExecuteSetAsTemperatureTenthK(
		esif_primitive_type primitive,
		Temperature temperature,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual Percentage primitiveExecuteGetAsPercentage(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual void primitiveExecuteSetAsPercentage(
		esif_primitive_type primitive,
		Percentage percentage,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual Frequency primitiveExecuteGetAsFrequency(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual void primitiveExecuteSetAsFrequency(
		esif_primitive_type primitive,
		Frequency frequency,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual Power primitiveExecuteGetAsPower(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual void primitiveExecuteSetAsPower(
		esif_primitive_type primitive,
		Power power,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual TimeSpan primitiveExecuteGetAsTimeInMilliseconds(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual DptfBuffer primitiveExecuteGetWithArgument(
		esif_primitive_type primitive,
		DptfBuffer esifDataType,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual void primitiveExecuteSetAsTimeInMilliseconds(
		esif_primitive_type primitive,
		TimeSpan time,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual std::string primitiveExecuteGetAsString(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual void primitiveExecuteSetAsString(
		esif_primitive_type primitive,
		std::string stringValue,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual DptfBuffer primitiveExecuteGet(
		esif_primitive_type primitive,
		esif_data_type esifDataType,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual void primitiveExecuteSet(
		esif_primitive_type primitive,
		esif_data_type esifDataType,
		void* bufferPtr,
		UInt32 bufferLength,
		UInt32 dataLength,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	// Message logging

	virtual void writeMessageFatal(
		const std::string& message,
		MessageCategory::Type messageCategory = MessageCategory::Default) override;
	virtual void writeMessageError(
		const std::string& message,
		MessageCategory::Type messageCategory = MessageCategory::Default) override;
	virtual void writeMessageWarning(
		const std::string& message,
		MessageCategory::Type messageCategory = MessageCategory::Default) override;
	virtual void writeMessageInfo(
		const std::string& message,
		MessageCategory::Type messageCategory = MessageCategory::Default) override;
	virtual void writeMessageDebug(
		const std::string& message,
		MessageCategory::Type messageCategory = MessageCategory::Default) override;
	virtual eLogType getLoggingLevel(void) override;

	// Event registration

	virtual void registerEvent(
		FrameworkEvent::Type frameworkEvent,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain) override;

	virtual void unregisterEvent(
		FrameworkEvent::Type frameworkEvent,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain) override;

	virtual void sendDptfEvent(
		FrameworkEvent::Type frameworkEvent,
		UIntN participantIndex,
		UIntN domainIndex,
		EsifData eventData) override;

	virtual eEsifError sendCommand(UInt32 argc, const std::string& argv) override;

private:
	// hide the copy constructor and assignment operator.
	EsifServices(const EsifServices& rhs);
	EsifServices& operator=(const EsifServices& rhs);

	const DptfManagerInterface* m_dptfManager;
	esif_handle_t m_esifHandle;
	EsifAppServicesInterface* m_appServices;
	eLogType m_currentLogVerbosityLevel;

	void writeMessage(eLogType messageLevel, MessageCategory::Type messageCategory, const std::string& message);

	std::string getParticipantName(UIntN participantIndex);
	std::string getDomainName(UIntN participantIndex, UIntN domainIndex);

	void throwIfNotSuccessful(
		const std::string& fileName,
		UIntN lineNumber,
		const std::string& executingFunctionName,
		eEsifError returnCode,
		UIntN participantIndex,
		UIntN domainIndex);
	void throwIfNotSuccessful(
		const std::string& fileName,
		UIntN lineNumber,
		const std::string& executingFunctionName,
		eEsifError returnCode,
		esif_primitive_type primitive,
		UIntN participantIndex,
		UIntN domainIndex,
		UInt8 instance);
	void throwIfNotSuccessful(
		const std::string& fileName,
		UIntN lineNumber,
		const std::string& executingFunctionName,
		eEsifError returnCode,
		const std::string& messageText);
	void throwIfParticipantDomainCombinationInvalid(
		const std::string& fileName,
		UIntN lineNumber,
		const std::string& executingFunctionName,
		UIntN participantIndex,
		UIntN domainIndex);

	EsifServicesInterface* getEsifServices() const;
};
