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
#include "DptfManagerInterface.h"

class ParticipantManagerInterface;
class Participant;
class WorkItemQueueManagerInterface;
class EsifServicesInterface;

// When a participant is created it receives this interface as a parameter.  These are the functions provided
// by the DPTF framework.  In many cases we pass through directly to ESIF.

class ParticipantServices final : public ParticipantServicesInterface
{
public:
	ParticipantServices(DptfManagerInterface* dptfManager, UIntN participantIndex);

	virtual UInt8 primitiveExecuteGetAsUInt8(
		esif_primitive_type primitive,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual void primitiveExecuteSetAsUInt8(
		esif_primitive_type primitive,
		UInt8 value,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual UInt32 primitiveExecuteGetAsUInt32(
		esif_primitive_type primitive,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual void primitiveExecuteSetAsUInt32(
		esif_primitive_type primitive,
		UInt32 value,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual UInt64 primitiveExecuteGetAsUInt64(
		esif_primitive_type primitive,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual void primitiveExecuteSetAsUInt64(
		esif_primitive_type primitive,
		UInt64 value,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual Temperature primitiveExecuteGetAsTemperatureTenthK(
		esif_primitive_type primitive,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual void primitiveExecuteSetAsTemperatureTenthK(
		esif_primitive_type primitive,
		Temperature temperature,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual Percentage primitiveExecuteGetAsPercentage(
		esif_primitive_type primitive,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual void primitiveExecuteSetAsPercentage(
		esif_primitive_type primitive,
		Percentage percentage,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual Frequency primitiveExecuteGetAsFrequency(
		esif_primitive_type primitive,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual void primitiveExecuteSetAsFrequency(
		esif_primitive_type primitive,
		Frequency frequency,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual Power primitiveExecuteGetAsPower(
		esif_primitive_type primitive,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual void primitiveExecuteSetAsPower(
		esif_primitive_type primitive,
		Power power,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual TimeSpan primitiveExecuteGetAsTimeInMilliseconds(
		esif_primitive_type primitive,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual void primitiveExecuteSetAsTimeInMilliseconds(
		esif_primitive_type primitive,
		TimeSpan time,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override;

	virtual std::string primitiveExecuteGetAsString(
		esif_primitive_type primitive,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual void primitiveExecuteSetAsString(
		esif_primitive_type primitive,
		std::string stringValue,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual DptfBuffer primitiveExecuteGet(
		esif_primitive_type primitive,
		esif_data_type esifDataType,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual void primitiveExecuteSet(
		esif_primitive_type primitive,
		esif_data_type esifDataType,
		void* bufferPtr,
		UInt32 bufferLength,
		UInt32 dataLength,
		UIntN domainIndex = Constants::Esif::NoDomain,
		UInt8 instance = Constants::Esif::NoInstance) override final;

	virtual void writeMessageFatal(const DptfMessage& message) override final;
	virtual void writeMessageError(const DptfMessage& message) override final;
	virtual void writeMessageWarning(const DptfMessage& message) override final;
	virtual void writeMessageInfo(const DptfMessage& message) override final;
	virtual void writeMessageDebug(const DptfMessage& message) override final;
	virtual eLogType getLoggingLevel(void) override final;

	virtual void registerEvent(ParticipantEvent::Type participantEvent) override final;
	virtual void unregisterEvent(ParticipantEvent::Type participantEvent) override final;

	virtual void createEventDomainPerformanceControlCapabilityChanged() override final;
	virtual void createEventDomainPowerControlCapabilityChanged() override final;
	virtual void createEventPowerLimitChanged() override final;
	virtual void createEventPerformanceCapabilitiesChanged() override final;

	virtual void sendDptfEvent(ParticipantEvent::Type participantEvent, UIntN domainId, esif_data eventData)
		override final;

	// Display Cache
	virtual UIntN getUserPreferredDisplayCacheValue(UIntN participantIndex, UIntN domainIndex) override final;
	virtual void setUserPreferredDisplayCacheValue(UIntN participantIndex, UIntN domainIndex, UIntN userPreferredIndex)
		override final;
	virtual void invalidateUserPreferredDisplayCache(UIntN participantIndex, UIntN domainIndex) override final;
	virtual Bool isUserPreferredDisplayCacheValid(UIntN participantIndex, UIntN domainIndex) override final;

	virtual void registerRequestHandler(DptfRequestType::Enum requestType, RequestHandlerInterface* handler) override;
	virtual void unregisterRequestHandler(DptfRequestType::Enum requestType, RequestHandlerInterface* handler) override;
	
	virtual DomainType::Type getDomainType(UIntN domainIndex) override final;

private:
	// hide the copy constructor and assignment operator.
	ParticipantServices(const ParticipantServices& rhs);
	ParticipantServices& operator=(const ParticipantServices& rhs);

	DptfManagerInterface* m_dptfManager;
	ParticipantManagerInterface* m_participantManager;
	Participant* m_participant;
	WorkItemQueueManagerInterface* m_workItemQueueManager;
	EsifServicesInterface* m_esifServices;
	UIntN m_participantIndex;

	void throwIfNotWorkItemThread(void);
	EsifServicesInterface* getEsifServices();
};
