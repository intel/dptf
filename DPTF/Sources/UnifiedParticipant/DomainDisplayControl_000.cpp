/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include "DomainDisplayControl_000.h"

DomainDisplayControl_000::DomainDisplayControl_000(UIntN participantIndex, UIntN domainIndex, 
    ParticipantServicesInterface* participantServicesInterface)
    : DomainDisplayControlBase(participantIndex, domainIndex, participantServicesInterface)
{
    // Do nothing.  Not an error.
}

DisplayControlDynamicCaps DomainDisplayControl_000::getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

DisplayControlStatus DomainDisplayControl_000::getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

DisplayControlSet DomainDisplayControl_000::getDisplayControlSet(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

void DomainDisplayControl_000::setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex)
{
    throw not_implemented();
}

void DomainDisplayControl_000::setDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex, DisplayControlDynamicCaps newCapabilities)
{
    throw not_implemented();
}

void DomainDisplayControl_000::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

void DomainDisplayControl_000::clearCachedData(void)
{
    // Do nothing.  Not an error.
}

std::shared_ptr<XmlNode> DomainDisplayControl_000::getXml(UIntN domainIndex)
{
    throw not_implemented();
}

std::string DomainDisplayControl_000::getName(void)
{
    return "Display Control (Version 0)";
}