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

#include "LpmConfigurationReaderV0.h"
#include <regex>



LpmConfigurationReaderV0::LpmConfigurationReaderV0(PolicyServicesInterfaceContainer policyServices)
    :LpmConfigurationReader("/lpmt/", policyServices)
{

}

LpmConfigurationReaderV0::~LpmConfigurationReaderV0()
{

}

CoreControlOffliningMode::Type LpmConfigurationReaderV0::readCpuOffliningMode()
{
    string key = "CpuOffLiningMode";
    CoreControlOffliningMode::Type cpuOffliningMode;
    try
    {
        UIntN valueInt = getPolicyServices().platformConfigurationData->readConfigurationUInt32(
            root() + keyPath() + key);
        if (valueInt > CoreControlOffliningMode::Core)
        {
            m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF,
            "Invalid CoreControlOffliningMode returned (" + to_string(valueInt) + ")" +
                " - defaulting to core", Constants::Invalid));
            cpuOffliningMode = CoreControlOffliningMode::Core;
        }
        else
        {
            cpuOffliningMode = (CoreControlOffliningMode::Type)valueInt;
        }
    }
    catch(dptf_exception& e)
    {
        string msg = e.what();
        m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
            "No CoreControlOffliningMode returned - defaulting to core. msg - " + msg,
            Constants::Invalid));
        cpuOffliningMode = CoreControlOffliningMode::Core;
    }

    return cpuOffliningMode;
}

UInt32 LpmConfigurationReaderV0::readCpuPercentageActiveLogicalProcessors()
{
    string key = "CpuPercentageActiveLogicalProcessors";
    UInt32 cpuPercentageActiveLogicalProcessors;
    try
    {
        cpuPercentageActiveLogicalProcessors =
            getPolicyServices().platformConfigurationData->readConfigurationUInt32(
                root() + keyPath() + key);
    }
    catch(dptf_exception& e)
    {
        string msg = e.what();

        m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
            "No CpuPercentageActiveLogicalProcessors - defaulting to 1%. msg - " + msg,
            Constants::Invalid));

        // Default to 1% (translates to 1 core).
        cpuPercentageActiveLogicalProcessors = 1;
    }

    if (cpuPercentageActiveLogicalProcessors < 1)
    {
        cpuPercentageActiveLogicalProcessors = 1;
    }
    else if (cpuPercentageActiveLogicalProcessors > 100)
    {
        cpuPercentageActiveLogicalProcessors = 100;
    }

    return cpuPercentageActiveLogicalProcessors;
}

UInt32 LpmConfigurationReaderV0::readCpuTargetFrequency()
{
    string key = "CpuTargetFrequency";
    UInt32 cpuTargetFrequency;
    try
    {
        cpuTargetFrequency =
          getPolicyServices().platformConfigurationData->readConfigurationUInt32(
              root() + keyPath() + key);
    }
    catch(dptf_exception& e)
    {
        string msg = e.what();

        m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
        "No CpuTargetFrequency - defaulting to 800Mhz. msg - " + msg,
        Constants::Invalid));

        // if not found, we default to 800 mhz (MFM or LFM)
        cpuTargetFrequency = 800;
    }

    return cpuTargetFrequency;
}

Bool LpmConfigurationReaderV0::readCpuUseTStateThrottling()
{
    string key = "CpuUseTStateThrottling";
    Bool cpuUseTStateThrottling;
    try
    {
        UIntN valueInt =
            getPolicyServices().platformConfigurationData->readConfigurationUInt32(
            root() + keyPath() + key);
        if (valueInt)
        {
            cpuUseTStateThrottling = true;
        }
        else
        {
            cpuUseTStateThrottling = false;
        }
    }
    catch(dptf_exception& e)
    {
        string msg = e.what();
        m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
            "No CpuUseTStateThrottling - defaulting to false. msg - " + msg,
            Constants::Invalid));
        cpuUseTStateThrottling = false;
    }

    if ((cpuUseTStateThrottling != true) && (cpuUseTStateThrottling != false))
    {
        cpuUseTStateThrottling = false;
    }

    return cpuUseTStateThrottling;
}

UInt32 LpmConfigurationReaderV0::readPackagePowerLimit()
{
    // TODO: Check regarding PL -> string/uint? Do we need to sku check like 7.0?
    UInt32 packagePowerLimit;
    string key = "PackagePowerLimit";

    try
    {
        string valueString = getPolicyServices().platformConfigurationData->readConfigurationString(
            root() + keyPath() + key);
        packagePowerLimit = extractPowerLimit(valueString);
    }
    catch (dptf_exception& e)
    {
        string msg = e.what();

        m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF,
            "Error in reading PackagePowerLimit, default 10000000. msg - " + msg,
            Constants::Invalid));
        packagePowerLimit = 10000000; // Max valid power
    }

    return packagePowerLimit;
}

UInt32 LpmConfigurationReaderV0::readGfxTargetFrequency()
{
    string key = "GfxTargetFrequency";
    UInt32 gfxTargetFrequency;
    try
    {
        gfxTargetFrequency =
            getPolicyServices().platformConfigurationData->readConfigurationUInt32(
            root() + keyPath() + key);
    }
    catch(dptf_exception& e)
    {
        string msg = e.what();

        m_policyServices.messageLogging->writeMessageError(PolicyMessage(FLF,
            "Error in reading GfxTargetFrequency, default 100Mhz. msg - " + msg,
            Constants::Invalid));

        //
        // if not found, we default to 100 mhz,
        // which will be the lowest graphics p-state (since we round up)
        //
        gfxTargetFrequency = 100;
    }

    return gfxTargetFrequency;
}

// TODO: Might not need this.
UInt32 LpmConfigurationReaderV0::extractPowerLimit(string plString)
{
    UInt32 skuPlValue = 10000000; // Max valid power;
    UInt32 nonSkuPlValue = 10000000;
    Bool skuMatch = false;
    vector<string> plKeyValuePairs = tokenize(plString, ';');

    trim(plString, ' ');
    for (auto entry = plKeyValuePairs.begin(); entry != plKeyValuePairs.end(); entry++)
    {
        if (entry->find(':') != string::npos)
        {
            // There are key PL value pairs in the string.
            vector<string> plKeyValue = tokenize(*entry, ':');
            if (plKeyValue.size() != 2)
            {
                throw dptf_exception("Invalid PL-SKU entry");
            }
            else
            {
                string skuPattern = plKeyValue[0];
                string plString2 = plKeyValue[1];

                // Trim the '"'
                trim(skuPattern, '"');

                std::regex regSkuPattern(skuPattern);
                if (std::regex_match(m_skuData, regSkuPattern))
                {
                    m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
                        "SKU matched: m_skuData(" + m_skuData + ")" + " plValue(" + plString2 + ")",
                        Constants::Invalid));
                    if (!(plString2.find_first_not_of("0123456789") == string::npos))
                    {
                        throw dptf_exception("The PL value is not a number (" + plString2 + ")");
                    }

                    try
                    {
                        skuPlValue = std::stoul(plString2);
                        skuMatch = true;
                        break;
                    }
                    catch (...)
                    {
                        throw dptf_exception("Invalid PL value in sku string (" + plString2 + ")");
                    }
                }
            }
        }
        else
        {
            try
            {
                m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
                    "Non-SKU PL string: " + *entry, Constants::Invalid));

                nonSkuPlValue = std::stoul(*entry);
            }
            catch (...)
            {
                throw dptf_exception("Invalid PL value in non-sku string (" + *entry + ")");
            }
        }
    }

    if (skuMatch == true)
    {
        return skuPlValue;
    }
    else
    {
        return nonSkuPlValue;
    }
}

vector<LpmEntry> LpmConfigurationReaderV0::readLpmEntries(void)
{
    // keyPath should already have been set.
    CoreControlOffliningMode::Type cpuOffliningMode = readCpuOffliningMode();
    UInt32 cpuPercentageActiveLogicalProcessors = readCpuPercentageActiveLogicalProcessors();
    UInt32 cpuTargetFrequency = readCpuTargetFrequency();
    Bool cpuUseTStateThrottling = readCpuUseTStateThrottling();
    UInt32 packagePowerLimit = readPackagePowerLimit();
    UInt32 gfxTargetFrequency = readGfxTargetFrequency();

    vector <LpmEntry> lpmEntries = convertV0LpmEntriesToV1Format(
        cpuOffliningMode,
        cpuPercentageActiveLogicalProcessors,
        cpuTargetFrequency,
        cpuUseTStateThrottling,
        packagePowerLimit,
        gfxTargetFrequency);
    return lpmEntries;
}

vector<LpmEntry> LpmConfigurationReaderV0::convertV0LpmEntriesToV1Format(
    CoreControlOffliningMode::Type cpuOffliningMode,
    UInt32 cpuPercentageActiveLogicalProcessors,
    UInt32 cpuTargetFrequency,
    Bool cpuUseTStateThrottling,
    UInt32 packagePowerLimit,
    UInt32 gfxTargetFrequency)
{
    vector<LpmEntry> lpmEntries;
    string acpiScope;       // ACPI scope - we do not have this.
    DomainType::Type domainType;
    ControlKnobType::Type controlKnob;
    UIntN controlKnobValue;

    domainType = DomainType::Processor;
    if (cpuUseTStateThrottling == false)
    {
        controlKnob = ControlKnobType::PerformanceControlPerfFrequency;
    }
    else
    {
        controlKnob = ControlKnobType::PerformanceControlThrottleFrequency;
    }
    controlKnobValue = cpuTargetFrequency;
    lpmEntries.push_back(LpmEntry(acpiScope, domainType, controlKnob, controlKnobValue));

    domainType = DomainType::Graphics;
    controlKnob = ControlKnobType::PerformanceControlPerfFrequency;
    controlKnobValue = gfxTargetFrequency;
    lpmEntries.push_back(LpmEntry(acpiScope, domainType, controlKnob, controlKnobValue));

    domainType = DomainType::MultiFunction;
    controlKnob = ControlKnobType::PowerControlPl1; // TODO: Only PL1?
    controlKnobValue = packagePowerLimit;
    lpmEntries.push_back(LpmEntry(acpiScope, domainType, controlKnob, controlKnobValue));

    domainType = DomainType::Processor;
    controlKnob = ControlKnobType::CoreControlLpo;
    controlKnobValue = cpuPercentageActiveLogicalProcessors;
    lpmEntries.push_back(LpmEntry(acpiScope, domainType, controlKnob, controlKnobValue));

    // TODO: cpuOffliningMode -> ? V1 does not have this?

    return lpmEntries;
}

vector<LpmEntry> LpmConfigurationReaderV0::readStandardEntries(void)
{
    setKeyPath("StandardMode/");
    return (readLpmEntries());
}

vector<AppSpecificEntry> LpmConfigurationReaderV0::readAppSpecificEntries(void)
{
    m_lpmSets.clear();

    vector<AppSpecificEntry> appSpecificEntries;
    UIntN entryIndex = 0;
    try
    {
        string key = "NumAppSpecificEntries";
        UIntN numEntries = getPolicyServices().platformConfigurationData->readConfigurationUInt32(
            root() + key);

        if (numEntries == 0)
        {
            throw dptf_exception("No app specific entries for v0");
        }

        for (entryIndex = 0; entryIndex < numEntries; entryIndex++)
        {
            vector<string> appNames = readAppNames(entryIndex);
            vector<LpmEntry> lpmEntries = readLpmEntries();

            appSpecificEntries.push_back(AppSpecificEntry(appNames, entryIndex));
            m_lpmSets.push_back(LpmSet(entryIndex, lpmEntries));
        }
    }
    catch (dptf_exception& e)
    {
        string msg = e.what();
        m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
            "Error msg (" + msg + "). Last entryIndex = " + to_string(entryIndex),
            Constants::Invalid));
    }

    return appSpecificEntries;
}

vector<string> LpmConfigurationReaderV0::readAppNames(UInt32 entryIndex)
{
    string key = "AppName";
    vector<string> appNames;
    string indexAsString = getIndexAsString(entryIndex);

    setKeyPath("AppSpecificMode" + indexAsString + "/");
    try
    {
        // TODO: Error check for appname?
        string appName = getPolicyServices().platformConfigurationData->readConfigurationString(
            root() + keyPath() + key);
        appNames.push_back(appName);
    }
    catch (dptf_exception& e)
    {
        string msg = e.what();
        m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
            "Error msg (" + msg + "). Error in reading AppName", Constants::Invalid));
    }

    return appNames;
}

vector<LpmSet> LpmConfigurationReaderV0::readLpmSets(void)
{
    return m_lpmSets;
}

std::string LpmConfigurationReaderV0::skuData(void) const
{
    return m_skuData;
}

void LpmConfigurationReaderV0::updateSkuData(void)
{
    // TODO:
}

