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

#include "LpmConfigurationReaderV1.h"
#include "BinaryParse.h"



LpmConfigurationReaderV1::LpmConfigurationReaderV1(PolicyServicesInterfaceContainer policyServices)
    :LpmConfigurationReader("/lpmt/", policyServices)
{

}

LpmConfigurationReaderV1::~LpmConfigurationReaderV1()
{

}

vector<string> LpmConfigurationReaderV1::readAppNames(UInt32 entryIndex)
{
    string indexAsString = getIndexAsString(entryIndex);
    string path = "AppSpecificLpm" + indexAsString + "/";
    setKeyPath(path);

    string key = "ExecutableNames";
    string execNames = getPolicyServices().platformConfigurationData->readConfigurationString(
        root() + keyPath() + key);
    if (execNames.empty())
    {
        throw dptf_exception("Empty execNames string returned");
    }

    // Parse the string, it contains app names each separated by space.
    vector<string> appNames = tokenize(execNames, ' ');
    return appNames;
}

vector<LpmEntry> LpmConfigurationReaderV1::readStandardEntries(void)
{
    throw("Should not be called for V1");
}

vector<AppSpecificEntry> LpmConfigurationReaderV1::readAppSpecificEntries(void)
{
    vector<AppSpecificEntry> appSpecificEntries;

    //
    // Since this configuration does not have the numAppSpecificEntries we loop through
    // till we fail to read. We handle it in the catch block.
    //
    UIntN appIndex;
    try
    {
        for (appIndex = 0; ; appIndex++)
        {
            vector<string> appNames = readAppNames(appIndex);

            string key = "LPMSet";
            UIntN lpmSetIndex = getPolicyServices().platformConfigurationData->readConfigurationUInt32(
                root() + keyPath() + key);

            appSpecificEntries.push_back(AppSpecificEntry(appNames, lpmSetIndex));

            if (appIndex >= LpmEntriesIndexLimit::MaxAppEntryIndex)
            {
                throw dptf_exception("App Entries exceeded max value");
            }
        }
    }
    catch (dptf_exception& e)
    {
        string msg = e.what();
        m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
            "Error msg (" + msg + "). Last appIndex = " + to_string(appIndex),
            Constants::Invalid));
        return appSpecificEntries;
    }

    return appSpecificEntries;

}

vector<LpmSet> LpmConfigurationReaderV1::readLpmSets(void)
{
    vector<LpmSet> lpmSets;
    vector<LpmEntry> lpmEntries;

    //
    // Since this configuration does not have the numLpmEntries we loop through
    // till we fail to read. We handle it in the catch block.
    //
    UIntN lpmSetIndex;
    try
    {
        for (lpmSetIndex = LpmEntriesIndexLimit::MinLpmSetIndex; ; lpmSetIndex++)
        {
            setLpmSetsKeyPath(lpmSetIndex);

            lpmEntries = readLpmEntries();
            if (lpmEntries.empty())
            {
                // No more LPM entries to process.
                throw dptf_exception("No more LPM entries");
            }

            lpmSets.push_back(LpmSet(lpmSetIndex, lpmEntries));
            if (lpmSetIndex >= LpmEntriesIndexLimit::MaxLpmSetIndex)
            {
                throw dptf_exception("Number of LPM sets exceeded max value");
            }
        }
    }
    catch (dptf_exception& e)
    {
        string msg = e.what();
        m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
            "Error msg (" + msg + "). Last lpmSetIndex = " + to_string(lpmSetIndex),
            Constants::Invalid));
        return lpmSets;
    }

    // Will come here only if there are no entries/lpmsets.
    return lpmSets;

}

vector<LpmEntry> LpmConfigurationReaderV1::readLpmEntries(void)
{
    vector<LpmEntry> lpmEntries;

    string target;
    DomainType::Type domainType;
    ControlKnobType::Type controlKnob;
    UInt32 controlValue;

    UIntN lpmEntryIndex;
    string inputKeyPath = keyPath();
    try
    {
        for (lpmEntryIndex = 0; ; lpmEntryIndex++)
        {
            setLpmEntryKeyPath(inputKeyPath, lpmEntryIndex);

            target = readTargetDeviceAcpiScope();
            domainType = readDomainType();
            controlKnob = readControlKnob();
            controlValue = readControlValue();
            
            lpmEntries.push_back(LpmEntry(target, domainType, controlKnob, controlValue));
            if (lpmEntryIndex >= LpmEntriesIndexLimit::MaxLpmEntryIndex)
            {
                throw dptf_exception("LPM entries exceeded max value");
            }
        }
    }
    catch (dptf_exception& e)
    {
        string msg = e.what();
        m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
            "Error msg (" + msg + "). Last lpmEntryIndex = " + to_string(lpmEntryIndex),
            Constants::Invalid));
        return lpmEntries;
    }

    return (lpmEntries);
}

Bool LpmConfigurationReaderV1::validControlKnob(UInt32 value) const
{
    if ((value != ControlKnobType::PowerControlPl1) &&
        (value != ControlKnobType::PowerControlPl2) &&
        (value != ControlKnobType::PowerControlPl3) &&
        (value != ControlKnobType::PerformanceControlPerfFrequency) &&
        (value != ControlKnobType::PerformanceControlThrottleFrequency) &&
        (value != ControlKnobType::DbptControlIccMax) &&
        (value != ControlKnobType::CoreControlLpo) &&
        (value != ControlKnobType::DisplayControlBrightness) &&
        (value != ControlKnobType::ActiveCoolingControlFanSpeed) &&
        (value != ControlKnobType::ConfigTdpControlTurboState) &&
        (value != ControlKnobType::TauControlPl1) &&
        (value != ControlKnobType::TauControlPl2) &&
        (value != ControlKnobType::TauControlPl3))
    {
        return false;
    }

    return true;
}

std::string LpmConfigurationReaderV1::readTargetDeviceAcpiScope(void)
{
    string key = "TargetDeviceObjectId";
    string target = getPolicyServices().platformConfigurationData->readConfigurationString(
        root() + keyPath() + key);
    if (target.empty())
    {
        throw dptf_exception("Empty ACPI scope string returned");
    }

    string normalizedTarget = BinaryParse::normalizeAcpiScope(target);
    return normalizedTarget;
}

DomainType::Type LpmConfigurationReaderV1::readDomainType(void)
{
    string key = "DomainType";
    UIntN valueInt = getPolicyServices().platformConfigurationData->readConfigurationUInt32(
        root() + keyPath() + key);
    // Convert from esif/acpi type to dptf type.
    esif_domain_type esifDomainType = static_cast<esif_domain_type>(valueInt);
    return(EsifDomainTypeToDptfDomainType(esifDomainType));
}

ControlKnobType::Type LpmConfigurationReaderV1::readControlKnob(void)
{
    string key = "ControlKnob";
    UIntN valueInt = getPolicyServices().platformConfigurationData->readConfigurationUInt32(
        root() + keyPath() + key);
    if (validControlKnob(valueInt) == false)
    {
        throw dptf_exception("Invalid control knob returned");
    }
    return(ControlKnobType::Type(valueInt));
}

UIntN LpmConfigurationReaderV1::readControlValue(void)
{
    string key = "ControlValue";
    UIntN controlValue = getPolicyServices().platformConfigurationData->readConfigurationUInt32(
        root() + keyPath() + key);
    return controlValue;
}

void LpmConfigurationReaderV1::setLpmEntryKeyPath(string keyPath, UIntN index)
{
    string indexAsString = getIndexAsString(index);
    string path = keyPath;
    path += "LPMEntry" + indexAsString + "/";
    setKeyPath(path);
}

void LpmConfigurationReaderV1::setLpmSetsKeyPath(UIntN index)
{
    string indexAsString = getIndexAsString(index);
    string path = "LPMSets" + indexAsString + "/";
    setKeyPath(path);
}
