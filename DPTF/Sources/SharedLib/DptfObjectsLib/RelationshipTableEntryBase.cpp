/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
	const std::string& sourceDeviceScope,
	DomainType::Type sourceDomainType,
	const std::string& targetDeviceScope,
	DomainType::Type targetDomainType)
	: m_sourceDeviceScope(sourceDeviceScope)
	, m_sourceDeviceIndex(Constants::Invalid)
	, m_sourceDomainType(sourceDomainType)
	, m_sourceDomainIndex(Constants::Invalid)
	, m_targetDeviceScope(targetDeviceScope)
	, m_targetDeviceIndex(Constants::Invalid)
	, m_targetDomainType(targetDomainType)
	, m_targetDomainIndex(Constants::Invalid)
	, m_sourceDeviceName(Constants::InvalidString)
	, m_targetDeviceName(Constants::InvalidString)
{
}

RelationshipTableEntryBase::RelationshipTableEntryBase(const std::string& participantScope, DomainType::Type domainType)
	: m_sourceDeviceScope(Constants::EmptyString)
	, m_sourceDeviceIndex(Constants::Invalid)
	, m_sourceDomainType(DomainType::Invalid)
	, m_sourceDomainIndex(Constants::Invalid)
	, m_targetDeviceScope(participantScope)
	, m_targetDeviceIndex(Constants::Invalid)
	, m_targetDomainType(domainType)
	, m_targetDomainIndex(Constants::Invalid)
	, m_sourceDeviceName(Constants::InvalidString)
	, m_targetDeviceName(Constants::InvalidString)
{
}

RelationshipTableEntryBase::RelationshipTableEntryBase(
	const std::string& sourceDeviceScope,
	const std::string& targetDeviceScope)
	: RelationshipTableEntryBase(sourceDeviceScope, DomainType::Invalid, targetDeviceScope, DomainType::Invalid)
{
}

RelationshipTableEntryBase::RelationshipTableEntryBase(const std::string& participantScope)
	: RelationshipTableEntryBase(participantScope, DomainType::Invalid)
{
}

RelationshipTableEntryBase::RelationshipTableEntryBase()
	: m_sourceDeviceScope(Constants::InvalidString)
	, m_sourceDeviceIndex(Constants::Invalid)
	, m_sourceDomainType(DomainType::Invalid)
	, m_sourceDomainIndex(Constants::Invalid)
	, m_targetDeviceScope(Constants::InvalidString)
	, m_targetDeviceIndex(Constants::Invalid)
	, m_targetDomainType(DomainType::Invalid)
	, m_targetDomainIndex(Constants::Invalid)
	, m_sourceDeviceName(Constants::InvalidString)
	, m_targetDeviceName(Constants::InvalidString)
{
}

RelationshipTableEntryBase::~RelationshipTableEntryBase()
{
}

const std::string& RelationshipTableEntryBase::getSourceDeviceScope() const
{
	return m_sourceDeviceScope;
}

const std::string& RelationshipTableEntryBase::getSourceDeviceName() const
{
	if (m_sourceDeviceName != Constants::InvalidString)
	{
		return m_sourceDeviceName;
	}

	return m_sourceDeviceScope;
}

UIntN RelationshipTableEntryBase::getSourceDeviceIndex() const
{
	return m_sourceDeviceIndex;
}

const std::string& RelationshipTableEntryBase::getTargetDeviceScope() const
{
	return m_targetDeviceScope;
}

const std::string& RelationshipTableEntryBase::getTargetDeviceName() const
{
	if (m_targetDeviceName != Constants::InvalidString)
	{
		return m_targetDeviceName;
	}

	return m_targetDeviceScope;
}

UIntN RelationshipTableEntryBase::getTargetDeviceIndex() const
{
	return m_targetDeviceIndex;
}

void RelationshipTableEntryBase::associateParticipant(
	std::string participantScope,
	UIntN participantIndex,
	std::string participantName)
{
	if (m_sourceDeviceScope == participantScope)
	{
		m_sourceDeviceIndex = participantIndex;
		m_sourceDeviceName = participantName;
	}

	if (m_targetDeviceScope == participantScope)
	{
		m_targetDeviceIndex = participantIndex;
		m_targetDeviceName = participantName;
	}
}

void RelationshipTableEntryBase::disassociateParticipant(UIntN participantIndex)
{
	if (m_sourceDeviceIndex == participantIndex)
	{
		m_sourceDeviceIndex = Constants::Invalid;
		m_sourceDomainIndex = Constants::Invalid;
		m_sourceDeviceName = Constants::InvalidString;
	}

	if (m_targetDeviceIndex == participantIndex)
	{
		m_targetDeviceIndex = Constants::Invalid;
		m_targetDomainIndex = Constants::Invalid;
		m_targetDeviceName = Constants::InvalidString;
	}
}

void RelationshipTableEntryBase::associateDomain(
	std::string participantScope,
	DomainType::Type domainType,
	UIntN domainIndex)
{
	if ((m_sourceDeviceScope == participantScope) && (m_sourceDomainType == domainType))
	{
		m_sourceDomainIndex = domainIndex;
	}

	if ((m_targetDeviceScope == participantScope) && (m_targetDomainType == domainType))
	{
		m_targetDomainIndex = domainIndex;
	}
}

void RelationshipTableEntryBase::associateDomain(UIntN participantIndex, DomainType::Type domainType, UIntN domainIndex)
{
	if ((m_sourceDeviceIndex == participantIndex) && (m_sourceDomainType == domainType))
	{
		m_sourceDomainIndex = domainIndex;
	}

	if ((m_targetDeviceIndex == participantIndex) && (m_targetDomainType == domainType))
	{
		m_targetDomainIndex = domainIndex;
	}
}

void RelationshipTableEntryBase::disassociateDomain(UIntN participantIndex, UIntN domainIndex)
{
	if ((m_sourceDeviceIndex == participantIndex) && (m_sourceDomainIndex == domainIndex))
	{
		m_sourceDomainIndex = Constants::Invalid;
	}

	if ((m_targetDeviceIndex == participantIndex) && (m_targetDomainIndex == domainIndex))
	{
		m_targetDomainIndex = Constants::Invalid;
	}
}

Bool RelationshipTableEntryBase::operator==(const RelationshipTableEntryInterface& baseEntry) const
{
	return (
		(m_sourceDeviceScope == baseEntry.getSourceDeviceScope())
		&& (m_sourceDeviceIndex == baseEntry.getSourceDeviceIndex())
		&& (m_sourceDomainType == baseEntry.getSourceDomainType())
		&& (m_sourceDomainIndex == baseEntry.getSourceDomainIndex())
		&& (m_targetDeviceScope == baseEntry.getTargetDeviceScope())
		&& (m_targetDeviceIndex == baseEntry.getTargetDeviceIndex())
		&& (m_targetDomainType == baseEntry.getTargetDomainType())
		&& (m_targetDomainIndex == baseEntry.getTargetDomainIndex()));
}

Bool RelationshipTableEntryBase::sourceDeviceIndexValid(void) const
{
	return (m_sourceDeviceIndex != Constants::Invalid);
}

Bool RelationshipTableEntryBase::sourceDomainIndexValid(void) const
{
	return (m_sourceDomainIndex != Constants::Invalid);
}

UIntN RelationshipTableEntryBase::getSourceDomainIndex() const
{
	return m_sourceDomainIndex;
}

DomainType::Type RelationshipTableEntryBase::getSourceDomainType() const
{
	return m_sourceDomainType;
}

Bool RelationshipTableEntryBase::targetDeviceIndexValid(void) const
{
	return (m_targetDeviceIndex != Constants::Invalid);
}

Bool RelationshipTableEntryBase::targetDomainIndexValid(void) const
{
	return (m_targetDomainIndex != Constants::Invalid);
}

UIntN RelationshipTableEntryBase::getTargetDomainIndex() const
{
	return m_targetDomainIndex;
}

DomainType::Type RelationshipTableEntryBase::getTargetDomainType() const
{
	return m_targetDomainType;
}
