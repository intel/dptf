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

#pragma once

#include "Dptf.h"
#include "esif_primitive_type.h"
#include "esif_data_type.h"

#include <string>

class EsifPrimitiveInterface
{
public:

    virtual ~EsifPrimitiveInterface()
    {
    };

    virtual UInt8 primitiveExecuteGetAsUInt8(
        esif_primitive_type primitive,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual void primitiveExecuteSetAsUInt8(
        esif_primitive_type primitive,
        UInt8 value,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual UInt32 primitiveExecuteGetAsUInt32(
        esif_primitive_type primitive,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual void primitiveExecuteSetAsUInt32(
        esif_primitive_type primitive,
        UInt32 value,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual UInt64 primitiveExecuteGetAsUInt64(
        esif_primitive_type primitive,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual void primitiveExecuteSetAsUInt64(
        esif_primitive_type primitive,
        UInt64 value,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual Temperature primitiveExecuteGetAsTemperatureC(
        esif_primitive_type primitive,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual void primitiveExecuteSetAsTemperatureC(
        esif_primitive_type primitive,
        Temperature temperature,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual Percentage primitiveExecuteGetAsPercentage(
        esif_primitive_type primitive,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual void primitiveExecuteSetAsPercentage(
        esif_primitive_type primitive,
        Percentage percentage,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual Frequency primitiveExecuteGetAsFrequency(
        esif_primitive_type primitive,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual void primitiveExecuteSetAsFrequency(
        esif_primitive_type primitive,
        Frequency frequency,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual Power primitiveExecuteGetAsPower(
        esif_primitive_type primitive,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual void primitiveExecuteSetAsPower(
        esif_primitive_type primitive,
        Power power,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual std::string primitiveExecuteGetAsString(
        esif_primitive_type primitive,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual void primitiveExecuteSetAsString(
        esif_primitive_type primitive,
        std::string stringValue,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual void primitiveExecuteGet(
        esif_primitive_type primitive,
        esif_data_type esifDataType,
        void* bufferPtr,
        UInt32 bufferLength,
        UInt32* dataLength,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;

    virtual void primitiveExecuteSet(
        esif_primitive_type primitive,
        esif_data_type esifDataType,
        void* bufferPtr,
        UInt32 bufferLength,
        UInt32 dataLength,
        UIntN domainIndex = Constants::Esif::NoDomain,
        UInt8 instance = Constants::Esif::NoInstance) = 0;
};