/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#include "ParticipantServices.h"
#include "DptfManager.h"
#include "ParticipantManager.h"
#include "WorkItemQueueManager.h"
#include "WIDomainPerformanceControlCapabilityChanged.h"
#include "WIDomainPowerControlCapabilityChanged.h"
#include "EsifServices.h"
#include "ManagerMessage.h"

ParticipantServices::ParticipantServices(DptfManager* dptfManager, UIntN participantIndex) :
    m_dptfManager(dptfManager),
    m_participantManager(dptfManager->getParticipantManager()),
    m_participant(dptfManager->getParticipantManager()->getParticipantPtr(participantIndex)),
    m_workItemQueueManager(dptfManager->getWorkItemQueueManager()),
    m_esifServices(dptfManager->getEsifServices()),
    m_participantIndex(participantIndex)
{
}

UInt8 ParticipantServices::primitiveExecuteGetAsUInt8(esif_primitive_type primitive,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGetAsUInt8(primitive, m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteSetAsUInt8(esif_primitive_type primitive,
    UInt8 value, UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteSetAsUInt8(primitive, value, m_participantIndex, domainIndex, instance);
}

UInt32 ParticipantServices::primitiveExecuteGetAsUInt32(esif_primitive_type primitive,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGetAsUInt32(primitive, m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteSetAsUInt32(esif_primitive_type primitive,
    UInt32 value, UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteSetAsUInt32(primitive, value, m_participantIndex, domainIndex, instance);
}

UInt64 ParticipantServices::primitiveExecuteGetAsUInt64(esif_primitive_type primitive,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGetAsUInt64(primitive, m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteSetAsUInt64(esif_primitive_type primitive,
    UInt64 value, UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteSetAsUInt64(primitive, value, m_participantIndex, domainIndex, instance);
}

Temperature ParticipantServices::primitiveExecuteGetAsTemperatureC(esif_primitive_type primitive,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGetAsTemperatureC(primitive, m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteSetAsTemperatureC(esif_primitive_type primitive,
    Temperature temperature, UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteSetAsTemperatureC(primitive, temperature,
        m_participantIndex, domainIndex, instance);
}

Percentage ParticipantServices::primitiveExecuteGetAsPercentage(esif_primitive_type primitive,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGetAsPercentage(primitive, m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteSetAsPercentage(esif_primitive_type primitive, Percentage percentage,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteSetAsPercentage(primitive, percentage, m_participantIndex, domainIndex, instance);
}

Frequency ParticipantServices::primitiveExecuteGetAsFrequency(esif_primitive_type primitive,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGetAsFrequency(primitive, m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteSetAsFrequency(esif_primitive_type primitive, Frequency frequency,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteSetAsFrequency(primitive, frequency, m_participantIndex, domainIndex, instance);
}

Power ParticipantServices::primitiveExecuteGetAsPower(esif_primitive_type primitive,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGetAsPower(primitive, m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteSetAsPower(esif_primitive_type primitive, Power power, UIntN domainIndex,
    UInt8 instance)
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteSetAsPower(primitive, power,
        m_participantIndex, domainIndex, instance);
}

std::string ParticipantServices::primitiveExecuteGetAsString(esif_primitive_type primitive,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGetAsString(primitive, m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteGet(esif_primitive_type primitive, esif_data_type esifDataType,
    void* bufferPtr, UInt32 bufferLength, UInt32* dataLength, UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteGet(primitive, esifDataType, bufferPtr, bufferLength, dataLength,
        m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteSet(esif_primitive_type primitive, esif_data_type esifDataType,
    void* bufferPtr, UInt32 bufferLength, UInt32 dataLength, UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteSet(primitive, esifDataType, bufferPtr, bufferLength, dataLength,
        m_participantIndex, domainIndex, instance);
}

void ParticipantServices::writeMessageFatal(const DptfMessage& message)
{
    throwIfNotWorkItemThread();

    ManagerMessage updatedMessage = ManagerMessage(m_dptfManager, message);
    updatedMessage.setParticipantIndex(m_participantIndex);

    m_esifServices->writeMessageFatal(updatedMessage);
}

void ParticipantServices::writeMessageError(const DptfMessage& message)
{
    throwIfNotWorkItemThread();

    ManagerMessage updatedMessage = ManagerMessage(m_dptfManager, message);
    updatedMessage.setParticipantIndex(m_participantIndex);

    m_esifServices->writeMessageError(updatedMessage);
}

void ParticipantServices::writeMessageWarning(const DptfMessage& message)
{
    throwIfNotWorkItemThread();

    ManagerMessage updatedMessage = ManagerMessage(m_dptfManager, message);
    updatedMessage.setParticipantIndex(m_participantIndex);

    m_esifServices->writeMessageWarning(updatedMessage);
}

void ParticipantServices::writeMessageInfo(const DptfMessage& message)
{
    throwIfNotWorkItemThread();

    ManagerMessage updatedMessage = ManagerMessage(m_dptfManager, message);
    updatedMessage.setParticipantIndex(m_participantIndex);

    m_esifServices->writeMessageInfo(updatedMessage);
}

void ParticipantServices::writeMessageDebug(const DptfMessage& message)
{
    throwIfNotWorkItemThread();

    ManagerMessage updatedMessage = ManagerMessage(m_dptfManager, message);
    updatedMessage.setParticipantIndex(m_participantIndex);

    m_esifServices->writeMessageDebug(updatedMessage);
}

void ParticipantServices::registerEvent(ParticipantEvent::Type participantEvent)
{
    throwIfNotWorkItemThread();
    m_participant->registerEvent(participantEvent);
}

void ParticipantServices::unregisterEvent(ParticipantEvent::Type participantEvent)
{
    throwIfNotWorkItemThread();
    m_participant->unregisterEvent(participantEvent);
}

void ParticipantServices::createEventDomainPerformanceControlCapabilityChanged()
{
    WorkItem* wi = new WIDomainPerformanceControlCapabilityChanged(m_dptfManager, m_participantIndex, Constants::Invalid);
    m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(wi);
}

void ParticipantServices::createEventDomainPowerControlCapabilityChanged()
{
    WorkItem* wi = new WIDomainPowerControlCapabilityChanged(m_dptfManager, m_participantIndex, Constants::Invalid);
    m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(wi);
}

void ParticipantServices::throwIfNotWorkItemThread(void)
{
    if (m_workItemQueueManager->isWorkItemThread() == false)
    {
        throw dptf_exception("Participant Services functionality called from an unknown thread.");
    }
}