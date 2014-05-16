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
#include "CoreControlOffliningMode.h"

class XmlNode;

class CoreControlLpoPreference final
{
public:

    CoreControlLpoPreference(Bool lpoEnabled, UIntN lpoStartPState, Percentage lpoStepSize,
        CoreControlOffliningMode::Type lpoPowerControlOffliningMode,
        CoreControlOffliningMode::Type lpoPerformanceControlOffliningMode);
    Bool isLpoEnabled(void) const;
    UIntN getStartPState(void) const;
    Percentage getStepSize(void) const;
    CoreControlOffliningMode::Type getPowerControlOffliningMode(void) const;
    CoreControlOffliningMode::Type getPerformanceControlOffliningMode(void) const;
    Bool operator==(const CoreControlLpoPreference& rhs) const;
    Bool operator!=(const CoreControlLpoPreference& rhs) const;
    XmlNode* getXml(void);

private:

    Bool m_lpoEnabled;                                                  // If true LPO is available to the policy
    UIntN m_startPState;                                                // Indicates when to start LPO control. Value is P state index
    Percentage m_stepSize;                                              // Instructs policy to take away logical processors in specified percentage steps.
                                                                        // e.g.: 25% on a dual core system is 1 logical processor
    CoreControlOffliningMode::Type m_powerControlOffliningMode;         // SMT or Core Offlining mode for power control
    CoreControlOffliningMode::Type m_performanceControlOffliningMode;   // SMT or Core Offlining mode for performance control
};