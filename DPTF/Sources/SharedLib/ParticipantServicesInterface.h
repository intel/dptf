/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include "MessageLoggingInterface.h"
#include "EsifPrimitiveInterface.h"
#include "ParticipantEventRegistrationInterface.h"
#include "FrameworkEventCreationInterface.h"
#include "ParticipantMessage.h"
#include "DptfEventInterface.h"
#include "UserPreferredCacheInterface.h"
#include "RequestHandlerInterface.h"
#include "DomainType.h"

class ParticipantServicesInterface : public MessageLoggingInterface,
									 public EsifPrimitiveInterface,
									 public ParticipantEventRegistrationInterface,
									 public FrameworkEventCreationInterface,
									 public DptfEventInterface,
									 public UserPreferredCacheInterface
{
public:
	virtual void registerRequestHandler(DptfRequestType::Enum requestType, RequestHandlerInterface* handler) = 0;
	virtual void unregisterRequestHandler(DptfRequestType::Enum requestType, RequestHandlerInterface* handler) = 0;
	virtual DomainType::Type getDomainType(UIntN domainIndex) = 0;
};
