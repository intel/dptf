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

#include "Indent.h"


Indent::Indent()
    : m_length(0), m_stepSize(2)
{
}

Indent::Indent(UIntN length, UIntN stepSize)
    : m_length(length), m_stepSize(stepSize)
{
}

std::ostream& operator<<(std::ostream& out, const Indent& indent)
{
    out << indent.emit();
    return out;
}

Indent& Indent::operator++() // prefix
{
    m_length += m_stepSize;
    return *this;
};

Indent Indent::operator++(int) // postfix
{
    Indent copyOfThis(*this);
    ++(*this);
    return copyOfThis;
};

Indent& Indent::operator--() // prefix
{
    if (m_length >= m_stepSize)
    {
        m_length -= m_stepSize;
    }
    else
    {
        m_length = 0;
    }
    return *this;
};

Indent Indent::operator--(int) // postfix
{
    Indent copyOfThis(*this);
    --(*this);
    return copyOfThis;
};

UIntN Indent::length() const
{
    return m_length;
}

std::string Indent::emit() const
{
    return std::string(m_length, ' ');
}