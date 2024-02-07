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

#include "DomainDynamicEppBase.h"
#include "CachedValue.h"

class DomainDynamicEpp_001 : public DomainDynamicEppBase
{
public:
	DomainDynamicEpp_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainDynamicEpp_001();

	// DomainDynamicEppInterface
	virtual UInt32 getEppSensitivityHint() override;
	virtual SocWorkloadSource::Source getEppSensitivityHintSource() override;
	virtual void updateEppSensitivityHint(UInt32 eppSensitivityHint) override;
	virtual void setDynamicEppSupport(UInt32 dynamicEppSupport) override;

	// ComponentExtendedInterface
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	UInt32 m_eppHint;
	void checkDynamicEppSupport();

	// hide the copy constructor and = operator
	DomainDynamicEpp_001(const DomainDynamicEpp_001& rhs);
	DomainDynamicEpp_001& operator=(const DomainDynamicEpp_001& rhs);
};
