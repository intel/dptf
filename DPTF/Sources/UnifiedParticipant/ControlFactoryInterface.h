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
#include "ParticipantServicesInterface.h"
#include "ControlBase.h"

class ControlFactoryInterface
{
public:
	virtual ~ControlFactoryInterface(){};

	virtual ControlBase* make(
		UIntN participantIndex,
		UIntN domainIndex,
		UIntN version,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface) = 0;

	virtual ControlBase* make(
		UIntN participantIndex,
		UIntN domainIndex,
		UIntN version,
		UIntN associatedVersion,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	{
		throw not_implemented();
	};
};
