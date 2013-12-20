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
#include "ControlKnobType.h"
#include "DomainType.h"
#include "LpmMode.h"

namespace StatusFormat
{
    std::string friendlyValue(UIntN value);
    std::string friendlyValue(Bool value);
    std::string friendlyValue(UInt64 value);
    std::string friendlyValue(float value);
    std::string friendlyValue(ControlKnobType::Type value);
    std::string friendlyValue(DomainType::Type value);
    std::string friendlyValue(LpmMode::Type type);
    std::string friendlyValue(LpmMode::Boss boss);
};