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

#pragma once

#include "Dptf.h"
#include "esif_sdk_iface_esif.h"
#include "esif_sdk_primitive_type.h"
#include "DptfManagerInterface.h"
#include "EsifAppServicesInterface.h"
#include "PolicyEvent.h"
#include "ParticipantEvent.h"
#include "MessageCategory.h"
#include "DptfBuffer.h"
#include "TimeSpan.h"

//
// Implements the ESIF services interface which allows the framework to call into ESIF.  See the ESIF HLD for a
// description of the interface.  This is a C++ wrapper that forwards calls through the C interface that is used to
// communicate with ESIF.
//

class dptf_export EsifServicesInterface
{
public:
	virtual ~EsifServicesInterface(void){};

	virtual eLogType getCurrentLogVerbosityLevel(void) const = 0;
	virtual void setCurrentLogVerbosityLevel(eLogType currentLogVerbosityLevel) = 0;

	// Read/write configuration data.

	virtual UInt32 readConfigurationUInt32(const std::string& elementPath) = 0;
	virtual UInt32 readConfigurationUInt32(const std::string& nameSpace, const std::string& elementPath) = 0;
	virtual void writeConfigurationUInt32(const std::string& elementPath, UInt32 elementValue) = 0;
	virtual std::string readConfigurationString(const std::string& elementPath) = 0;
	virtual std::string readConfigurationString(const std::string& nameSpace, const std::string& elementPath) = 0;
	virtual DptfBuffer readConfigurationBinary(const std::string& elementPath) = 0;
	virtual DptfBuffer readConfigurationBinaryFromNameSpace(
		const std::string& nameSpace,
		const std::string& elementPath) = 0;

	// Primitives

	virtual UInt8 primitiveExecuteGetAsUInt8(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual void primitiveExecuteSetAsUInt8(
		esif_primitive_type primitive,
		UInt8 elementValue,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual UInt32 primitiveExecuteGetAsUInt32(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual void primitiveExecuteSetAsUInt32(
		esif_primitive_type primitive,
		UInt32 elementValue,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual UInt64 primitiveExecuteGetAsUInt64(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual void primitiveExecuteSetAsUInt64(
		esif_primitive_type primitive,
		UInt64 elementValue,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual Temperature primitiveExecuteGetAsTemperatureTenthK(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual void primitiveExecuteSetAsTemperatureTenthK(
		esif_primitive_type primitive,
		Temperature temperature,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual Percentage primitiveExecuteGetAsPercentage(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual void primitiveExecuteSetAsPercentage(
		esif_primitive_type primitive,
		Percentage percentage,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual Frequency primitiveExecuteGetAsFrequency(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual void primitiveExecuteSetAsFrequency(
		esif_primitive_type primitive,
		Frequency frequency,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual Power primitiveExecuteGetAsPower(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual void primitiveExecuteSetAsPower(
		esif_primitive_type primitive,
		Power power,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual TimeSpan primitiveExecuteGetAsTimeInMilliseconds(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual void primitiveExecuteSetAsTimeInMilliseconds(
		esif_primitive_type primitive,
		TimeSpan time,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual std::string primitiveExecuteGetAsString(
		esif_primitive_type primitive,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual void primitiveExecuteSetAsString(
		esif_primitive_type primitive,
		std::string stringValue,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual DptfBuffer primitiveExecuteGet(
		esif_primitive_type primitive,
		esif_data_type esifDataType,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	virtual void primitiveExecuteSet(
		esif_primitive_type primitive,
		esif_data_type esifDataType,
		void* bufferPtr,
		UInt32 bufferLength,
		UInt32 dataLength,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) = 0;

	// Message logging

	virtual void writeMessageFatal(
		const std::string& message,
		MessageCategory::Type messageCategory = MessageCategory::Default) = 0;
	virtual void writeMessageError(
		const std::string& message,
		MessageCategory::Type messageCategory = MessageCategory::Default) = 0;
	virtual void writeMessageWarning(
		const std::string& message,
		MessageCategory::Type messageCategory = MessageCategory::Default) = 0;
	virtual void writeMessageInfo(
		const std::string& message,
		MessageCategory::Type messageCategory = MessageCategory::Default) = 0;
	virtual void writeMessageDebug(
		const std::string& message,
		MessageCategory::Type messageCategory = MessageCategory::Default) = 0;
	virtual eLogType getLoggingLevel(void) = 0;

	// Event registration

	virtual void registerEvent(
		FrameworkEvent::Type frameworkEvent,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain) = 0;

	virtual void unregisterEvent(
		FrameworkEvent::Type frameworkEvent,
		UIntN participantIndex = Constants::Esif::NoParticipant,
		UIntN domainIndex = Constants::Esif::NoDomain) = 0;

	virtual void sendDptfEvent(
		FrameworkEvent::Type frameworkEvent,
		UIntN participantIndex,
		UIntN domainIndex,
		EsifData eventData) = 0;

	virtual eEsifError sendCommand(UInt32 argc, const std::string& argv) = 0;
};
