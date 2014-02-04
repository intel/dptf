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

#include "LpmTable.h"
#include "StatusFormat.h"

using namespace StatusFormat;

LpmTable::LpmTable(LpmConfigurationVersion::Type version, vector<LpmEntry> lpmEntries)
    : m_version(version), m_lpmEntries(lpmEntries)
{
}

LpmTable::~LpmTable(void)
{
}

LpmConfigurationVersion::Type LpmTable::version(void) const
{
    return m_version;
}

vector<LpmEntry> LpmTable::lpmEntries(void) const
{
    return m_lpmEntries;
}

XmlNode* LpmTable::getXml(void) const
{
    XmlNode* lpmTableNode = XmlNode::createWrapperElement("lpm_table");
    lpmTableNode->addChild(XmlNode::createDataElement("version", friendlyValue((UIntN)m_version)));

    XmlNode* lpmEntriesNode = XmlNode::createWrapperElement("lpm_entries");
    for (auto entry = m_lpmEntries.begin(); entry != m_lpmEntries.end(); entry++)
    {
        XmlNode* lpmEntryNode = entry->getXml();
        lpmEntriesNode->addChild(lpmEntryNode);
    }
    lpmTableNode->addChild(lpmEntriesNode);

    return lpmTableNode;
}