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

#include "RelationshipTableBase.h"


RelationshipTableBase::RelationshipTableBase()
{
}

RelationshipTableBase::~RelationshipTableBase()
{
}

void RelationshipTableBase::associateParticipant(std::string acpiScope, UIntN participantIndex)
{
    auto tableRows = findTableRowsWithAcpiScope(acpiScope);
    for (UIntN tableRow = 0; tableRow < tableRows.size(); ++tableRow)
    {
        getEntry(tableRows[tableRow])->associateParticipant(acpiScope, participantIndex);
    }
}

void RelationshipTableBase::disassociateParticipant(UIntN participantIndex)
{
    auto tableRows = findTableRowsWithParticipantIndex(participantIndex);
    for (UIntN tableRow = 0; tableRow < tableRows.size(); ++tableRow)
    {
        getEntry(tableRows[tableRow])->disassociateParticipant(participantIndex);
    }
}

Bool RelationshipTableBase::isParticipantSourceDevice(UIntN participantIndex) const
{
    auto tableRows = findTableRowsWithParticipantIndex(participantIndex);
    for (UIntN tableRow = 0; tableRow < tableRows.size(); ++tableRow)
    {
        if (getEntry(tableRows[tableRow])->sourceDeviceIndex() == participantIndex)
        {
            return true;
        }
    }
    return false;
}

Bool RelationshipTableBase::isParticipantTargetDevice(UIntN participantIndex) const
{
    auto tableRows = findTableRowsWithParticipantIndex(participantIndex);
    for (UIntN tableRow = 0; tableRow < tableRows.size(); ++tableRow)
    {
        if (getEntry(tableRows[tableRow])->targetDeviceIndex() == participantIndex)
        {
            return true;
        }
    }
    return false;
}

std::vector<UIntN> RelationshipTableBase::findTableRowsWithAcpiScope(std::string acpiScope) const
{
    std::vector<UIntN> rows;
    for (UIntN row = 0; row < getNumberOfEntries(); ++row)
    {
        if ((getEntry(row)->sourceDeviceAcpiScope() == acpiScope) ||
            (getEntry(row)->targetDeviceAcpiScope() == acpiScope))
        {
            rows.push_back(row);
        }
    }
    return rows;
}

std::vector<UIntN> RelationshipTableBase::findTableRowsWithParticipantIndex(UIntN participantIndex) const
{
    std::vector<UIntN> rows;
    for (UIntN row = 0; row < getNumberOfEntries(); ++row)
    {
        if ((getEntry(row)->sourceDeviceIndex() == participantIndex) ||
            (getEntry(row)->targetDeviceIndex() == participantIndex))
        {
            rows.push_back(row);
        }
    }
    return rows;
}