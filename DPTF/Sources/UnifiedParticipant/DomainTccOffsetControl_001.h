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

#pragma once

#include "Dptf.h"
#include "DomainTccOffsetControlBase.h"
#include "CachedValue.h"

//
// Implements the Null Object pattern.  In the case that the functionality isn't implemented, we use
// this in place so we don't have to check for NULL pointers all throughout the participant implementation.
//

class DomainTccOffsetControl_001 : public DomainTccOffsetControlBase
{
public:
	DomainTccOffsetControl_001(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainTccOffsetControl_001(void);

	// DomainTccOffsetControlInterface
	virtual Temperature getTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex, const Temperature& tccOffset) override;
	virtual Temperature getMaxTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex) override;
	virtual Temperature getMinTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex) override;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void clearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

private:
	void throwIfInvalidTemperature(const Temperature& temperature);

	// hide the copy constructor and = operator
	DomainTccOffsetControl_001(const DomainTccOffsetControl_001& rhs);
	DomainTccOffsetControl_001& operator=(const DomainTccOffsetControl_001& rhs);

	CachedValue<Temperature> m_tccOffset;
};
