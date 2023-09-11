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

#include "Dptf.h"
#include "RelationshipTableInterface.h"
#include "RelationshipTableEntryBase.h"

class dptf_export RelationshipTableBase : public RelationshipTableInterface
{
public:
	RelationshipTableBase() = default;
	RelationshipTableBase(const std::vector<std::shared_ptr<RelationshipTableEntryBase>>& entries);
	~RelationshipTableBase() override = default;

	RelationshipTableBase(const RelationshipTableBase& other) = default;
	RelationshipTableBase& operator=(const RelationshipTableBase& other) = default;
	RelationshipTableBase(RelationshipTableBase&& other) = default;
	RelationshipTableBase& operator=(RelationshipTableBase&& other) = default;

	void associateParticipant(std::string participantScope, UIntN participantIndex, std::string participantName)
		override;
	void disassociateParticipant(UIntN participantIndex) override;
	void associateDomain(std::string participantScope, DomainType::Type domainType, UIntN domainIndex) override;
	void associateDomain(UIntN participantIndex, DomainType::Type domainType, UIntN domainIndex) override;
	void disassociateDomain(UIntN participantIndex, UIntN domainIndex) override;
	Bool isParticipantSourceDevice(UIntN participantIndex) const override;
	Bool isParticipantTargetDevice(UIntN participantIndex) const override;
	UIntN getNumberOfEntries(void) const override;
	std::set<UIntN> getAllTargetIndexes() const override;
	std::set<UIntN> getAllSourceIndexes() const override;

protected:
	std::vector<UIntN> findTableRowsWithParticipantScope(const std::string& participantScope) const;
	std::vector<UIntN> findTableRowsWithParticipantIndex(UIntN participantIndex) const;

	std::vector<std::shared_ptr<RelationshipTableEntryBase>> m_entries;
};
