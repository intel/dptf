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

class PerformanceControlStaticCaps final
{
public:

    PerformanceControlStaticCaps(Bool dynamicPerformanceControlStates);
    Bool supportsDynamicPerformanceControlStates(void) const;
    XmlNode* getXml(void);

private:

    // PD:  Talked w/ Vasu 5/1/2013 to see if this can be removed as it hasn't been used in DPTF 6.0
    //      or DPTF 7.0.  He said it might be used in DPTF 8.0 so we should keep it.
    Bool m_dynamicPerformanceControlStates;                         // if TRUE, the PerformanceControlSet can change dynamically.
};