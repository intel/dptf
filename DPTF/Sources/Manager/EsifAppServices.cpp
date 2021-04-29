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

#include "EsifAppServices.h"
#include "esif_ccb_memory.h"

EsifAppServices::EsifAppServices(const EsifInterfacePtr esifInterfacePtr)
{
	esif_ccb_memcpy(&m_esifInterface, esifInterfacePtr, sizeof(m_esifInterface));
}

EsifAppServices::~EsifAppServices()
{
}

eEsifError EsifAppServices::getConfigurationValue(
	const esif_handle_t esifHandle,
	const esif_handle_t appHandle,
	const EsifDataPtr nameSpace,
	const EsifDataPtr elementPath,
	EsifDataPtr elementValue)
{
	UNREFERENCED_PARAMETER(appHandle);
	return m_esifInterface.fGetConfigFuncPtr(esifHandle, nameSpace, elementPath, elementValue);
}

eEsifError EsifAppServices::setConfigurationValue(
	const esif_handle_t esifHandle,
	const esif_handle_t appHandle,
	const EsifDataPtr nameSpace,
	const EsifDataPtr elementPath,
	const EsifDataPtr elementValue,
	const EsifFlags elementFlags)
{
	UNREFERENCED_PARAMETER(appHandle);
	return m_esifInterface.fSetConfigFuncPtr(esifHandle, nameSpace, elementPath, elementValue, elementFlags);
}

eEsifError EsifAppServices::executePrimitive(
	const esif_handle_t esifHandle,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr request,
	EsifDataPtr response,
	const ePrimitiveType primitive,
	const UInt8 instance)
{
	UNREFERENCED_PARAMETER(appHandle);
	return m_esifInterface.fPrimitiveFuncPtr(
		esifHandle, participantHandle, domainHandle, request, response, primitive, instance);
}

eEsifError EsifAppServices::writeLog(
	const esif_handle_t esifHandle,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr message,
	const eLogType logType)
{
	UNREFERENCED_PARAMETER(appHandle);
	return m_esifInterface.fWriteLogFuncPtr(esifHandle, participantHandle, domainHandle, message, logType);
}

eEsifError EsifAppServices::registerForEvent(
	const esif_handle_t esifHandle,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventGuid)
{
	UNREFERENCED_PARAMETER(appHandle);
	return m_esifInterface.fRegisterEventFuncPtr(esifHandle, participantHandle, domainHandle, eventGuid);
}

eEsifError EsifAppServices::unregisterForEvent(
	const esif_handle_t esifHandle,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventGuid)
{
	UNREFERENCED_PARAMETER(appHandle);
	return m_esifInterface.fUnregisterEventFuncPtr(esifHandle, participantHandle, domainHandle, eventGuid);
}

eEsifError EsifAppServices::sendEvent(
	const esif_handle_t esifHandle,
	const esif_handle_t appHandle,
	const esif_handle_t participantHandle,
	const esif_handle_t domainHandle,
	const EsifDataPtr eventData,
	const EsifDataPtr eventGuid)
{
	UNREFERENCED_PARAMETER(appHandle);
	return m_esifInterface.fSendEventFuncPtr(
		esifHandle, participantHandle, domainHandle, eventData, eventGuid);
}

eEsifError EsifAppServices::sendCommand(
	const esif_handle_t esifHandle,
	const esif_handle_t appHandle,
	const UInt32 argc,
	const EsifDataArray argv,
	EsifDataPtr response)
{
	UNREFERENCED_PARAMETER(appHandle);
	return m_esifInterface.fSendCommandFuncPtr(
		esifHandle, argc, argv, response
	);
}