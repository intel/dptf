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

class XmlNode;

class DomainPriority final
{
public:

    DomainPriority();                                               // defaults priority to 0 (meaning low priority)
    DomainPriority(UIntN currentPriority);
    UIntN getCurrentPriority(void) const;
    Bool operator==(const DomainPriority& rhs) const;
    Bool operator!=(const DomainPriority& rhs) const;
    Bool operator>(const DomainPriority& rhs) const;
    Bool operator<(const DomainPriority& rhs) const;
    XmlNode* getXml(void);

private:

    // Stores the priority for the domain.  For a given participant with multiple domains, a higher
    // number is a higher priority.  If two domains have the same priority, the behavior is undefined
    // and the policy will act as it chooses.  As an example of its usage, the passive policy uses
    // information to choose whether it limits the CPU first or Graphics first when a thermal
    // condition occurs.
    UIntN m_currentPriority;
};