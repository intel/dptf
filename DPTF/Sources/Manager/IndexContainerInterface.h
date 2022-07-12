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

typedef struct _IndexStruct
{
	UIntN participantIndex;
	esif_handle_t participantHandle;

	UIntN domainIndex;
	esif_handle_t domainHandle;
} IndexStruct, *IndexStructPtr;

class IndexContainerInterface
{
public:
	virtual ~IndexContainerInterface(){};

	virtual void insertHandle(UIntN participantIndex,
		UIntN domainIndex,
		esif_handle_t participantHandle,
		esif_handle_t domainHandle) = 0;

	virtual void removeHandle(esif_handle_t participantHandle, esif_handle_t domainHandle) = 0;

	virtual esif_handle_t getParticipantHandle(UIntN participantIndex) = 0;
	virtual UIntN getParticipantIndex(esif_handle_t participantHandle) = 0;

	virtual esif_handle_t getDomainHandle(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual UIntN getDomainIndex(esif_handle_t participantHandle, esif_handle_t domainHandle) = 0;
};
