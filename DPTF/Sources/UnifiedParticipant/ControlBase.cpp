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

#include "ControlBase.h"
#include "esif_sdk_data_misc.h"

ControlBase::ControlBase(UIntN participantIndex, UIntN domainIndex, std::shared_ptr<ParticipantServicesInterface> participantServices)
    : m_participantIndex(participantIndex), m_domainIndex(domainIndex), m_participantServices(participantServices),
    m_activityLoggingEnabled(false)
{

}

ControlBase::~ControlBase()
{

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

UInt16 ControlBase::createTupleDomain() const
{
    UInt16 tupleDomain;
    tupleDomain = (('0' + (UInt8)m_domainIndex) << 8) + 'D';
    return tupleDomain;
}
