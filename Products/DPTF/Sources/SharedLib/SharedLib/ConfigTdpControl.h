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

class ConfigTdpControl final
{
public:

    ConfigTdpControl(UIntN controlId, UIntN tarRatio, UIntN tdpPower, UIntN tdpFrequency);
    UIntN getControlId(void) const;
    UIntN getTdpRatio(void) const;
    UIntN getTdpPower(void) const;
    UIntN getTdpFrequency(void) const;
    Bool operator==(const ConfigTdpControl& rhs) const;
    Bool operator!=(const ConfigTdpControl& rhs) const;
    XmlNode* getXml(void);

private:

    UIntN m_controlId;                                              // Unique identifier for this control
    UIntN m_tdpRatio;                                               // Unique control value used to calculate the TAR (Turbo Activation Ratio)
    UIntN m_tdpPower;                                               // Tdp Power consumed by this ConfigTDP level; unit: mW
    UIntN m_tdpFrequency;                                           // IA Tdp frequency for this ConfigTDP level; unit: MHZ
};