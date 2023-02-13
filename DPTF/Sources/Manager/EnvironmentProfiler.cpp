/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "EnvironmentProfiler.h"
#include "esif_ccb_cpuid.h"
#include <string>

using namespace std;

EnvironmentProfiler::EnvironmentProfiler()
{
}

EnvironmentProfile EnvironmentProfiler::generate() const
{
	const auto cpuId = getCpuId();
	return {cpuId};
}


UInt64 EnvironmentProfiler::getCpuId(void)
{
	esif_ccb_cpuid_t esifCpuId = {0, 0, 0, 0, 0};
	esifCpuId.leaf = ESIF_CPUID_LEAF_PROCESSOR_SIGNATURE;
	esif_ccb_cpuid(&esifCpuId);
	return esifCpuId.eax & CPUID_FAMILY_MODEL_MASK;
}
