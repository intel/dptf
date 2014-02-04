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
#include <vector>
#include <string>
#include "XmlNode.h"

using namespace std;

class dptf_export AppSpecificEntry final
{
public:
    AppSpecificEntry(vector<string> appNames, UIntN lpmSetIndex);
    ~AppSpecificEntry();
    UIntN lpmSetIndex(void) const;
    Bool containsAppName(string appName) const;

    XmlNode* getXml() const;

private:
    UIntN m_lpmSetIndex;
    vector<string> m_appNames;
    Bool compareAppNamesIgnoreCase(string appName1, string appName2) const;

};

