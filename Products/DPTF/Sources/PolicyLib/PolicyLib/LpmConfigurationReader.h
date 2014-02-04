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

#include "LpmConfiguration.h"

namespace LpmEntriesIndexLimit
{
    enum MaxEntries
    {
        MaxAppEntryIndex = 100,
        MaxLpmSetIndex = 102,   // To take care of 0,1 indexes which is not valid.
        MaxLpmEntryIndex = 100
    };

    enum MinEntries
    {
        MinLpmSetIndex = 2
    };
}

class dptf_export LpmConfigurationReader : public LpmConfigurationHelper
{
public:
    LpmConfigurationReader(string root, PolicyServicesInterfaceContainer policyServices);
    virtual ~LpmConfigurationReader() = 0;

    virtual void setKeyPath(string keyPath);
    virtual string root(void) const;
    virtual string keyPath(void) const;
    virtual vector<string> tokenize(string inputString, Int8 c);
    virtual void trim(string& stringToTrim, Int8 c);

    virtual vector<LpmEntry> readStandardEntries(void) = 0;
    virtual vector<AppSpecificEntry> readAppSpecificEntries(void) = 0;
    virtual vector<LpmSet> readLpmSets(void) = 0;

protected:

    string getIndexAsString(UIntN index);

private:

    string m_root;
    string m_keyPath;

};

