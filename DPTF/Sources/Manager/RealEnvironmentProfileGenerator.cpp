/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "RealEnvironmentProfileGenerator.h"
#include "esif_ccb_cpuid.h"
#include <string>

using namespace std;

RealEnvironmentProfileGenerator::RealEnvironmentProfileGenerator(
	const std::shared_ptr<MessageLogger>& messageLogger,
	EsifServicesInterface* esifServices)
	: m_messageLogger(messageLogger)
	, m_esifServices(esifServices)
{
}

EnvironmentProfile RealEnvironmentProfileGenerator::generateWithCpuIdOnly() const
{
	const auto cpuId = getCpuId();
	return {cpuId, Power::createInvalid()};
}

EnvironmentProfile RealEnvironmentProfileGenerator::generateWithFullProfile(UInt32 socParticipant, UInt32 socDomain) const
{
	const auto cpuId = getCpuId();
	const auto socBasePower = getSocBasePower(socParticipant, socDomain);
	return {cpuId, socBasePower};
}

UInt64 RealEnvironmentProfileGenerator::getCpuId()
{
	esif_ccb_cpuid_t cpuId = {0, 0, 0, 0, 0};
	cpuId.leaf = ESIF_CPUID_LEAF_PROCESSOR_SIGNATURE;
	esif_ccb_cpuid(&cpuId);
	return cpuId.eax & CPUID_FAMILY_MODEL_MASK;
}

Power RealEnvironmentProfileGenerator::getSocBasePower(UInt32 socParticipant, UInt32 socDomain) const
{
	const Power configTdpLevel = getConfigTdpLevel(socParticipant, socDomain);
	if (configTdpLevel.isValid())
	{
		return configTdpLevel;
	}

	const Power processorTdpLevel = getProcessorTdpLevel(socParticipant, socDomain);
	if (processorTdpLevel.isValid())
	{
		return processorTdpLevel;
	}

	return Power::createInvalid();
}

Power RealEnvironmentProfileGenerator::getConfigTdpLevel(UInt32 socParticipant, UInt32 socDomain) const
{
	try
	{
		const auto currentConfigTdpControl = m_esifServices->primitiveExecuteGetAsUInt32(GET_CONFIG_TDP_CONTROL, socParticipant, socDomain);
		throwIfInvalidConfigTdpLevel(currentConfigTdpControl);
		const auto configTdpPrimitive =
			static_cast<esif_primitive_type>(GET_CONFIG_TDP_LEVEL0 + currentConfigTdpControl);
		const auto configTdpLevel = m_esifServices->primitiveExecuteGetAsPower(configTdpPrimitive, socParticipant, socDomain);
		return configTdpLevel;
	}
	catch (const exception& ex)
	{
		LOG_MESSAGE_FRAMEWORK_WARNING(
			m_messageLogger, { return "Failed to get processor Config TDP Level value: "s + string(ex.what()); })
		return Power::createInvalid();
	}
}

Power RealEnvironmentProfileGenerator::getProcessorTdpLevel(UInt32 socParticipant, UInt32 socDomain) const
{
	try
	{
		const auto processorTdpLevel = m_esifServices->primitiveExecuteGetAsPower(GET_PROC_THERMAL_DESIGN_POWER, socParticipant, socDomain);
		return processorTdpLevel;
	}
	catch (const exception& ex)
	{
		LOG_MESSAGE_FRAMEWORK_WARNING(
			m_messageLogger, { return "Failed to get processor TDP value: "s + string(ex.what()); })
		return Power::createInvalid();
	}
}

void RealEnvironmentProfileGenerator::throwIfInvalidConfigTdpLevel(UInt32 configTdpLevel)
{
	if (configTdpLevel > GET_CONFIG_TDP_LEVEL2 - GET_CONFIG_TDP_LEVEL0)
	{
		throw dptf_exception("Invalid Config TDP level"s);
	}
}