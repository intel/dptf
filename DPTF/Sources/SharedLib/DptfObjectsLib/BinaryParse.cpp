/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
#include "esif_sdk_fan.h"

std::vector<PerformanceControl> BinaryParse::genericPpssObject(const DptfBuffer& buffer)
{
    std::vector<PerformanceControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryPpssPackage* currentRow = reinterpret_cast<struct EsifDataBinaryPpssPackage*>(data);

    validateData(buffer.size());

    UIntN rows = countPpssRows(buffer.size(), data);

    // Reset currentRow to point to the beginning of the data block
    data = reinterpret_cast<UInt8*>(buffer.get());
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

ActiveControlStaticCaps* BinaryParse::fanFifObject(const DptfBuffer& buffer)
{
    ActiveControlStaticCaps* control;
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryFifPackage* currentRow = reinterpret_cast<struct EsifDataBinaryFifPackage*>(data);

    validateData(buffer.size());

    if (buffer.size() != sizeof(EsifDataBinaryFifPackage))
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

std::vector<ActiveControl> BinaryParse::fanFpsObject(const DptfBuffer& buffer)
{
    std::vector<ActiveControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    data += sizeof(esif_data_variant); //Ignore revision field
    struct EsifDataBinaryFpsPackage* currentRow = reinterpret_cast<struct EsifDataBinaryFpsPackage*>(data);

    validateData(buffer.size());

    // Subtracting one data variant here for the revision field
    UIntN rows = (buffer.size() - sizeof(esif_data_variant)) / sizeof(EsifDataBinaryFpsPackage);

    if ((buffer.size() - sizeof(esif_data_variant)) % sizeof(EsifDataBinaryFpsPackage))
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

ActiveControlStatus* BinaryParse::fanFstObject(const DptfBuffer& buffer)
{
    ActiveControlStatus* control;
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryFstPackage* currentRow = reinterpret_cast<struct EsifDataBinaryFstPackage*>(data);

    validateData(buffer.size());

    if (buffer.size() != sizeof(EsifDataBinaryFstPackage))
    {
        //Data size mismatch
        throw dptf_exception("Data size mismatch.");
    }

    control = new ActiveControlStatus(
        static_cast<UInt32>(currentRow->control.integer.value),
        static_cast<UInt32>(currentRow->speed.integer.value));

    return control;
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

std::vector<PerformanceControl> BinaryParse::processorPssObject(const DptfBuffer& buffer)
{
    std::vector<PerformanceControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryPssPackage* currentRow = reinterpret_cast<struct EsifDataBinaryPssPackage*>(data);

    validateData(buffer.size());

    UIntN rows = buffer.size() / sizeof(EsifDataBinaryPssPackage);

    if (buffer.size() % sizeof(EsifDataBinaryPssPackage))
    {
        // Data size mismatch, should be evenly divisible
        throw dptf_exception("Failed to parse PSS object.  The length of data received does not match the expected \
                             data length.");
    }

    for (UIntN row = 0; row < rows; row++)
    {
        Percentage ratio(1.0);

        if (!controls.empty())
        {
            if (controls.front().getControlAbsoluteValue() != 0)
            {
                ratio = (static_cast<UIntN>((100 * currentRow->coreFrequency.integer.value) / 
                    controls.front().getControlAbsoluteValue())) / 100.0;
            }
            else
            {
                ratio = (rows - row) / static_cast<double>(rows);
            }
        }

        PerformanceControl performanceControl(
            static_cast<UInt32>(currentRow->control.integer.value),
            PerformanceControlType::PerformanceState,
            static_cast<UInt32>(currentRow->power.integer.value),
            ratio,
            static_cast<UInt32>(currentRow->latency.integer.value),
            static_cast<UInt32>(currentRow->coreFrequency.integer.value),
            std::string("MHz"));
        controls.push_back(performanceControl);

        data += sizeof(struct EsifDataBinaryPssPackage);
        currentRow = reinterpret_cast<struct EsifDataBinaryPssPackage*>(data);
    }
    return controls;
}

std::vector<PerformanceControl> BinaryParse::processorTssObject(PerformanceControl pN, const DptfBuffer& buffer)
{
    std::vector<PerformanceControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryTssPackage* currentRow = reinterpret_cast<struct EsifDataBinaryTssPackage*>(data);

    validateData(buffer.size());

    UIntN rows = buffer.size() / sizeof(EsifDataBinaryTssPackage);

    if (buffer.size() % sizeof(EsifDataBinaryTssPackage))
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

std::vector<PerformanceControl> BinaryParse::processorGfxPstates(const DptfBuffer& buffer)
{
    std::vector<PerformanceControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryGfxPstateConfig* currentRow = reinterpret_cast<struct EsifDataBinaryGfxPstateConfig*>(data);

    validateData(buffer.size());

    UIntN rows = buffer.size() / sizeof(EsifDataBinaryGfxPstateConfig);

    if (buffer.size() % sizeof(EsifDataBinaryGfxPstateConfig))
    {
        // Data size mismatch, should be evenly divisible
        throw dptf_exception("Data size mismatch.");
    }

    // Reset currentRow to point to the beginning of the data block
    data = reinterpret_cast<UInt8*>(buffer.get());
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

std::vector<ConfigTdpControl> BinaryParse::processorTdplObject(const DptfBuffer& buffer)
{
    std::vector<ConfigTdpControl> controls;
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryTdplPackage* currentRow = reinterpret_cast<struct EsifDataBinaryTdplPackage*>(data);

    validateData(buffer.size());

    UIntN rows = buffer.size() / sizeof(EsifDataBinaryTdplPackage);

    if (buffer.size() % sizeof(EsifDataBinaryTdplPackage))
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

std::vector<PowerControlDynamicCaps> BinaryParse::processorPpccObject(const DptfBuffer& buffer)
{
    std::vector<PowerControlDynamicCaps> controls;
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    data += sizeof(esif_data_variant); //Ignore revision field
    struct EsifDataBinaryPpccPackage* currentRow = reinterpret_cast<struct EsifDataBinaryPpccPackage*>(data);

    validateData(buffer.size());

    UIntN rows = (buffer.size() - sizeof(esif_data_variant)) / sizeof(EsifDataBinaryPpccPackage);

    if ((buffer.size() - sizeof(esif_data_variant)) % sizeof(EsifDataBinaryPpccPackage))
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
            TimeSpan::createFromMilliseconds(static_cast<UIntN>(currentRow->timeWindowMinimum.integer.value)),
            TimeSpan::createFromMilliseconds(static_cast<UIntN>(currentRow->timeWindowMaximum.integer.value)),
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

CoreControlLpoPreference* BinaryParse::processorClpoObject(const DptfBuffer& buffer)
{
    CoreControlLpoPreference* preference;

    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    data += sizeof(esif_data_variant); //Ignore revision field

    struct EsifDataBinaryClpoPackage* currentRow = reinterpret_cast<struct EsifDataBinaryClpoPackage*>(data);

    validateData(buffer.size());

    if ((buffer.size() - sizeof(esif_data_variant)) % sizeof(EsifDataBinaryClpoPackage))
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

std::vector<DisplayControl> BinaryParse::displayBclObject(const DptfBuffer& buffer)
{
    std::vector<DisplayControl> controls;

    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryBclPackage* currentRow = reinterpret_cast<struct EsifDataBinaryBclPackage*>(data);

    validateData(buffer.size());

    UIntN rows = buffer.size() / sizeof(EsifDataBinaryBclPackage);

    if (buffer.size() % sizeof(EsifDataBinaryBclPackage))
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
            charsSinceLastDot++;
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
