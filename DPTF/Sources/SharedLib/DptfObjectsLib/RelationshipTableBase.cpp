/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

RelationshipTableBase::RelationshipTableBase(const std::vector<std::shared_ptr<RelationshipTableEntryBase>>& entries)
	: m_entries(entries)
{
}

RelationshipTableBase::~RelationshipTableBase()
{
}

void RelationshipTableBase::associateParticipant(std::string participantScope, UIntN participantIndex)
{
	auto tableRows = findTableRowsWithParticipantScope(participantScope);
	for (auto tableRow = tableRows.begin(); tableRow != tableRows.end(); ++tableRow)
	{
		m_entries.at(*tableRow)->associateParticipant(participantScope, participantIndex);
	}
}

void RelationshipTableBase::disassociateParticipant(UIntN participantIndex)
{
	auto tableRows = findTableRowsWithParticipantIndex(participantIndex);
	for (auto tableRow = tableRows.begin(); tableRow != tableRows.end(); ++tableRow)
	{
		m_entries.at(*tableRow)->disassociateParticipant(participantIndex);
	}
}

void RelationshipTableBase::associateDomain(
	std::string participantScope,
	DomainType::Type domainType,
	UIntN domainIndex)
{
	auto tableRows = findTableRowsWithParticipantScope(participantScope);
	for (auto tableRow = tableRows.begin(); tableRow != tableRows.end(); ++tableRow)
	{
		m_entries.at(*tableRow)->associateDomain(participantScope, domainType, domainIndex);
	}
}

void RelationshipTableBase::associateDomain(UIntN participantIndex, DomainType::Type domainType, UIntN domainIndex)
{
	auto tableRows = findTableRowsWithParticipantIndex(participantIndex);
	for (auto tableRow = tableRows.begin(); tableRow != tableRows.end(); ++tableRow)
	{
		m_entries.at(*tableRow)->associateDomain(participantIndex, domainType, domainIndex);
	}
}

void RelationshipTableBase::disassociateDomain(UIntN participantIndex, UIntN domainIndex)
{
	auto tableRows = findTableRowsWithParticipantIndex(participantIndex);
	for (auto tableRow = tableRows.begin(); tableRow != tableRows.end(); ++tableRow)
	{
		m_entries.at(*tableRow)->disassociateDomain(participantIndex, domainIndex);
	}
}

Bool RelationshipTableBase::isParticipantSourceDevice(UIntN participantIndex) const
{
	auto tableRows = findTableRowsWithParticipantIndex(participantIndex);
	for (auto tableRow = tableRows.begin(); tableRow != tableRows.end(); ++tableRow)
	{
		if (m_entries.at(*tableRow)->getSourceDeviceIndex() == participantIndex)
		{
			return true;
		}
	}
	return false;
}

Bool RelationshipTableBase::isParticipantTargetDevice(UIntN participantIndex) const
{
	auto tableRows = findTableRowsWithParticipantIndex(participantIndex);
	for (auto tableRow = tableRows.begin(); tableRow != tableRows.end(); ++tableRow)
	{
		if (m_entries.at(*tableRow)->getTargetDeviceIndex() == participantIndex)
		{
			return true;
		}
	}
	return false;
}

UIntN RelationshipTableBase::getNumberOfEntries(void) const
{
	return (UIntN)m_entries.size();
}

std::vector<UIntN> RelationshipTableBase::findTableRowsWithParticipantScope(std::string participantScope) const
{
	std::vector<UIntN> rows;
	for (UIntN row = 0; row < getNumberOfEntries(); ++row)
	{
		auto entry = m_entries.at(row);
		if ((entry->getSourceDeviceScope() == participantScope) || (entry->getTargetDeviceScope() == participantScope))
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
		auto entry = m_entries.at(row);
		if ((entry->getSourceDeviceIndex() == participantIndex) || (entry->getTargetDeviceIndex() == participantIndex))
		{
			rows.push_back(row);
		}
	}
	return rows;
}

std::set<UIntN> RelationshipTableBase::getAllTargetIndexes() const
{
	std::set<UIntN> targetIndexes;
	for (auto entry = m_entries.begin(); entry < m_entries.end(); ++entry)
	{
		if ((*entry)->targetDeviceIndexValid())
		{
			targetIndexes.insert((*entry)->getTargetDeviceIndex());
		}
	}
	return targetIndexes;
}

std::set<UIntN> RelationshipTableBase::getAllSourceIndexes() const
{
	std::set<UIntN> sourceIndexes;
	for (auto entry = m_entries.begin(); entry < m_entries.end(); ++entry)
	{
		if ((*entry)->sourceDeviceIndexValid())
		{
			sourceIndexes.insert((*entry)->getSourceDeviceIndex());
		}
	}
	return sourceIndexes;
}
