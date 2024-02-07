/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "DomainType.h"

class dptf_export RelationshipTableEntryInterface
{
public:
	virtual ~RelationshipTableEntryInterface(){};

	virtual const std::string& getSourceDeviceScope() const = 0;
	virtual const std::string& getSourceDeviceName() const = 0;
	virtual UIntN getSourceDeviceIndex() const = 0;
	virtual Bool sourceDeviceIndexValid(void) const = 0;
	virtual Bool sourceDomainIndexValid(void) const = 0;
	virtual UIntN getSourceDomainIndex() const = 0;
	virtual DomainType::Type getSourceDomainType() const = 0;
	virtual const std::string& getTargetDeviceScope() const = 0;
	virtual const std::string& getTargetDeviceName() const = 0;
	virtual UIntN getTargetDeviceIndex() const = 0;
	virtual Bool targetDeviceIndexValid(void) const = 0;
	virtual Bool targetDomainIndexValid(void) const = 0;
	virtual UIntN getTargetDomainIndex() const = 0;
	virtual DomainType::Type getTargetDomainType() const = 0;
	virtual void associateParticipant(
		std::string participantScope,
		UIntN participantIndex,
		std::string participantName) = 0;
	virtual void disassociateParticipant(UIntN participantIndex) = 0;
	virtual void associateDomain(std::string participantScope, DomainType::Type domainType, UIntN domainIndex) = 0;
	virtual void associateDomain(UIntN participantIndex, DomainType::Type domainType, UIntN domainIndex) = 0;
	virtual void disassociateDomain(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual Bool operator==(const RelationshipTableEntryInterface& baseEntry) const = 0;
};
