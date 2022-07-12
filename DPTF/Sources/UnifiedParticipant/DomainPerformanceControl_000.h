/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "DomainPerformanceControlBase.h"

//
// Implements the Null Object pattern.  In the case that the functionality isn't implemented, we use
// this in place so we don't have to check for NULL pointers all throughout the participant implementation.
//
class DomainPerformanceControl_000 : public DomainPerformanceControlBase
{
public:
	DomainPerformanceControl_000(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);

	// DomainPerformanceControlInterface
	virtual PerformanceControlStaticCaps getPerformanceControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
		override;
	virtual PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
		override;
	virtual PerformanceControlStatus getPerformanceControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual PerformanceControlSet getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setPerformanceControl(UIntN participantIndex, UIntN domainIndex, UIntN performanceControlIndex)
		override;
	virtual void setPerformanceControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PerformanceControlDynamicCaps newCapabilities) override;
	virtual void setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData(void) override;
	virtual std::string getName(void) override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;
	virtual void capture(void) override;
	virtual void restore(void) override;

protected:
	virtual UIntN getCurrentPerformanceControlIndex(UIntN ParticipantIndex, UIntN domainIndex) override;
};
