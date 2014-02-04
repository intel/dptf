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

#include "LpmConfigurationHelper.h"


LpmConfigurationHelper::LpmConfigurationHelper()
{

}

LpmConfigurationHelper::~LpmConfigurationHelper()
{

}

vector<UIntN> LpmConfigurationHelper::findLpmEntriesWithAcpiScope(
    std::string acpiScope,
    const vector<LpmEntry>& lpmEntries 
    ) const
{
    std::vector<UIntN> rows;
    for (UIntN row = 0; row < lpmEntries.size(); ++row)
    {
        if (lpmEntries[row].targetDeviceAcpiScope() == acpiScope)
        {
            rows.push_back(row);
        }
    }
    return rows;
}

vector<UIntN> LpmConfigurationHelper::findLpmEntriesWithParticipantIndex(
    UIntN participantIndex,
    const vector<LpmEntry>& lpmEntries
    ) const
{
    std::vector<UIntN> rows;
    for (UIntN row = 0; row < lpmEntries.size(); ++row)
    {
        if (lpmEntries[row].targetDeviceIndex() == participantIndex)
        {
            rows.push_back(row);
        }
    }
    return rows;
}

vector<UIntN> LpmConfigurationHelper::findLpmEntriesWithDomainType(
    DomainType::Type domainType,
    const vector<LpmEntry>& lpmEntries
    ) const
{
    std::vector<UIntN> rows;
    for (UIntN row = 0; row < lpmEntries.size(); ++row)
    {
        if (lpmEntries[row].domainType() == domainType)
        {
            rows.push_back(row);
        }
    }
    return rows;
}

void LpmConfigurationHelper::setPolicyServices(PolicyServicesInterfaceContainer policyServices)
{
    m_policyServices = policyServices;
}

PolicyServicesInterfaceContainer& LpmConfigurationHelper::getPolicyServices(void)
{
    return m_policyServices;
}
