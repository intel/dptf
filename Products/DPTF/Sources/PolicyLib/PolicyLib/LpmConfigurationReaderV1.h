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

#include "LpmConfigurationReader.h"

using namespace std;

class dptf_export LpmConfigurationReaderV1 final : public LpmConfigurationReader
{
public:
    LpmConfigurationReaderV1(PolicyServicesInterfaceContainer policyServices);
    ~LpmConfigurationReaderV1();

private:

    string readTargetDeviceAcpiScope(void);
    DomainType::Type readDomainType(void);
    ControlKnobType::Type readControlKnob(void);
    UIntN readControlValue(void);

    vector<string> readAppNames(UInt32 entryIndex);

    vector<LpmEntry> readStandardEntries(void) override;
    vector<AppSpecificEntry> readAppSpecificEntries(void) override;
    vector<LpmSet> readLpmSets(void) override;
    vector<LpmEntry> readLpmEntries(void);

    Bool validControlKnob(UInt32 value) const;
    void setLpmEntryKeyPath(string keyPath, UIntN index);
    void setLpmSetsKeyPath(UIntN index);
};