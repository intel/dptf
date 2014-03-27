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

#include "BinaryParse.h"

std::vector<PerformanceControl> BinaryParse::genericPpssObject(UInt32 dataLength, void* esifData)
{
    std::vector<PerformanceControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(esifData);
    struct EsifDataBinaryPpssPackage* currentRow = reinterpret_cast<struct EsifDataBinaryPpssPackage*>(data);

    validateData(dataLength);

    UIntN rows = countPpssRows(dataLength, data);

    // Reset currentRow to point to the beginning of the data block
    data = reinterpret_cast<UInt8*>(esifData);
    currentRow = reinterpret_cast<struct EsifDataBinaryPpssPackage*>(data);

    for (UIntN i = 0; i < rows; i++)
    {
        PerformanceControl temp(
            static_cast<UInt32>(currentRow->control.integer.value),
            PerformanceControlType::PerformanceState,
            static_cast<UInt32>(currentRow->power.integer.value),
            Percentage(static_cast<UInt32>(currentRow->performancePercentage.integer.value) / 100.0),
            static_cast<UInt32>(currentRow->latency.integer.value),
            static_cast<UInt32>(currentRow->rawPerformance.integer.value),
            std::string(
                reinterpret_cast<const char*>(&(currentRow->rawUnits)) + sizeof(union esif_data_variant),
                currentRow->rawUnits.string.length
                ));

        controls.push_back(temp);

        data += sizeof(struct EsifDataBinaryPpssPackage) + currentRow->rawUnits.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryPpssPackage*>(data);
    }

    return controls;
}

UIntN BinaryParse::countPpssRows(UIntN size, UInt8* data)
{
    IntN bytesRemaining = size;
    UIntN rows = 0;

    struct EsifDataBinaryPpssPackage* currentRow = reinterpret_cast<struct EsifDataBinaryPpssPackage*>(data);

    while (bytesRemaining > 0)
    {
        bytesRemaining -= sizeof(struct EsifDataBinaryPpssPackage);
        bytesRemaining -= currentRow->rawUnits.string.length;

        if (bytesRemaining >= 0)
        {
            // The math done here will vary based on the number of strings in the BIOS object
            rows++;
            data += sizeof(struct EsifDataBinaryPpssPackage) + currentRow->rawUnits.string.length;
            currentRow = reinterpret_cast<struct EsifDataBinaryPpssPackage*>(data);
        }
        else // Data size mismatch, we went negative
        {
            throw dptf_exception("Expected binary data size mismatch. (PPSS)");
        }
    }

    return rows;
}

std::vector<ThermalRelationshipTableEntry> BinaryParse::passiveTrtObject(UInt32 dataLength, void* esifData)
{
    std::vector<ThermalRelationshipTableEntry> controls;
    UInt8* data = reinterpret_cast<UInt8*>(esifData);
    struct EsifDataBinaryTrtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

    validateData(dataLength);

    UIntN rows = countTrtRows(dataLength, data);

    // Reset currentRow to point to the beginning of the data block
    data = reinterpret_cast<UInt8*>(esifData);
    currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

    for (UIntN i = 0; i < rows; i++)
    {
        // Since the TRT has 2 strings in it, the process for extracting them is:
        //  1. Extract the source at the beginning of the structure
        //  2. Since the actual string data is placed between the source and target, the pointer needs moved
        //  3. Move the pointer past the source string data and set current row
        //  4. Now the targetDevice field will actually point to the right spot
        //  5. Extract target device
        //  6. Move the pointer as before (past the targetDevice string data) and set current row
        //  7. Extract the remaining fields
        //  8. Point data and currentRow to the next row

        std::string source(
            reinterpret_cast<const char*>(&(currentRow->sourceDevice)) + sizeof(union esif_data_variant),
            currentRow->sourceDevice.string.length);

        data += currentRow->sourceDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

        std::string target(
            reinterpret_cast<const char*>(&(currentRow->targetDevice)) + sizeof(union esif_data_variant),
            currentRow->targetDevice.string.length);

        data += currentRow->targetDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

        ThermalRelationshipTableEntry temp(
            normalizeAcpiScope(source),
            normalizeAcpiScope(target),
            static_cast<UInt32>(currentRow->thermalInfluence.integer.value),
            static_cast<UInt32>(currentRow->thermalSamplingPeriod.integer.value));

        controls.push_back(temp);

        // Since we've already accounted for the strings, we now move the pointer by the size of the structure
        //  to get to the next row.
        data += sizeof(struct EsifDataBinaryTrtPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);
    }

    return controls;
}

UIntN BinaryParse::countTrtRows(UInt32 size, UInt8* data)
{
    IntN bytesRemaining = size;
    UIntN rows = 0;

    struct EsifDataBinaryTrtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

    while (bytesRemaining > 0)
    {
        bytesRemaining -= sizeof(struct EsifDataBinaryTrtPackage);
        bytesRemaining -= currentRow->sourceDevice.string.length;

        data += currentRow->sourceDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

        bytesRemaining -= currentRow->targetDevice.string.length;

        data += currentRow->targetDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

        if (bytesRemaining >= 0)
        {
            // The math done here will vary based on the number of strings in the BIOS object
            rows++;

            data += sizeof(struct EsifDataBinaryTrtPackage);
            currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);
        }
        else // Data size mismatch, we went negative
        {
            throw dptf_exception("Expected binary data size mismatch. (TRT)");
        }
    }

    return rows;
}

std::vector<ActiveRelationshipTableEntry> BinaryParse::activeArtObject(UInt32 dataLength, void* binaryData)
{
    std::vector<ActiveRelationshipTableEntry> controls;
    UInt8* data = reinterpret_cast<UInt8*>(binaryData);
    struct EsifDataBinaryArtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

    validateData(dataLength);

    UIntN rows = countArtRows(dataLength, data);

    // Reset currentRow to point to the beginning of the data block
    data = reinterpret_cast<UInt8*>(binaryData);
    data += sizeof(esif_data_variant); //Ignore revision field
    currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

    for (UIntN i = 0; i < rows; i++)
    {
        // Since the ART has 2 strings in it, the process for extracting them is:
        //  1. Extract the source at the beginning of the structure
        //  2. Since the actual string data is placed between the source and target, the pointer needs moved
        //  3. Move the pointer past the source string data and set current row
        //  4. Now the targetDevice field will actually point to the right spot
        //  5. Extract target device
        //  6. Move the pointer as before (past the targetDevice string data) and set current row
        //  7. Extract the remaining fields
        //  8. Point data and currentRow to the next row

        std::string source(
            reinterpret_cast<const char*>(&(currentRow->sourceDevice)) + sizeof(union esif_data_variant),
            currentRow->sourceDevice.string.length);

        data += currentRow->sourceDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

        std::string target(
            reinterpret_cast<const char*>(&(currentRow->targetDevice)) + sizeof(union esif_data_variant),
            currentRow->targetDevice.string.length);

        data += currentRow->targetDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

        std::vector <UInt32> acEntries;
        acEntries.push_back(static_cast<UInt32>(currentRow->ac0MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac1MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac2MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac3MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac4MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac5MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac6MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac7MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac8MaxFanSpeed.integer.value));
        acEntries.push_back(static_cast<UInt32>(currentRow->ac9MaxFanSpeed.integer.value));

        ActiveRelationshipTableEntry temp(
            normalizeAcpiScope(source),
            normalizeAcpiScope(target),
            static_cast<UInt32>(currentRow->weight.integer.value),
            acEntries);

        controls.push_back(temp);

        // Since we've already accounted for the strings, we now move the pointer by the size of the structure
        //  to get to the next row.
        data += sizeof(struct EsifDataBinaryArtPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);
    }

    return controls;
}

UIntN BinaryParse::countArtRows(UInt32 size, UInt8* data)
{
    IntN bytesRemaining = size;
    UIntN rows = 0;

    //Remove revision field
    data += sizeof(esif_data_variant);
    bytesRemaining -= sizeof(esif_data_variant);

    struct EsifDataBinaryArtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

    while (bytesRemaining > 0)
    {
        bytesRemaining -= sizeof(struct EsifDataBinaryArtPackage);
        bytesRemaining -= currentRow->sourceDevice.string.length;

        data += currentRow->sourceDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

        bytesRemaining -= currentRow->targetDevice.string.length;

        data += currentRow->targetDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

        if (bytesRemaining >= 0)
        {
            // The math done here will vary based on the number of strings in the BIOS object
            rows++;

            data += sizeof(struct EsifDataBinaryArtPackage);
            currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);
        }
        else // Data size mismatch, we went negative
        {
            throw dptf_exception("Expected binary data size mismatch. (ART)");
        }
    }

    return rows;
}

ActiveControlStaticCaps* BinaryParse::fanFifObject(UInt32 dataLength, void* esifData)
{
    ActiveControlStaticCaps* control;
    UInt8* data = reinterpret_cast<UInt8*>(esifData);
    struct EsifDataBinaryFifPackage* currentRow = reinterpret_cast<struct EsifDataBinaryFifPackage*>(data);

    validateData(dataLength);

    if (dataLength != sizeof(EsifDataBinaryFifPackage))
    {
        //Data size mismatch
        throw dptf_exception("Data size mismatch.");
    }

    control = new ActiveControlStaticCaps(
        (static_cast<UInt32>(currentRow->hasFineGrainControl.integer.value) != 0),
        (static_cast<UInt32>(currentRow->supportsLowSpeedNotification.integer.value) != 0),
        static_cast<UInt32>(currentRow->stepSize.integer.value));

    return control;
}

std::vector<ActiveControl> BinaryParse::fanFpsObject(UInt32 dataLength, void* esifData)
{
    std::vector<ActiveControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(esifData);
    data += sizeof(esif_data_variant); //Ignore revision field
    struct EsifDataBinaryFpsPackage* currentRow = reinterpret_cast<struct EsifDataBinaryFpsPackage*>(data);

    validateData(dataLength);

    // Subtracting one data variant here for the revision field
    UIntN rows = (dataLength - sizeof(esif_data_variant)) / sizeof(EsifDataBinaryFpsPackage);

    if ((dataLength - sizeof(esif_data_variant)) % sizeof(EsifDataBinaryFpsPackage))
    {
        // Data size mismatch, should be evenly divisible
        throw dptf_exception("Data size mismatch.");
    }

    for (UIntN i = 0; i < rows; i++)
    {
        ActiveControl temp(
            static_cast<UInt32>(currentRow->control.integer.value),
            static_cast<UInt32>(currentRow->tripPoint.integer.value),  // May want to represent this differently; -1 is MAX_INT for whatever type
            static_cast<UInt32>(currentRow->speed.integer.value),
            static_cast<UInt32>(currentRow->noiseLevel.integer.value),
            static_cast<UInt32>(currentRow->power.integer.value));

        controls.push_back(temp);

        data += sizeof(struct EsifDataBinaryFpsPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryFpsPackage*>(data);
    }

    return controls;
}

ActiveControlStatus* BinaryParse::fanFstObject(UInt32 dataLength, void* esifData)
{
    ActiveControlStatus* control;
    UInt8* data = reinterpret_cast<UInt8*>(esifData);
    struct EsifDataBinaryFstPackage* currentRow = reinterpret_cast<struct EsifDataBinaryFstPackage*>(data);

    validateData(dataLength);

    if (dataLength != sizeof(EsifDataBinaryFstPackage))
    {
        //Data size mismatch
        throw dptf_exception("Data size mismatch.");
    }

    control = new ActiveControlStatus(
        static_cast<UInt32>(currentRow->control.integer.value),
        static_cast<UInt32>(currentRow->speed.integer.value));

    return control;
}

LpmTable BinaryParse::lpmTableObject(UInt32 dataLength, void* binaryData)
{
    std::vector<LpmEntry> lpmTableEntries;
    UInt8* data = reinterpret_cast<UInt8*>(binaryData);
    struct EsifDataBinaryLpmtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryLpmtPackage*>(data);
    esif_data_variant* dataVariant;
    LpmConfigurationVersion::Type version;

    validateData(dataLength);

    UIntN rows = countLpmtRows(dataLength, data);

    dataVariant = reinterpret_cast<esif_data_variant*>(binaryData);
    version = static_cast<LpmConfigurationVersion::Type>(dataVariant->integer.value);

    // Reset currentRow to point to the beginning of the data block
    data = reinterpret_cast<UInt8*>(binaryData);
    data += sizeof(esif_data_variant);
    currentRow = reinterpret_cast<struct EsifDataBinaryLpmtPackage*>(data);

    for (UIntN i = 0; i < rows; i++)
    {
        std::string target(
            reinterpret_cast<const char*>(&(currentRow->targetDevice)) + sizeof(union esif_data_variant),
            currentRow->targetDevice.string.length);

        data += currentRow->targetDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryLpmtPackage*>(data);

        // Convert from esif/acpi type to dptf type.
        esif_domain_type esifDomainType =
            static_cast<esif_domain_type>(currentRow->domainType.integer.value);
        DomainType::Type domainType = EsifDomainTypeToDptfDomainType(esifDomainType);
        LpmEntry lpmEntry(
            normalizeAcpiScope(target),
            domainType,
            static_cast<ControlKnobType::Type>(currentRow->controlKnob.integer.value),
            static_cast<UInt32>(currentRow->controlValue.integer.value));

        lpmTableEntries.push_back(lpmEntry);

        // Since we've already accounted for the strings, we now move the pointer by the size of the structure
        //  to get to the next row.
        data += sizeof(struct EsifDataBinaryLpmtPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryLpmtPackage*>(data);
    }

    return (LpmTable(version, lpmTableEntries));
}

UIntN BinaryParse::countLpmtRows(UInt32 size, UInt8* data)
{
    IntN bytesRemaining = size;
    UIntN rows = 0;

    // Remove version field
    data += sizeof(esif_data_variant);
    bytesRemaining -= sizeof(esif_data_variant);

    struct EsifDataBinaryLpmtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryLpmtPackage*>(data);

    while (bytesRemaining > 0)
    {
        bytesRemaining -= sizeof(struct EsifDataBinaryLpmtPackage);
        bytesRemaining -= currentRow->targetDevice.string.length;

        data += currentRow->targetDevice.string.length;
        currentRow = reinterpret_cast<struct EsifDataBinaryLpmtPackage*>(data);

        if (bytesRemaining >= 0)
        {
            // The math done here will vary based on the number of strings in the BIOS object
            rows++;

            data += sizeof(struct EsifDataBinaryLpmtPackage);
            currentRow = reinterpret_cast<struct EsifDataBinaryLpmtPackage*>(data);
        }
        else // Data size mismatch, we went negative
        {
            throw dptf_exception("Expected binary data size mismatch. (LPMT)");
        }
    }

    return rows;
}

void BinaryParse::validateData(UInt32 size)
{
    if (size == 0)
    {
        //There is no data - impossible
        throw dptf_exception("There is no data to process.");
    }
}

UInt64 BinaryParse::extractBits(UInt16 startBit, UInt16 stopBit, UInt64 data)
{
    if (startBit < stopBit)
    {
        throw dptf_exception("The start bit must be greater than the stop bit.");
    }

    UInt64 bitCount = (startBit - stopBit) + 1;
    UInt64 mask = (1 << bitCount) - 1;

    return (data >> stopBit) & mask;
}

std::vector<PerformanceControl> BinaryParse::processorPssObject(UInt32 dataLength, void* esifData)
{
    std::vector<PerformanceControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(esifData);
    struct EsifDataBinaryPssPackage* currentRow = reinterpret_cast<struct EsifDataBinaryPssPackage*>(data);

    validateData(dataLength);

    UIntN rows = dataLength / sizeof(EsifDataBinaryPssPackage);

    if (dataLength % sizeof(EsifDataBinaryPssPackage))
    {
        // Data size mismatch, should be evenly divisible
        throw dptf_exception("Failed to parse PSS object.  The length of data received does not match the expected \
                             data length.");
    }

    for (UIntN row = 0; row < rows; row++)
    {
        if (controls.empty())
        {
            Percentage ratio(1.0);
            PerformanceControl performanceControl(
                static_cast<UInt32>(currentRow->control.integer.value),
                PerformanceControlType::PerformanceState,
                static_cast<UInt32>(currentRow->power.integer.value),
                ratio,
                static_cast<UInt32>(currentRow->latency.integer.value),
                static_cast<UInt32>(currentRow->coreFrequency.integer.value),
                std::string("MHz"));
            controls.push_back(performanceControl);
        }
        else
        {
            if (controls.front().getControlAbsoluteValue() != 0)
            {
                Percentage ratio((static_cast<UIntN>((100 * currentRow->coreFrequency.integer.value) / 
                    controls.front().getControlAbsoluteValue())) / 100.0);
                PerformanceControl performanceControl(
                    static_cast<UInt32>(currentRow->control.integer.value),
                    PerformanceControlType::PerformanceState,
                    static_cast<UInt32>(currentRow->power.integer.value),
                    ratio,
                    static_cast<UInt32>(currentRow->latency.integer.value),
                    static_cast<UInt32>(currentRow->coreFrequency.integer.value),
                    std::string("MHz"));
                controls.push_back(performanceControl);
            }
            else
            {
                throw dptf_exception("Invalid performance control set.  Performance controls will not be available for this domain.");
            }
        }

        data += sizeof(struct EsifDataBinaryPssPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryPssPackage*>(data);
    }
    return controls;
}

std::vector<PerformanceControl> BinaryParse::processorTssObject(PerformanceControl pN, UInt32 dataLength, void* esifData)
{
    std::vector<PerformanceControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(esifData);
    struct EsifDataBinaryTssPackage* currentRow = reinterpret_cast<struct EsifDataBinaryTssPackage*>(data);

    validateData(dataLength);

    UIntN rows = dataLength / sizeof(EsifDataBinaryTssPackage);

    if (dataLength % sizeof(EsifDataBinaryTssPackage))
    {
        // Data size mismatch, should be evenly divisible
        throw dptf_exception("Data size mismatch.");
    }

    for (UIntN i = 0; i < rows; i++)
    {
        Percentage performancePercentage = static_cast<UIntN>(currentRow->performancePercentage.integer.value) / 100.0;

        PerformanceControl temp(
            static_cast<UInt32>(currentRow->control.integer.value),
            PerformanceControlType::ThrottleState,
            static_cast<UInt32>(currentRow->power.integer.value),
            performancePercentage,
            static_cast<UInt32>(currentRow->latency.integer.value),
            static_cast<UIntN>(pN.getControlAbsoluteValue() * performancePercentage),
            pN.getValueUnits());

        if (temp.getControlAbsoluteValue() != 0)
        {
            controls.push_back(temp);
        }

        data += sizeof(struct EsifDataBinaryTssPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryTssPackage*>(data);
    }
    return controls;
}

std::vector<PerformanceControl> BinaryParse::processorGfxPstates(UInt32 dataLength, void* binaryData)
{
    std::vector<PerformanceControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(binaryData);
    struct EsifDataBinaryGfxPstateConfig* currentRow = reinterpret_cast<struct EsifDataBinaryGfxPstateConfig*>(data);

    validateData(dataLength);

    UIntN rows = dataLength / sizeof(EsifDataBinaryGfxPstateConfig);

    if (dataLength % sizeof(EsifDataBinaryGfxPstateConfig))
    {
        // Data size mismatch, should be evenly divisible
        throw dptf_exception("Data size mismatch.");
    }

    // Reset currentRow to point to the beginning of the data block
    data = reinterpret_cast<UInt8*>(binaryData);
    currentRow = reinterpret_cast<struct EsifDataBinaryGfxPstateConfig*>(data);

    for (UIntN i = 0; i < rows; i++)
    {
        Percentage* p;

        if (controls.empty())
        {
            p = new Percentage(1.0);
        }
        else
        {
            p = new Percentage((static_cast<UIntN>((100 * currentRow->maxRenderFrequency.integer.value)
                / controls.front().getControlAbsoluteValue())) / 100.0);
        }

        PerformanceControl temp(
            i, // GFX has no control ID so the index is used.
            PerformanceControlType::PerformanceState,
            Constants::Invalid,
            *p,
            GFX_PSTATE_TRANSITION_LATENCY,
            static_cast<UInt32>(currentRow->maxRenderFrequency.integer.value),
            std::string("MHz"));

        delete p;

        controls.push_back(temp);

        data += sizeof(struct EsifDataBinaryGfxPstateConfig);
        currentRow = reinterpret_cast<struct EsifDataBinaryGfxPstateConfig*>(data);
    }

    return controls;
}

std::vector<ConfigTdpControl> BinaryParse::processorTdplObject(UInt32 dataLength, void* binaryData)
{
    std::vector<ConfigTdpControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(binaryData);
    struct EsifDataBinaryTdplPackage* currentRow = reinterpret_cast<struct EsifDataBinaryTdplPackage*>(data);

    validateData(dataLength);

    UIntN rows = dataLength / sizeof(EsifDataBinaryTdplPackage);

    if (dataLength % sizeof(EsifDataBinaryTdplPackage))
    {
        // Data size mismatch, should be evenly divisible
        throw dptf_exception("Data size mismatch.");
    }

    for (UIntN i = 0; i < rows; i++)
    {
        ConfigTdpControl* temp = new ConfigTdpControl(
            currentRow->tdpControl.integer.value,
            currentRow->frequencyControl.integer.value,
            currentRow->tdpPower.integer.value,
            currentRow->frequency.integer.value);

        controls.push_back(*temp);
        delete temp;

        data += sizeof(struct EsifDataBinaryTdplPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryTdplPackage*>(data);
    }

    return controls;
}

std::vector<PowerControlDynamicCaps> BinaryParse::processorPpccObject(UInt32 dataLength, void* binaryData)
{
    std::vector<PowerControlDynamicCaps> controls;
    UInt8* data = reinterpret_cast<UInt8*>(binaryData);
    data += sizeof(esif_data_variant); //Ignore revision field
    struct EsifDataBinaryPpccPackage* currentRow = reinterpret_cast<struct EsifDataBinaryPpccPackage*>(data);

    validateData(dataLength);

    UIntN rows = (dataLength - sizeof(esif_data_variant)) / sizeof(EsifDataBinaryPpccPackage);

    if ((dataLength - sizeof(esif_data_variant)) % sizeof(EsifDataBinaryPpccPackage))
    {
        // Data size mismatch, should be evenly divisible
        throw dptf_exception("dataLength is invalid.");
    }

    for (UIntN i = 0; i < rows; i++)
    {
        PowerControlDynamicCaps* temp = new PowerControlDynamicCaps(
            static_cast<PowerControlType::Type>(currentRow->powerLimitIndex.integer.value),
            static_cast<UIntN>(currentRow->powerLimitMinimum.integer.value),
            static_cast<UIntN>(currentRow->powerLimitMaximum.integer.value),
            static_cast<UIntN>(currentRow->stepSize.integer.value),
            static_cast<UIntN>(currentRow->timeWindowMinimum.integer.value),
            static_cast<UIntN>(currentRow->timeWindowMaximum.integer.value),
            Percentage(0.0),
            Percentage(0.0));

        // TODO : Need to revisit if there are more than 2 power limits
        if (controls.empty())
        {
            controls.push_back(*temp);
        }
        else if (controls.front().getPowerControlType() < temp->getPowerControlType())
        {
            controls.push_back(*temp);
        }
        else
        {
            controls.insert(controls.begin(), *temp);
        }

        data += sizeof(struct EsifDataBinaryPpccPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryPpccPackage*>(data);

        delete temp;
    }

    return controls;
}

CoreControlLpoPreference* BinaryParse::processorClpoObject(UInt32 dataLength, void* binaryData)
{
    CoreControlLpoPreference* preference;

    UInt8* data = reinterpret_cast<UInt8*>(binaryData);
    data += sizeof(esif_data_variant); //Ignore revision field

    struct EsifDataBinaryClpoPackage* currentRow = reinterpret_cast<struct EsifDataBinaryClpoPackage*>(data);

    validateData(dataLength);

    if ((dataLength - sizeof(esif_data_variant)) % sizeof(EsifDataBinaryClpoPackage))
    {
        // Data size mismatch, should be evenly divisible
        throw dptf_exception("dataLength is invalid.");
    }

    preference = new CoreControlLpoPreference(
        (currentRow->lpoEnable.integer.value != 0),
        static_cast<UIntN>(currentRow->startPstateIndex.integer.value),
        Percentage(static_cast<UIntN>(currentRow->stepSize.integer.value) / 100.0),
        static_cast<CoreControlOffliningMode::Type>(currentRow->powerControlSetting.integer.value),
        static_cast<CoreControlOffliningMode::Type>(currentRow->performanceControlSetting.integer.value));

    return preference;
}

std::vector<DisplayControl> BinaryParse::displayBclObject(UInt32 dataLength, void* binaryData)
{
    std::vector<DisplayControl> controls;

    UInt8* data = reinterpret_cast<UInt8*>(binaryData);
    struct EsifDataBinaryBclPackage* currentRow = reinterpret_cast<struct EsifDataBinaryBclPackage*>(data);

    validateData(dataLength);

    UIntN rows = dataLength / sizeof(EsifDataBinaryBclPackage);

    if (dataLength % sizeof(EsifDataBinaryBclPackage))
    {
        // Data size mismatch, should be evenly divisible
        throw dptf_exception("Data size mismatch.");
    }

    for (UIntN i = 0; i < rows; i++)
    {
        DisplayControl temp(Percentage(static_cast<UInt32>(currentRow->brightnessLevel.integer.value) / 100.0));

        controls.push_back(temp);

        data += sizeof(struct EsifDataBinaryBclPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryBclPackage*>(data);
    }

    return controls;
}

// TODO : Values come in from ESIF as u64's and the majority of the time I cast them to UIntN...


std::string BinaryParse::normalizeAcpiScope(const std::string& acpiScope)
{
    std::stringstream normalizedAcpiScope;
    UIntN charsSinceLastDot(0);
    for (UIntN pos = 0; pos < acpiScope.size(); pos++)
    {
        if (acpiScope[pos] == '\\')
        {
            normalizedAcpiScope << acpiScope[pos];
            charsSinceLastDot = 0;
        }
        else if (acpiScope[pos] == '.')
        {
            IntN underscoresToAdd = 4 - charsSinceLastDot;
            while (underscoresToAdd > 0)
            {
                normalizedAcpiScope << '_';
                underscoresToAdd--;
            }
            normalizedAcpiScope << acpiScope[pos];
            charsSinceLastDot = 0;
        }
        else if (acpiScope[pos] == '\0')
        {
            continue;
        }
        else
        {
            normalizedAcpiScope << acpiScope[pos];
            charsSinceLastDot++;
        }
    }

    if (acpiScope.size() > 0)
    {
        IntN underscoresToAdd = 4 - charsSinceLastDot;
        while (underscoresToAdd > 0)
        {
            normalizedAcpiScope << '_';
            underscoresToAdd--;
        }
    }

    return normalizedAcpiScope.str();
}