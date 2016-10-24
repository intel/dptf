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

#include "Dptf.h"

// TODO: consider extending std::string and putting this function there.
class StringConverter final
{
public:

    static UInt32 toUInt32(const std::string& input);
    static UInt64 toUInt64(const std::string& input);
    static Int32 toInt32(const std::string& input);
    static double toDouble(const std::string& input);
    static std::string toUpper(const std::string& input);
    static std::string toLower(const std::string& input);
    static std::string trimWhitespace(const std::string& input);
};