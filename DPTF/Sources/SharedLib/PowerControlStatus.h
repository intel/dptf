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
#include "PowerControlType.h"

class XmlNode;

class PowerControlStatus final
{
public:

    PowerControlStatus(PowerControlType::Type powerControlType, Power currentPowerLimit, UIntN currentTimeWindow,
        Percentage currentDutyCycle);

    PowerControlType::Type getPowerControlType(void) const;
    Power getCurrentPowerLimit(void) const;
    UIntN getCurrentTimeWindow(void) const;
    Percentage getCurrentDutyCycle(void) const;
    XmlNode* getXml(void);
    Bool operator==(const PowerControlStatus& rhs) const;
    Bool operator!=(const PowerControlStatus& rhs) const;

private:

    friend class PowerControlArbitrator;

    PowerControlType::Type m_powerControlType;
    Power m_currentPowerLimit;                                      // mW
    UIntN m_currentTimeWindow;                                      // ms
    Percentage m_currentDutyCycle;                                  // %
};