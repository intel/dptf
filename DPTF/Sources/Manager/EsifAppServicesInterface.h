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
#include "esif_sdk_iface_app.h"

class EsifAppServicesInterface
{
public:
	virtual ~EsifAppServicesInterface(){};

	virtual eIfaceType getInterfaceType(void) = 0;
	virtual UInt16 getInterfaceVersion(void) = 0;
	virtual UInt64 getInterfaceSize(void) = 0;

	virtual eEsifError getConfigurationValue(
		const void* esifHandle,
		const void* appHandle,
		const EsifDataPtr nameSpace,
		const EsifDataPtr elementPath,
		EsifDataPtr elementValue) = 0;

	virtual eEsifError setConfigurationValue(
		const void* esifHandle,
		const void* appHandle,
		const EsifDataPtr nameSpace,
		const EsifDataPtr elementPath,
		const EsifDataPtr elementValue,
		const EsifFlags elementFlags) = 0;

	virtual eEsifError executePrimitive(
		const void* esifHandle,
		const void* appHandle,
		const void* participantHandle,
		const void* domainHandle,
		const EsifDataPtr request,
		EsifDataPtr response,
		ePrimitiveType primitive,
		const UInt8 instance) = 0;

	virtual eEsifError writeLog(
		const void* esifHandle,
		const void* appHandle,
		const void* participantHandle,
		const void* domainHandle,
		const EsifDataPtr message,
		const eLogType logType) = 0;

	virtual eEsifError registerForEvent(
		const void* esifHandle,
		const void* appHandle,
		const void* participantHandle,
		const void* domainHandle,
		const EsifDataPtr eventGuid) = 0;

	virtual eEsifError unregisterForEvent(
		const void* esifHandle,
		const void* appHandle,
		const void* participantHandle,
		const void* domainHandle,
		const EsifDataPtr eventGuid) = 0;

	virtual eEsifError sendEvent(
		const void* esifHandle,
		const void* appHandle,
		const void* participantHandle,
		const void* domainHandle,
		const EsifDataPtr eventData,
		const EsifDataPtr eventGuid) = 0;

	virtual eEsifError sendCommand(
		const void* esifHandle,
		const void* appHandle,
		const UInt32 argc,
		const EsifDataPtr argv,
		EsifDataPtr response) = 0;
};
