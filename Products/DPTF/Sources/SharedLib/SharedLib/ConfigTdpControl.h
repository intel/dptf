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

    ConfigTdpControl(UInt64 controlId, UInt64 tarRatio, UInt64 tdpPower, UInt64 tdpFrequency);
    UInt64 getControlId(void) const;
    UInt64 getTdpRatio(void) const;
    UInt64 getTdpPower(void) const;
    UInt64 getTdpFrequency(void) const;
    Bool operator==(const ConfigTdpControl& rhs) const;
    Bool operator!=(const ConfigTdpControl& rhs) const;
    std::string getNameListString() const;
    XmlNode* getXml(void);

private:

    UInt64 m_controlId; // Unique identifier for this control
    UInt64 m_tdpRatio; // Unique control value used to calculate the TAR (Turbo Activation Ratio)
    UInt64 m_tdpPower; // Tdp Power consumed by this ConfigTDP level; unit: mW
    UInt64 m_tdpFrequency; // IA Tdp frequency for this ConfigTDP level; unit: MHZ
};