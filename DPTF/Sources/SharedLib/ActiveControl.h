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

class ActiveControl final
{
public:

    ActiveControl(UIntN controlId, UIntN tripPoint, UIntN speed, UIntN noiseLevel, UIntN power);
    UIntN getControlId(void) const;
    UIntN getTripPoint(void) const;
    UIntN getSpeed(void) const;
    UIntN getNoiseLevel(void) const;
    UIntN getPower(void) const;
    Bool operator==(const ActiveControl& rhs) const;
    Bool operator!=(const ActiveControl& rhs) const;
    XmlNode* getXml(void);

private:

    UIntN m_controlId;
    UIntN m_tripPoint;
    UIntN m_speed;
    UIntN m_noiseLevel;
    UIntN m_power;
};