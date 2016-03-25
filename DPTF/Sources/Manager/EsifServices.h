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

#pragma once

#include "Dptf.h"
#include "PolicyEvent.h"
#include "ParticipantEvent.h"
#include "esif_sdk_iface_esif.h"
#include "esif_sdk_primitive_type.h"
#include "MessageCategory.h"
#include "DptfBuffer.h"
#include "EsifAppServicesInterface.h"
#include "DptfManagerInterface.h"
#include "TimeSpan.h"

//
// Implements the ESIF services interface which allows the framework to call into ESIF.  See the ESIF HLD for a
// description of the interface.  This is a C++ wrapper that forwards calls through the C interface that is used to
// communicate with ESIF.
//

class dptf_export EsifServices
{
public:

    EsifServices(const DptfManagerInterface* dptfManager, const void* esifHandle, EsifAppServicesInterface* appServices,
        eLogType currentLogVerbosityLevel);

    eLogType getCurrentLogVerbosityLevel(void) const;
    void setCurrentLogVerbosityLevel(eLogType currentLogVerbosityLevel);

    // Read/write configuration data.

    UInt32 readConfigurationUInt32(const std::string& elementPath);
    void writeConfigurationUInt32(const std::string& elementPath, UInt32 elementValue);
    std::string readConfigurationString(const std::string& elementPath);
    DptfBuffer readConfigurationBinary(const std::string& elementPath);

    // Primitives

    UInt8 primitiveExecuteGetAsUInt8(
        esif_primitive_type primitive,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    void primitiveExecuteSetAsUInt8(
        esif_primitive_type primitive,
        UInt8 elementValue,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    UInt32 primitiveExecuteGetAsUInt32(
        esif_primitive_type primitive,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    void primitiveExecuteSetAsUInt32(
        esif_primitive_type primitive,
        UInt32 elementValue,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    UInt64 primitiveExecuteGetAsUInt64(
        esif_primitive_type primitive,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    void primitiveExecuteSetAsUInt64(
        esif_primitive_type primitive,
        UInt64 elementValue,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    Temperature primitiveExecuteGetAsTemperatureC(
        esif_primitive_type primitive,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    void primitiveExecuteSetAsTemperatureC(
        esif_primitive_type primitive,
        Temperature temperature,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    Percentage primitiveExecuteGetAsPercentage(
        esif_primitive_type primitive,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    void primitiveExecuteSetAsPercentage(
        esif_primitive_type primitive,
        Percentage percentage,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    Frequency primitiveExecuteGetAsFrequency(
        esif_primitive_type primitive,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    void primitiveExecuteSetAsFrequency(
        esif_primitive_type primitive,
        Frequency frequency,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    Power primitiveExecuteGetAsPower(
        esif_primitive_type primitive,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    void primitiveExecuteSetAsPower(
        esif_primitive_type primitive,
        Power power,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    TimeSpan primitiveExecuteGetAsTimeInMilliseconds(
        esif_primitive_type primitive,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    void primitiveExecuteSetAsTimeInMilliseconds(
        esif_primitive_type primitive,
        TimeSpan time,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    std::string primitiveExecuteGetAsString(
        esif_primitive_type primitive,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    void primitiveExecuteSetAsString(
        esif_primitive_type primitive,
        std::string stringValue,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    DptfBuffer primitiveExecuteGet(
        esif_primitive_type primitive,
        esif_data_type esifDataType,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    void primitiveExecuteSet(
        esif_primitive_type primitive,
        esif_data_type esifDataType,
        void* bufferPtr,
        UInt32 bufferLength,
        UInt32 dataLength,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance);

    // Message logging

    void writeMessageFatal(const std::string& message, MessageCategory::Type messageCategory = MessageCategory::Default);
    void writeMessageError(const std::string& message, MessageCategory::Type messageCategory = MessageCategory::Default);
    void writeMessageWarning(const std::string& message, MessageCategory::Type messageCategory = MessageCategory::Default);
    void writeMessageInfo(const std::string& message, MessageCategory::Type messageCategory = MessageCategory::Default);
    void writeMessageDebug(const std::string& message, MessageCategory::Type messageCategory = MessageCategory::Default);

    // Event registration

    void registerEvent(
        FrameworkEvent::Type frameworkEvent,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain);

    void unregisterEvent(
        FrameworkEvent::Type frameworkEvent,
        UIntN participantIndex = Constants::Esif::NoParticipant,
        UIntN domainIndex = Constants::Esif::NoDomain);

    void sendDptfEvent(
        FrameworkEvent::Type frameworkEvent,
        UIntN participantIndex,
        UIntN domainIndex,
        EsifData eventData
        );
private:

    // hide the copy constructor and assignment operator.
    EsifServices(const EsifServices& rhs);
    EsifServices& operator=(const EsifServices& rhs);

    const DptfManagerInterface* m_dptfManager;
    const void* m_esifHandle;
    EsifAppServicesInterface* m_appServices;
    eLogType m_currentLogVerbosityLevel;

    void writeMessage(eLogType messageLevel, MessageCategory::Type messageCategory, const std::string& message);

    std::string getParticipantName(UIntN participantIndex);
    std::string getDomainName(UIntN participantIndex, UIntN domainIndex);

    void throwIfNotSuccessful(const std::string& fileName, UIntN lineNumber, const std::string& executingFunctionName,
        eEsifError returnCode, UIntN participantIndex, UIntN domainIndex);
    void throwIfNotSuccessful(const std::string& fileName, UIntN lineNumber, const std::string& executingFunctionName,
        eEsifError returnCode, esif_primitive_type primitive, UIntN participantIndex, UIntN domainIndex,
        UInt8 instance);
    void throwIfNotSuccessful(const std::string& fileName, UIntN lineNumber, const std::string& executingFunctionName,
        eEsifError returnCode, const std::string& messageText);
    void throwIfParticipantDomainCombinationInvalid(const std::string& fileName, UIntN lineNumber,
        const std::string& executingFunctionName, UIntN participantIndex, UIntN domainIndex);
};