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
#include "Dptf.h"
#include "BasicTypes.h"
#include "DptfExport.h"

class XmlNode;

//
// represents power in mW
//

class Power final
{
public:

    Power(void);
    Power(UInt32 power);
    UInt32 getPower() const;
    Bool isPowerValid() const;
    XmlNode* getXml(void);
    XmlNode* getXml(std::string tag);
    Bool operator>(const Power& rhs) const;
    Bool operator<(const Power& rhs) const;
    Bool operator==(const Power& rhs) const;
    Power operator-(const Power& rhs) const;
    Power operator+(const Power& rhs) const;

    std::string toString() const;

private:

    static const UInt32 maxValidPower = 10000000;                   // 10,000 watts
    static const UInt32 invalidPower = 0xFFFFFFFF;
    UInt32 m_power;
};