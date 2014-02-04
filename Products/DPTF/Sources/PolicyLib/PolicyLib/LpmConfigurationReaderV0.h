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

class dptf_export LpmConfigurationReaderV0 final : public LpmConfigurationReader
{
public:
    LpmConfigurationReaderV0(PolicyServicesInterfaceContainer policyServices);
    ~LpmConfigurationReaderV0();

private:

    string                          m_skuData;
    vector<LpmSet>                  m_lpmSets;

    CoreControlOffliningMode::Type readCpuOffliningMode();
    UInt32 readCpuPercentageActiveLogicalProcessors();
    UInt32 readCpuTargetFrequency();
    Bool readCpuUseTStateThrottling();
    UInt32 readPackagePowerLimit();
    UInt32 readGfxTargetFrequency();
    vector<string> readAppNames(UInt32 entryIndex);

    vector<LpmEntry> readStandardEntries(void) override;
    vector<AppSpecificEntry> readAppSpecificEntries(void) override;
    vector<LpmSet> readLpmSets(void) override;
    vector<LpmEntry> readLpmEntries(void);
    vector<LpmEntry> convertV0LpmEntriesToV1Format(
        CoreControlOffliningMode::Type cpuOffliningMode,
        UInt32 cpuPercentageActiveLogicalProcessors,
        UInt32 cpuTargetFrequency,
        Bool cpuUseTStateThrottling,
        UInt32 packagePowerLimit,
        UInt32 gfxTargetFrequency );
    UInt32 extractPowerLimit(string plString);

    std::string skuData(void) const;
    void updateSkuData(void);
};
