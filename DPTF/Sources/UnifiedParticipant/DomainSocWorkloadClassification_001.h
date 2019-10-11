/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "DomainSocWorkloadClassificationBase.h"
#include "CachedValue.h"

class DomainSocWorkloadClassification_001 : public DomainSocWorkloadClassificationBase
{
public:
	DomainSocWorkloadClassification_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainSocWorkloadClassification_001();

	// DomainSocWorkloadClassificationInterface
	virtual UInt32 getSocWorkloadClassification() override;

	// ComponentExtendedInterface
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	void checkSocWorkloadClassificationSupport();

	// hide the copy constructor and = operator
	DomainSocWorkloadClassification_001(const DomainSocWorkloadClassification_001& rhs);
	DomainSocWorkloadClassification_001& operator=(const DomainSocWorkloadClassification_001& rhs);
};
