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
#include "CoreControlStaticCaps.h"
#include "CoreControlDynamicCaps.h"
#include "CoreControlLpoPreference.h"
#include "CoreControlStatus.h"

class DomainCoreControlInterface
{
public:
	virtual ~DomainCoreControlInterface(){};

	virtual CoreControlStaticCaps getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual CoreControlDynamicCaps getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual CoreControlLpoPreference getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual CoreControlStatus getCoreControlStatus(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void setActiveCoreControl(
		UIntN participantIndex,
		UIntN domainIndex,
		const CoreControlStatus& coreControlStatus) = 0;
};
