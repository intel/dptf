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

#include "LpmAppSpecificEntry.h"
#include "StatusFormat.h"
#include <algorithm>

using namespace StatusFormat;

AppSpecificEntry::AppSpecificEntry(vector<string> appNames, UIntN lpmSetIndex)
    :m_appNames(appNames),
     m_lpmSetIndex(lpmSetIndex)
{
}

AppSpecificEntry::~AppSpecificEntry()
{
}

UIntN AppSpecificEntry::lpmSetIndex(void) const
{
    return m_lpmSetIndex;
}

Bool AppSpecificEntry::containsAppName(string appName) const
{
    for (auto entry = m_appNames.begin(); entry!= m_appNames.end(); entry++)
    {
        if (compareAppNamesIgnoreCase(appName, *entry) == true)
        {
            return true;
        }
    }

    return false;
}

Bool AppSpecificEntry::compareAppNamesIgnoreCase(string appName1, string appName2) const
{
    std::transform(appName1.begin(), appName1.end(), appName1.begin(), ::tolower);
    std::transform(appName2.begin(), appName2.end(), appName2.begin(), ::tolower);

    if (appName1 == appName2)
    {
        return true;
    }

    return false;
}

XmlNode* AppSpecificEntry::getXml() const
{
    XmlNode* appEntryRoot = XmlNode::createWrapperElement("lpm_app_entry");
    string appNames;

    for (auto entry = m_appNames.begin(); entry != m_appNames.end(); entry++)
    {
        appNames += *entry;
        appNames += " ";
    }

    appEntryRoot->addChild(
        XmlNode::createDataElement("lpmset_index", friendlyValue(lpmSetIndex())));
    appEntryRoot->addChild(XmlNode::createDataElement("app_names", appNames));

    return appEntryRoot;
}
