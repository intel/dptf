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

class Temperature final
{
public:

    Temperature(void); // Initialized to invalid by default
    Temperature(UInt32 temperature);
    static Temperature createInvalid();

    Bool operator==(const Temperature& rhs) const;
    Bool operator!=(const Temperature& rhs) const;
    Bool operator>(const Temperature& rhs) const;
    Bool operator>=(const Temperature& rhs) const;
    Bool operator<(const Temperature& rhs) const;
    Bool operator<=(const Temperature& rhs) const;
    friend std::ostream& operator<<(std::ostream& os, const Temperature& temperature);
    operator UInt32(void) const;

    Bool isValid() const;
    std::string toString() const;

private:

    static const UInt32 maxValidTemperature = 2500;
    Bool m_valid;
    UInt32 m_temperature;

    void throwIfInvalid(const Temperature& temperature) const;
};