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

#include "DomainConfigTdpControl_000.h"

DomainConfigTdpControl_000::DomainConfigTdpControl_000(ParticipantServicesInterface* participantServicesInterface)
{
    // Do nothing.  Not an error.
}

ConfigTdpControlDynamicCaps DomainConfigTdpControl_000::getConfigTdpControlDynamicCaps(UIntN participantIndex,
    UIntN domainIndex)
{
    throw not_implemented();
}

ConfigTdpControlStatus DomainConfigTdpControl_000::getConfigTdpControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

ConfigTdpControlSet DomainConfigTdpControl_000::getConfigTdpControlSet(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

void DomainConfigTdpControl_000::setConfigTdpControl(UIntN participantIndex, UIntN domainIndex,
    UIntN configTdpControlIndex)
{
    throw not_implemented();
}

void DomainConfigTdpControl_000::clearCachedData(void)
{
    // Do nothing.  Not an error.
}

XmlNode* DomainConfigTdpControl_000::getXml(UIntN domainIndex)
{
    throw not_implemented();
}