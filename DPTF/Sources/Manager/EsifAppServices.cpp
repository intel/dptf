/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

eIfaceType EsifAppServices::getInterfaceType(void)
{
    return m_esifInterface.fIfaceType;
}

UInt16 EsifAppServices::getInterfaceVersion(void)
{
    return m_esifInterface.fIfaceVersion;
}

UInt64 EsifAppServices::getInterfaceSize(void)
{
    return m_esifInterface.fIfaceSize;
}

eEsifError EsifAppServices::getConfigurationValue(const void* esifHandle, const void* appHandle, 
    const EsifDataPtr nameSpace, const EsifDataPtr elementPath, EsifDataPtr elementValue)
{
    return m_esifInterface.fGetConfigFuncPtr(esifHandle, appHandle, nameSpace, elementPath, elementValue);
}

eEsifError EsifAppServices::setConfigurationValue(const void* esifHandle, const void* appHandle, 
    const EsifDataPtr nameSpace, const EsifDataPtr elementPath, const EsifDataPtr elementValue, 
    const EsifFlags elementFlags)
{
    return m_esifInterface.fSetConfigFuncPtr(esifHandle, appHandle, nameSpace, elementPath, elementValue, 
        elementFlags);
}

eEsifError EsifAppServices::executePrimitive(const void* esifHandle, const void* appHandle, 
    const void* participantHandle, const void* domainHandle, const EsifDataPtr request, EsifDataPtr response, 
    ePrimitiveType primitive, const UInt8 instance)
{
    return m_esifInterface.fPrimitiveFuncPtr(esifHandle, appHandle, participantHandle, domainHandle, 
        request, response, primitive, instance);
}

eEsifError EsifAppServices::writeLog(const void* esifHandle, const void* appHandle, 
    const void* participantHandle, const void* domainHandle, const EsifDataPtr message, const eLogType logType)
{
    return m_esifInterface.fWriteLogFuncPtr(esifHandle, appHandle, participantHandle, domainHandle, message, logType);
}

eEsifError EsifAppServices::registerForEvent(const void* esifHandle, const void* appHandle, 
    const void* participantHandle, const void* domainHandle, const EsifDataPtr eventGuid)
{
    return m_esifInterface.fRegisterEventFuncPtr(esifHandle, appHandle, participantHandle, domainHandle, eventGuid);
}

eEsifError EsifAppServices::unregisterForEvent(const void* esifHandle, const void* appHandle, 
    const void* participantHandle, const void* domainHandle, const EsifDataPtr eventGuid)
{
    return m_esifInterface.fUnregisterEventFuncPtr(esifHandle, appHandle, participantHandle, domainHandle, eventGuid);
}

eEsifError EsifAppServices::sendEvent(const void* esifHandle, const void* appHandle,
    const void* participantHandle, const void* domainHandle, const EsifDataPtr eventData, const EsifDataPtr eventGuid)
{
    return m_esifInterface.fSendEventFuncPtr(esifHandle, appHandle, participantHandle, domainHandle, eventData, eventGuid);
}