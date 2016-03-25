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

#include "RelationshipTableEntryBase.h"

RelationshipTableEntryBase::RelationshipTableEntryBase(
    const std::string& sourceDeviceAcpiScope,
    const std::string& targetDeviceAcpiScope)
    : m_sourceDeviceAcpiScope(sourceDeviceAcpiScope),
    m_sourceDeviceIndex(Constants::Invalid),
    m_targetDeviceAcpiScope(targetDeviceAcpiScope),
    m_targetDeviceIndex(Constants::Invalid)
{
}

RelationshipTableEntryBase::RelationshipTableEntryBase(
    const std::string& targetDeviceAcpiScope)
    : m_sourceDeviceIndex(Constants::Invalid), 
    m_targetDeviceAcpiScope(targetDeviceAcpiScope),
    m_targetDeviceIndex(Constants::Invalid)
{
}

RelationshipTableEntryBase::~RelationshipTableEntryBase()
{
}

const std::string& RelationshipTableEntryBase::getSourceDeviceAcpiScope() const
{
    return m_sourceDeviceAcpiScope;
}

UIntN RelationshipTableEntryBase::getSourceDeviceIndex() const
{
    return m_sourceDeviceIndex;
}

const std::string& RelationshipTableEntryBase::getTargetDeviceAcpiScope() const
{
    return m_targetDeviceAcpiScope;
}

UIntN RelationshipTableEntryBase::getTargetDeviceIndex() const
{
    return m_targetDeviceIndex;
}

void RelationshipTableEntryBase::associateParticipant(std::string deviceAcpiScope, UIntN deviceIndex)
{
    if (m_sourceDeviceAcpiScope == deviceAcpiScope)
    {
        m_sourceDeviceIndex = deviceIndex;
    }

    if (m_targetDeviceAcpiScope == deviceAcpiScope)
    {
        m_targetDeviceIndex = deviceIndex;
    }
}

void RelationshipTableEntryBase::disassociateParticipant(UIntN deviceIndex)
{
    if (m_sourceDeviceIndex == deviceIndex)
    {
        m_sourceDeviceIndex = Constants::Invalid;
    }

    if (m_targetDeviceIndex == deviceIndex)
    {
        m_targetDeviceIndex = Constants::Invalid;
    }
}

Bool RelationshipTableEntryBase::operator==(const RelationshipTableEntryBase& baseEntry) const
{
    return ((m_sourceDeviceAcpiScope == baseEntry.m_sourceDeviceAcpiScope) && (m_sourceDeviceIndex == baseEntry.m_sourceDeviceIndex)
            && (m_targetDeviceAcpiScope == baseEntry.m_targetDeviceAcpiScope) && (m_targetDeviceIndex == baseEntry.m_targetDeviceIndex));
}

Bool RelationshipTableEntryBase::sourceDeviceIndexValid(void) const
{
    return (m_sourceDeviceIndex != Constants::Invalid);
}

Bool RelationshipTableEntryBase::targetDeviceIndexValid(void) const
{
    return (m_targetDeviceIndex != Constants::Invalid);
}
