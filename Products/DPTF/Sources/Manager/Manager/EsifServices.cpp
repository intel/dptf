/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#include "EsifServices.h"
#include "DptfManager.h"
#include "ParticipantManager.h"
#include "IndexContainer.h"
#include "Constants.h"
#include "EsifData.h"
#include "EsifDataPercentage.h"
#include "EsifDataFrequency.h"
#include "EsifDataPower.h"
#include "EsifDataString.h"
#include "EsifDataTemperature.h"
#include "EsifDataUInt8.h"
#include "EsifDataUInt32.h"
#include "EsifDataUInt64.h"
#include "EsifDataVoid.h"
#include "EsifDataGuid.h"
#include "esif_ccb_memory.h"
#include "esif_rc.h"
#include "ManagerMessage.h"

EsifServices::EsifServices(const DptfManager* dptfManager, const void* esifHandle,
    const EsifInterfacePtr esifInterfacePtr, eLogType currentLogVerbosityLevel) :
    m_dptfManager(dptfManager), m_esifHandle(esifHandle), m_currentLogVerbosityLevel(currentLogVerbosityLevel)
{
    esif_ccb_memcpy(&m_esifInterface, esifInterfacePtr, sizeof(EsifInterface));
}

eLogType EsifServices::getCurrentLogVerbosityLevel(void) const
{
    return m_currentLogVerbosityLevel;
}

void EsifServices::setCurrentLogVerbosityLevel(eLogType currentLogVerbosityLevel)
{
    m_currentLogVerbosityLevel = currentLogVerbosityLevel;
}

UInt32 EsifServices::readConfigurationUInt32(const std::string& elementPath)
{
    EsifDataUInt32 esifResult;

    eEsifError rc = m_esifInterface.fGetConfigFuncPtr(m_esifHandle, m_dptfManager, EsifDataString("dptf"),
        EsifDataString(elementPath), esifResult);

    if (rc != ESIF_OK)
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, FLF,
            "Error returned from ESIF services interface function call");
        message.addMessage("Element Path", elementPath);
        message.setEsifErrorCode(rc);
        writeMessageWarning(message);
        throw dptf_exception(message);
    }

    return esifResult;
}

void EsifServices::writeConfigurationUInt32(const std::string& elementPath, UInt32 elementValue)
{
    eEsifError rc = m_esifInterface.fSetConfigFuncPtr(m_esifHandle, m_dptfManager, EsifDataString("dptf"),
        EsifDataString(elementPath), EsifDataUInt32(elementValue), ESIF_SERVICE_CONFIG_PERSIST);

    if (rc != ESIF_OK)
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, FLF,
            "Error returned from ESIF services interface function call");
        message.addMessage("Element Path", elementPath);
        message.addMessage("Element Value", elementValue);
        message.setEsifErrorCode(rc);
        writeMessageWarning(message);
        throw dptf_exception(message);
    }
}

std::string EsifServices::readConfigurationString(const std::string& elementPath)
{
    EsifDataString esifResult(Constants::DefaultBufferSize);

    eEsifError rc = m_esifInterface.fGetConfigFuncPtr(m_esifHandle, m_dptfManager, EsifDataString("dptf"),
        EsifDataString(elementPath), esifResult);

    if (rc != ESIF_OK)
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, FLF,
            "Error returned from ESIF services interface function call");
        message.addMessage("Element Path", elementPath);
        message.setEsifErrorCode(rc);
        writeMessageWarning(message);
        throw dptf_exception(message);
    }

    return esifResult;
}

UInt8 EsifServices::primitiveExecuteGetAsUInt8(esif_primitive_type primitive, UIntN participantIndex,
    UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    EsifDataUInt8 esifResult;

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataVoid(), esifResult, primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

    return esifResult;
}

void EsifServices::primitiveExecuteSetAsUInt8(esif_primitive_type primitive, UInt8 elementValue, UIntN participantIndex,
    UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataUInt8(elementValue), EsifDataVoid(), primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

UInt32 EsifServices::primitiveExecuteGetAsUInt32(esif_primitive_type primitive, UIntN participantIndex,
    UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    EsifDataUInt32 esifResult;

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataVoid(), esifResult, primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

    return esifResult;
}

void EsifServices::primitiveExecuteSetAsUInt32(esif_primitive_type primitive, UInt32 elementValue,
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataUInt32(elementValue), EsifDataVoid(), primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

UInt64 EsifServices::primitiveExecuteGetAsUInt64(esif_primitive_type primitive, UIntN participantIndex,
    UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    EsifDataUInt64 esifResult;

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataVoid(), esifResult, primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

    return esifResult;
}

void EsifServices::primitiveExecuteSetAsUInt64(esif_primitive_type primitive, UInt64 elementValue,
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataUInt64(elementValue), EsifDataVoid(), primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

Temperature EsifServices::primitiveExecuteGetAsTemperatureC(esif_primitive_type primitive,
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    EsifDataTemperature esifResult;

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataVoid(), esifResult, primitive, instance);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
    // Added to help debug issue with missing temperature threshold events
    if (primitive == esif_primitive_type::GET_TEMPERATURE)
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, FLF,
            "Requested participant temperature from ESIF.");
        message.addMessage("Temperature", esifResult);
        message.setEsifPrimitive(primitive, instance);
        message.setParticipantAndDomainIndex(participantIndex, domainIndex);
        message.setEsifErrorCode(rc);
        writeMessageDebug(message, MessageCategory::TemperatureThresholds);
    }
#endif

    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

    return esifResult;
}

void EsifServices::primitiveExecuteSetAsTemperatureC(esif_primitive_type primitive, Temperature temperature,
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
    // Added to help debug issue with missing temperature threshold events
    if (primitive == esif_primitive_type::SET_TEMPERATURE_THRESHOLDS)
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, FLF,
            "Setting new temperature threshold for participant.");
        message.addMessage("Temperature", temperature.toString());
        message.setEsifPrimitive(primitive, instance);
        message.setParticipantAndDomainIndex(participantIndex, domainIndex);
        writeMessageDebug(message, MessageCategory::TemperatureThresholds);
    }
#endif

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataTemperature(temperature), EsifDataVoid(), primitive, instance);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
    // Added to help debug issue with missing temperature threshold events
    if (primitive == esif_primitive_type::SET_TEMPERATURE_THRESHOLDS &&
        rc != ESIF_OK)
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, FLF,
            "Failed to set new temperature threshold.");
        message.addMessage("Temperature", temperature.toString());
        message.setEsifPrimitive(primitive, instance);
        message.setParticipantAndDomainIndex(participantIndex, domainIndex);
        message.setEsifErrorCode(rc);
        writeMessageError(message, MessageCategory::TemperatureThresholds);
    }
#endif

    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

Percentage EsifServices::primitiveExecuteGetAsPercentage(esif_primitive_type primitive,
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    EsifDataPercentage esifResult;

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataVoid(), esifResult, primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

    return esifResult;
}

void EsifServices::primitiveExecuteSetAsPercentage(esif_primitive_type primitive, Percentage percentage,
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataPercentage(percentage), EsifDataVoid(), primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

Frequency EsifServices::primitiveExecuteGetAsFrequency(esif_primitive_type primitive,
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    EsifDataFrequency esifResult;

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataVoid(), esifResult, primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

    return esifResult;
}

void EsifServices::primitiveExecuteSetAsFrequency(esif_primitive_type primitive, Frequency frequency,
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataFrequency(frequency), EsifDataVoid(), primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

Power EsifServices::primitiveExecuteGetAsPower(esif_primitive_type primitive, UIntN participantIndex,
    UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    EsifDataPower esifResult;

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataVoid(), esifResult, primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

    return esifResult;
}

void EsifServices::primitiveExecuteSetAsPower(esif_primitive_type primitive, Power power,
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataPower(power), EsifDataVoid(), primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

std::string EsifServices::primitiveExecuteGetAsString(esif_primitive_type primitive,
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    EsifDataString esifResult(Constants::DefaultBufferSize);

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataVoid(), esifResult, primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

    return esifResult;
}

void EsifServices::primitiveExecuteSetAsString(esif_primitive_type primitive, std::string stringValue, 
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataString(stringValue), EsifDataVoid(), primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

void EsifServices::primitiveExecuteGet(esif_primitive_type primitive, esif_data_type esifDataType,
    void* bufferPtr, UInt32 bufferLength, UInt32* dataLength, UIntN participantIndex,
    UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    EsifData esifData(esifDataType, bufferPtr, bufferLength, 0);

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataVoid(), esifData, primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);

    *dataLength = esifData.getDataLength();
}

void EsifServices::primitiveExecuteSet(esif_primitive_type primitive, esif_data_type esifDataType,
    void* bufferPtr, UInt32 bufferLength, UInt32 dataLength, UIntN participantIndex,
    UIntN domainIndex, UInt8 instance)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    eEsifError rc = m_esifInterface.fPrimitiveFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifData(esifDataType, bufferPtr, bufferLength, dataLength), EsifDataVoid(), primitive, instance);
    throwIfNotSuccessful(FLF, rc, primitive, participantIndex, domainIndex, instance);
}

void EsifServices::writeMessageFatal(const std::string& message, MessageCategory::Type messageCategory)
{
    if (eLogType::eLogTypeFatal <= m_currentLogVerbosityLevel)
    {
        writeMessage(eLogType::eLogTypeFatal, messageCategory, message);
    }
}

void EsifServices::writeMessageError(const std::string& message, MessageCategory::Type messageCategory)
{
    if (eLogType::eLogTypeError <= m_currentLogVerbosityLevel)
    {
        writeMessage(eLogType::eLogTypeError, messageCategory, message);
    }
}

void EsifServices::writeMessageWarning(const std::string& message, MessageCategory::Type messageCategory)
{
    if (eLogType::eLogTypeWarning <= m_currentLogVerbosityLevel)
    {
        writeMessage(eLogType::eLogTypeWarning, messageCategory, message);
    }
}

void EsifServices::writeMessageInfo(const std::string& message, MessageCategory::Type messageCategory)
{
    if (eLogType::eLogTypeInfo <= m_currentLogVerbosityLevel)
    {
        writeMessage(eLogType::eLogTypeInfo, messageCategory, message);
    }
}

void EsifServices::writeMessageDebug(const std::string& message, MessageCategory::Type messageCategory)
{
    if (eLogType::eLogTypeDebug <= m_currentLogVerbosityLevel)
    {
        writeMessage(eLogType::eLogTypeDebug, messageCategory, message);
    }
}

void EsifServices::registerEvent(FrameworkEvent::Type frameworkEvent, UIntN participantIndex, UIntN domainIndex)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    Guid guid = FrameworkEventInfo::instance()->getGuid(frameworkEvent);

    eEsifError rc = m_esifInterface.fRegisterEventFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataGuid(guid));

    // FIXME:  this should throw an exception if we get an unexpected return code.  For now we will just log an
    //         error so we can see a list of items to correct.
    if (rc != ESIF_OK)
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, FLF,
            "Error returned from ESIF register event function call");
        message.setFrameworkEvent(frameworkEvent);
        message.addMessage("Guid", guid.toString());
        message.setParticipantAndDomainIndex(participantIndex, domainIndex);
        message.setEsifErrorCode(rc);
        writeMessageWarning(message);
    }
}

void EsifServices::unregisterEvent(FrameworkEvent::Type frameworkEvent, UIntN participantIndex, UIntN domainIndex)
{
    throwIfParticipantDomainCombinationInvalid(FLF, participantIndex, domainIndex);

    Guid guid = FrameworkEventInfo::instance()->getGuid(frameworkEvent);

    eEsifError rc = m_esifInterface.fUnregisterEventFuncPtr(m_esifHandle, m_dptfManager,
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(participantIndex),
        (void*)m_dptfManager->getIndexContainer()->getIndexPtr(domainIndex),
        EsifDataGuid(guid));

    // FIXME:  this should throw an exception if we get an unexpected return code.  For now we will just log an
    //         error so we can see a list of items to correct.
    if (rc != ESIF_OK)
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, FLF,
            "Error returned from ESIF unregister event function call");
        message.setFrameworkEvent(frameworkEvent);
        message.addMessage("Guid", guid.toString());
        message.setParticipantAndDomainIndex(participantIndex, domainIndex);
        message.setEsifErrorCode(rc);
        writeMessageWarning(message);
    }
}

void EsifServices::writeMessage(eLogType messageLevel, MessageCategory::Type messageCategory, const std::string& message)
{
    // Do not throw an error here....
    // In general we will write to the log file when an error has been thrown and we don't want to create
    // a recursive mess.  Anyway, if the message isn't written, what can we do about?  We can't write another
    // error message.  So, if this fails, we ignore it.

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
    if ((logType == eLogType::eLogTypeFatal) ||
        (logType == eLogType::eLogTypeError) ||
        (messageCategory == MessageCategory::TemperatureThresholds))
    {
#endif

        m_esifInterface.fWriteLogFuncPtr(m_esifHandle, m_dptfManager, nullptr,
            nullptr, EsifDataString(message), messageLevel);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
    }

#endif
}

void EsifServices::throwIfNotSuccessful(const std::string& fileName, UIntN lineNumber,
    const std::string& executingFunctionName, eEsifError returnCode, UIntN participantIndex, UIntN domainIndex)
{
    if (returnCode == ESIF_OK)
    {
        return;
    }

    ManagerMessage message = ManagerMessage(m_dptfManager, fileName, lineNumber, executingFunctionName,
        "Error returned from ESIF services interface function call");
    message.setParticipantAndDomainIndex(participantIndex, domainIndex);
    message.setEsifErrorCode(returnCode);

    writeMessageWarning(message);
    throw dptf_exception(message);
}

void EsifServices::throwIfNotSuccessful(const std::string& fileName, UIntN lineNumber,
    const std::string& executingFunctionName, eEsifError returnCode, esif_primitive_type primitive,
    UIntN participantIndex, UIntN domainIndex, UInt8 instance)
{
    if (returnCode == ESIF_OK)
    {
        return;
    }

    ManagerMessage message = ManagerMessage(m_dptfManager, fileName, lineNumber, executingFunctionName,
        "Error returned from ESIF services interface function call");
    message.setEsifPrimitive(primitive, instance);
    message.setParticipantAndDomainIndex(participantIndex, domainIndex);
    message.setEsifErrorCode(returnCode);

    if ((primitive == GET_TRIP_POINT_ACTIVE) && (returnCode == ESIF_I_ACPI_TRIP_POINT_NOT_PRESENT))
    {
        // no message.  we still throw an exception to inform the policy.
    }
    else
    {
        writeMessageWarning(message);
    }

    throw primitive_execution_failed(message);
}

void EsifServices::throwIfParticipantDomainCombinationInvalid(const std::string& fileName, UIntN lineNumber,
    const std::string& executingFunctionName, UIntN participantIndex, UIntN domainIndex)
{
    if ((participantIndex == Constants::Esif::NoParticipant) && (domainIndex != Constants::Esif::NoDomain))
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, fileName, lineNumber, executingFunctionName,
            "Domain index not valid without associated participant index");
        message.setParticipantAndDomainIndex(participantIndex, domainIndex);

        writeMessageWarning(message);
        throw dptf_exception(message);
    }
}
