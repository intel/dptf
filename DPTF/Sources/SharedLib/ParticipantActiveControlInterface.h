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
#include "ActiveControlStaticCaps.h"
#include "ActiveControlStatus.h"
#include "ActiveControlSet.h"

class dptf_export ParticipantActiveControlInterface
{
public:
	virtual ActiveControlStaticCaps getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual ActiveControlStatus getActiveControlStatus(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual ActiveControlSet getActiveControlSet(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void setActiveControl(UIntN participantIndex, UIntN domainIndex, UIntN controlId) = 0;
};