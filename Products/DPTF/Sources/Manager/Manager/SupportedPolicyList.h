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
#include "Guid.h"

class SupportedPolicyList
{
public:

    SupportedPolicyList(void);

    UIntN getCount(void) const;
    const Guid& operator[](UIntN index) const;
    Bool isPolicyValid(const Guid& guid) const;

private:

    std::vector<Guid> m_guid;

    // Uses GET_SUPPORTED_POLICIES primitive to build a list of policy guids that should be loaded.
    void createSupportedPolicyList(void);
};