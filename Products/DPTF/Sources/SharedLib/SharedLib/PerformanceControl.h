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
#include <string>

class XmlNode;

namespace PerformanceControlType
{
    enum Type
    {
        Unknown,
        PerformanceState,
        ThrottleState
    };
}

class PerformanceControl final
{
public:

    PerformanceControl(UIntN controlId, PerformanceControlType::Type performanceControlType, UIntN tdpPower,
        Percentage performancePercentage, UIntN transitionLatency, UIntN controlAbsoluteValue,
        std::string valueUnits);

    UIntN getControlId(void) const;
    PerformanceControlType::Type getPerformanceControlType(void) const;
    UIntN getTdpPower(void) const;
    Percentage getPerformancePercentage(void) const;
    UIntN getTransitionLatency(void) const;
    UIntN getControlAbsoluteValue(void) const;
    std::string getValueUnits(void) const;
    Bool operator==(const PerformanceControl& rhs) const;
    Bool operator!=(const PerformanceControl& rhs) const;
    XmlNode* getXml(void);
    std::string PerformanceControlTypeToString(PerformanceControlType::Type type);

private:

    UIntN m_controlId;
    PerformanceControlType::Type m_performanceControlType;
    UIntN m_tdpPower;                                               // TDP Power consumed by this P/T State
    Percentage m_performancePercentage;
    UIntN m_transitionLatency;                                      // How long does it take to realize the new performance limit
    UIntN m_controlAbsoluteValue;                                   // if CPU or graphics, controlAbsoluteValue contains the frequency and
    std::string m_valueUnits;                                       // valueUnits is set to "MHz"
};