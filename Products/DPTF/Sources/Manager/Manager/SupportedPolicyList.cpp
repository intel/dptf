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

#include "SupportedPolicyList.h"

SupportedPolicyList::SupportedPolicyList(void)
{
    createSupportedPolicyList();
}

UIntN SupportedPolicyList::getCount(void) const
{
    return static_cast<UIntN>(m_guid.size());
}

const Guid& SupportedPolicyList::operator[](UIntN index) const
{
    return m_guid.at(index);
}

Bool SupportedPolicyList::isPolicyValid(const Guid& guid) const
{
    Bool supported = false;

    for (auto it = m_guid.begin(); it != m_guid.end(); it++)
    {
        if (*it == guid)
        {
            supported = true;
            break;
        }
    }

    return supported;
}

void SupportedPolicyList::createSupportedPolicyList(void)
{
    // FIXME
    // throw implement_me();
}