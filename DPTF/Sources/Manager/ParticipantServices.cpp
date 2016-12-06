/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "ParticipantManagerInterface.h"
#include "WorkItemQueueManagerInterface.h"
#include "WIDomainPerformanceControlCapabilityChanged.h"
#include "WIDomainPowerControlCapabilityChanged.h"
#include "EsifServicesInterface.h"
#include "ManagerMessage.h"

ParticipantServices::ParticipantServices(DptfManagerInterface* dptfManager, UIntN participantIndex) :
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

Temperature ParticipantServices::primitiveExecuteGetAsTemperatureTenthK(esif_primitive_type primitive,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGetAsTemperatureTenthK(primitive, m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteSetAsTemperatureTenthK(esif_primitive_type primitive,
    Temperature temperature, UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteSetAsTemperatureTenthK(primitive, temperature,
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

TimeSpan ParticipantServices::primitiveExecuteGetAsTimeInMilliseconds(
    esif_primitive_type primitive, 
    UIntN domainIndex /*= Constants::Esif::NoDomain*/, 
    UInt8 instance /*= Constants::Esif::NoInstance*/)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGetAsTimeInMilliseconds(
        primitive, m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteSetAsTimeInMilliseconds(
    esif_primitive_type primitive, 
    TimeSpan time, 
    UIntN domainIndex /*= Constants::Esif::NoDomain*/, 
    UInt8 instance /*= Constants::Esif::NoInstance*/)
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteSetAsTimeInMilliseconds(primitive, time,
        m_participantIndex, domainIndex, instance);
}

std::string ParticipantServices::primitiveExecuteGetAsString(esif_primitive_type primitive,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGetAsString(primitive, m_participantIndex, domainIndex, instance);
}

void ParticipantServices::primitiveExecuteSetAsString(esif_primitive_type primitive, std::string stringValue, 
    UIntN domainIndex, UInt8 instance)  
{
    throwIfNotWorkItemThread();
    m_esifServices->primitiveExecuteSetAsString(primitive, stringValue,
        m_participantIndex, domainIndex, instance);
}

DptfBuffer ParticipantServices::primitiveExecuteGet(esif_primitive_type primitive, esif_data_type esifDataType,
    UIntN domainIndex, UInt8 instance)
{
    throwIfNotWorkItemThread();
    return m_esifServices->primitiveExecuteGet(primitive, esifDataType,
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

void ParticipantServices::sendDptfEvent(ParticipantEvent::Type participantEvent, UIntN domainId, esif_data eventData)
{
    throwIfNotWorkItemThread();
    m_esifServices->sendDptfEvent(ParticipantEvent::ToFrameworkEvent(ParticipantEvent::DptfParticipantControlAction), 
        m_participantIndex, domainId, eventData);
}

// We get/set/invalidate only the display cache value now, 
// to be re-factored if more controls need to be cached later

UIntN ParticipantServices::getUserPreferredDisplayCacheValue(UIntN participantIndex, UIntN domainIndex)
{
    throwIfNotWorkItemThread();

    auto participantScope = m_participant->getParticipantProperties().getAcpiInfo().getAcpiScope();
    auto domainType = m_participant->getDomainPropertiesSet().getDomainProperties(domainIndex).getDomainType();

    return m_dptfManager->getUserPreferredCache()->getUserPreferredDisplayCacheValue(participantScope, domainType);
}

void ParticipantServices::setUserPreferredDisplayCacheValue(UIntN participantIndex, UIntN domainIndex, UIntN userPreferredIndex)
{
    throwIfNotWorkItemThread();

    auto participantScope = m_participant->getParticipantProperties().getAcpiInfo().getAcpiScope();
    auto domainType = m_participant->getDomainPropertiesSet().getDomainProperties(domainIndex).getDomainType();

    m_dptfManager->getUserPreferredCache()->setUserPreferredDisplayCacheValue(participantScope, domainType, userPreferredIndex);
}

void ParticipantServices::invalidateUserPreferredDisplayCache(UIntN participantIndex, UIntN domainIndex)
{
    throwIfNotWorkItemThread();

    auto participantScope = m_participant->getParticipantProperties().getAcpiInfo().getAcpiScope();
    auto domainType = m_participant->getDomainPropertiesSet().getDomainProperties(domainIndex).getDomainType();

    m_dptfManager->getUserPreferredCache()->invalidateUserPreferredDisplayCache(participantScope, domainType);
}

Bool ParticipantServices::isUserPreferredDisplayCacheValid(UIntN participantIndex, UIntN domainIndex)
{
    throwIfNotWorkItemThread();

    auto participantScope = m_participant->getParticipantProperties().getAcpiInfo().getAcpiScope();
    auto domainType = m_participant->getDomainPropertiesSet().getDomainProperties(domainIndex).getDomainType();

    return m_dptfManager->getUserPreferredCache()->isUserPreferredDisplayCacheValid(participantScope, domainType);
}
