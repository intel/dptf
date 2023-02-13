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
#include "RelationshipTableEntryInterface.h"

class dptf_export RelationshipTableEntryBase : public RelationshipTableEntryInterface
{
public:
	RelationshipTableEntryBase(
		const std::string& sourceDeviceScope,
		DomainType::Type sourceDomainType,
		const std::string& targetDeviceScope,
		DomainType::Type targetDomainType);
	RelationshipTableEntryBase(const std::string& participantScope, DomainType::Type domainType);
	RelationshipTableEntryBase(const std::string& sourceDeviceScope, const std::string& targetDeviceScope);
	RelationshipTableEntryBase(const std::string& participantScope);
	RelationshipTableEntryBase();
	virtual ~RelationshipTableEntryBase();

	virtual const std::string& getSourceDeviceScope() const override;
	virtual const std::string& getSourceDeviceName() const override;
	virtual UIntN getSourceDeviceIndex() const override;
	virtual Bool sourceDeviceIndexValid(void) const override;
	virtual Bool sourceDomainIndexValid(void) const override;
	virtual UIntN getSourceDomainIndex() const override;
	virtual DomainType::Type getSourceDomainType() const override;
	virtual const std::string& getTargetDeviceScope() const override;
	virtual const std::string& getTargetDeviceName() const override;
	virtual UIntN getTargetDeviceIndex() const override;
	virtual Bool targetDeviceIndexValid(void) const override;
	virtual Bool targetDomainIndexValid(void) const override;
	virtual UIntN getTargetDomainIndex() const override;
	virtual DomainType::Type getTargetDomainType() const override;
	virtual void associateParticipant(std::string participantScope, UIntN participantIndex, std::string participantName)
		override;
	virtual void disassociateParticipant(UIntN participantIndex) override;
	virtual void associateDomain(std::string participantScope, DomainType::Type domainType, UIntN domainIndex) override;
	virtual void associateDomain(UIntN participantIndex, DomainType::Type domainType, UIntN domainIndex) override;
	virtual void disassociateDomain(UIntN participantIndex, UIntN domainIndex) override;
	virtual Bool operator==(const RelationshipTableEntryInterface& baseEntry) const override;

private:
	std::string m_sourceDeviceScope;
	UIntN m_sourceDeviceIndex;
	DomainType::Type m_sourceDomainType;
	UIntN m_sourceDomainIndex;
	std::string m_targetDeviceScope;
	UIntN m_targetDeviceIndex;
	DomainType::Type m_targetDomainType;
	UIntN m_targetDomainIndex;
	std::string m_sourceDeviceName;
	std::string m_targetDeviceName;
};
