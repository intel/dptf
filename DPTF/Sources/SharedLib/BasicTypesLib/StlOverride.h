/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>

// These functions override STL functions that are not supported on all STL implementations.
// It is expected that over time these can be removed as STL implementations for platforms that we support
// improve.
namespace StlOverride
{
    // Replaces std::to_string
    template <typename T>
    std::string to_string(const T& val)
    {
        std::ostringstream oss;
        oss << val;
        return oss.str();
    }

    // Replaces std::stoul
    inline unsigned long stoul(const std::string& val)
    {
        return strtoul(val.c_str(), NULL, 0);
    }
}