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

#include "RelationshipTableEntryBase.h"

RelationshipTableEntryBase::RelationshipTableEntryBase(
    const std::string& sourceDeviceAcpiScope,
    const std::string& targetDeviceAcpiScope)
    : m_sourceDeviceIndex(Constants::Invalid),
    m_targetDeviceIndex(Constants::Invalid),
    m_sourceDeviceAcpiScope(sourceDeviceAcpiScope),
    m_targetDeviceAcpiScope(targetDeviceAcpiScope)
{
}

RelationshipTableEntryBase::~RelationshipTableEntryBase()
{
}

const std::string& RelationshipTableEntryBase::sourceDeviceAcpiScope() const
{
    return m_sourceDeviceAcpiScope;
}

UIntN RelationshipTableEntryBase::sourceDeviceIndex() const
{
    return m_sourceDeviceIndex;
}

const std::string& RelationshipTableEntryBase::targetDeviceAcpiScope() const
{
    return m_targetDeviceAcpiScope;
}

UIntN RelationshipTableEntryBase::targetDeviceIndex() const
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