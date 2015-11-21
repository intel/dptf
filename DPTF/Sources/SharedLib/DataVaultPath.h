/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

namespace DataVaultPathBasePaths
{
    static const std::string SharedRoot = "/shared";
    static const std::string TablesRoot = SharedRoot + "/tables";
    static const std::string ValuesRoot = SharedRoot + "/values";
    static const std::string ExportRoot = SharedRoot + "/export";
}

namespace DataVaultPath
{
    namespace Shared
    {
        namespace Export
        {
            static const std::string WorkloadHints = DataVaultPathBasePaths::ExportRoot + "/workload_hints/";
        };

        namespace Tables
        {
            static const std::string Psvt = DataVaultPathBasePaths::TablesRoot + "/psvt/";
        };
    };
};