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

#include "StatusFormat.h"
#include "Constants.h"
#include <sstream>
#include <iomanip>

using namespace std;

std::string StatusFormat::friendlyValue(Bool value)
{
    return (value == true) ? "true" : "false";
}

std::string StatusFormat::friendlyValue(UInt32 value)
{
    if (value == Constants::Invalid)
    {
        return Constants::InvalidString;
    }
    else
    {
        return std::to_string(value);
    }
}

std::string StatusFormat::friendlyValue(UInt64 value)
{
    if (value == Constants::Invalid)
    {
        return Constants::InvalidString;
    }
    else
    {
        return std::to_string(value);
    }
}

std::string StatusFormat::friendlyValue(double value)
{
    if (value == Constants::Invalid)
    {
        return Constants::InvalidString;
    }
    else
    {
        stringstream stream;
        stream << setprecision(2) << fixed << value;
        return stream.str();
    }
}