/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
#include "esif_sdk_iface_esif.h"
#include "EsifAppServicesInterface.h"

class EsifAppServices : public EsifAppServicesInterface
{
public:
	EsifAppServices(const EsifInterfacePtr esifInterfacePtr);
	~EsifAppServices();

	virtual eEsifError getConfigurationValue(
		const esif_handle_t esifHandle,
		const esif_handle_t appHandle,
		const EsifDataPtr nameSpace,
		const EsifDataPtr elementPath,
		EsifDataPtr elementValue) override;

	virtual eEsifError setConfigurationValue(
		const esif_handle_t esifHandle,
		const esif_handle_t appHandle,
		const EsifDataPtr nameSpace,
		const EsifDataPtr elementPath,
		const EsifDataPtr elementValue,
		const EsifFlags elementFlags) override;

	virtual eEsifError executePrimitive(
		const esif_handle_t esifHandle,
		const esif_handle_t appHandle,
		const esif_handle_t participantHandle,
		const esif_handle_t domainHandle,
		const EsifDataPtr request,
		EsifDataPtr response,
		const ePrimitiveType primitive,
		const UInt8 instance) override;

	virtual eEsifError writeLog(
		const esif_handle_t esifHandle,
		const esif_handle_t appHandle,
		const esif_handle_t participantHandle,
		const esif_handle_t domainHandle,
		const EsifDataPtr message,
		const eLogType logType) override;

	virtual eEsifError registerForEvent(
		const esif_handle_t esifHandle,
		const esif_handle_t appHandle,
		const esif_handle_t participantHandle,
		const esif_handle_t domainHandle,
		const EsifDataPtr eventGuid) override;

	virtual eEsifError unregisterForEvent(
		const esif_handle_t esifHandle,
		const esif_handle_t appHandle,
		const esif_handle_t participantHandle,
		const esif_handle_t domainHandle,
		const EsifDataPtr eventGuid) override;

	virtual eEsifError sendEvent(
		const esif_handle_t esifHandle,
		const esif_handle_t appHandle,
		const esif_handle_t participantHandle,
		const esif_handle_t domainHandle,
		const EsifDataPtr eventData,
		const EsifDataPtr eventGuid) override;

	virtual eEsifError sendCommand(
		const esif_handle_t esifHandle,
		const esif_handle_t appHandle,
		const UInt32 argc,
		const EsifDataPtr argv,
		EsifDataPtr response) override;

private:
	EsifInterface m_esifInterface;
};
