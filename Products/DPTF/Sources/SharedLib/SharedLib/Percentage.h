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
#include "Constants.h"

class XmlNode;

class Percentage final
{
public:

    Percentage(void);
    Percentage(UIntN percentage);
    UIntN getPercentage() const;
    Bool isPercentageValid() const;
    XmlNode* getXml(void);
    XmlNode* getXml(std::string tag);
    Bool operator==(const Percentage& rhs) const;
    Bool operator!=(const Percentage& rhs) const;
    Bool operator>(const Percentage& rhs) const;
    Bool operator>=(const Percentage& rhs) const;
    Bool operator<(const Percentage& rhs) const;
    Bool operator<=(const Percentage& rhs) const;

    std::string toString() const;

    static const UInt32 invalidPercentage = Constants::Invalid;

private:

    UIntN m_percentage;

    void throwIfInvalidPercentage(void) const;
};