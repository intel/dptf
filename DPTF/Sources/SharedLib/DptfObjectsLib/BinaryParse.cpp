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

#include "BinaryParse.h"
#include "esif_sdk_fan.h"
#include "EsifDataBinaryPpccPackage.h"
#include "EsifDataBinaryClpoPackage.h"

ActiveControlStaticCaps BinaryParse::fanFifObject(const DptfBuffer& buffer)
{
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryFifPackage* currentRow = reinterpret_cast<struct EsifDataBinaryFifPackage*>(data);

    validateData(buffer.size());

    if (buffer.size() != sizeof(EsifDataBinaryFifPackage))
    {
        //Data size mismatch
        throw dptf_exception("Data size mismatch.");
    }

    return ActiveControlStaticCaps(
        (static_cast<UInt32>(currentRow->hasFineGrainControl.integer.value) != 0),
        (static_cast<UInt32>(currentRow->supportsLowSpeedNotification.integer.value) != 0),
        static_cast<UInt32>(currentRow->stepSize.integer.value));
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

ActiveControlStatus BinaryParse::fanFstObject(const DptfBuffer& buffer)
{
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    struct EsifDataBinaryFstPackage* currentRow = reinterpret_cast<struct EsifDataBinaryFstPackage*>(data);

    validateData(buffer.size());

    if (buffer.size() != sizeof(EsifDataBinaryFstPackage))
    {
        //Data size mismatch
        throw dptf_exception("Data size mismatch.");
    }

    return ActiveControlStatus(
        static_cast<UInt32>(currentRow->control.integer.value),
        static_cast<UInt32>(currentRow->speed.integer.value));
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

CoreControlLpoPreference BinaryParse::processorClpoObject(const DptfBuffer& buffer)
{
    UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
    data += sizeof(esif_data_variant); //Ignore revision field

    struct EsifDataBinaryClpoPackage* currentRow = reinterpret_cast<struct EsifDataBinaryClpoPackage*>(data);

    validateData(buffer.size());

    if ((buffer.size() - sizeof(esif_data_variant)) % sizeof(EsifDataBinaryClpoPackage))
    {
        // Data size mismatch, should be evenly divisible
        throw dptf_exception("dataLength is invalid.");
    }

    return CoreControlLpoPreference(
        (currentRow->lpoEnable.integer.value != 0),
        static_cast<UIntN>(currentRow->startPstateIndex.integer.value),
        Percentage(static_cast<UIntN>(currentRow->stepSize.integer.value) / 100.0),
        static_cast<CoreControlOffliningMode::Type>(currentRow->powerControlSetting.integer.value),
        static_cast<CoreControlOffliningMode::Type>(currentRow->performanceControlSetting.integer.value));
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
