/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

#include "ActiveControl.h"
#include "BasicTypes.h"
#include <vector>

class XmlNode;

class ActiveControlSet final
{
public:

    ActiveControlSet(std::vector<ActiveControl> activeControl);
    UIntN getCount(void) const;
    const ActiveControl& operator[](UIntN index) const;
    Bool operator==(const ActiveControlSet& rhs) const;
    XmlNode* getXml(void);

private:

    std::vector<ActiveControl> m_activeControl;
};