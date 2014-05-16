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

//FIXME:  why do we return the control id and set based on control id instead of using index?

#include "Dptf.h"

class XmlNode;

class ActiveControlStatus final
{
public:

    ActiveControlStatus(UIntN currentControlId, UIntN currentSpeed);
    UIntN getCurrentControlId(void) const;
    UIntN getCurrentSpeed(void) const;
    Bool operator==(const ActiveControlStatus rhs) const;
    Bool operator!=(const ActiveControlStatus rhs) const;
    XmlNode* getXml(void);

private:

    UIntN m_currentControlId;
    UIntN m_currentSpeed;                                           // Current real time speed of the fan - not necessarily the expected speed
};