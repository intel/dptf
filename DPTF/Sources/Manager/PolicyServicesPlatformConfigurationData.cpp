/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "PolicyServicesPlatformConfigurationData.h"
#include "EsifServicesInterface.h"
#include "esif_sdk_data_misc.h"

PolicyServicesPlatformConfigurationData::PolicyServicesPlatformConfigurationData(
	DptfManagerInterface* dptfManager,
	UIntN policyIndex)
	: PolicyServices(dptfManager, policyIndex)
	, m_defaultSamplePeriod(TimeSpan::createInvalid())
{
	resetAllTables();
}

UInt32 PolicyServicesPlatformConfigurationData::readConfigurationUInt32(const std::string& key)
{
	throwIfNotWorkItemThread();
	UInt32 value = getEsifServices()->readConfigurationUInt32(key);
	return value;
}

void PolicyServicesPlatformConfigurationData::writeConfigurationUInt32(const std::string& key, UInt32 data)
{
	throwIfNotWorkItemThread();
	getEsifServices()->writeConfigurationUInt32(key, data);
}

std::string PolicyServicesPlatformConfigurationData::readConfigurationString(const std::string& key)
{
	throwIfNotWorkItemThread();
	std::string value = getEsifServices()->readConfigurationString(key);
	return value;
}

DptfBuffer PolicyServicesPlatformConfigurationData::readConfigurationBinary(const std::string& key)
{
	throwIfNotWorkItemThread();
	return getEsifServices()->readConfigurationBinary(key);
}

TimeSpan PolicyServicesPlatformConfigurationData::getMinimumAllowableSamplePeriod(void)
{
	throwIfNotWorkItemThread();

	if (m_defaultSamplePeriod.isInvalid())
	{
		m_defaultSamplePeriod =
			getEsifServices()->primitiveExecuteGetAsTimeInMilliseconds(esif_primitive_type::GET_MINIMUM_SAMPLE_PERIOD);
	}

	return m_defaultSamplePeriod;
}

DptfBuffer PolicyServicesPlatformConfigurationData::getActiveRelationshipTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_ACTIVE_RELATIONSHIP_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setActiveRelationshipTable(DptfBuffer data)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSet(
		esif_primitive_type::SET_ACTIVE_RELATIONSHIP_TABLE,
		esif_data_type::ESIF_DATA_BINARY,
		data.get(),
		data.size(),
		data.size(),
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoPersistInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getThermalRelationshipTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_THERMAL_RELATIONSHIP_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setThermalRelationshipTable(DptfBuffer data)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSet(
		esif_primitive_type::SET_THERMAL_RELATIONSHIP_TABLE,
		esif_data_type::ESIF_DATA_BINARY,
		data.get(),
		data.size(),
		data.size(),
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoPersistInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPassiveTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_PASSIVE_RELATIONSHIP_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setPassiveTable(DptfBuffer data)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSet(
		esif_primitive_type::SET_PASSIVE_RELATIONSHIP_TABLE,
		esif_data_type::ESIF_DATA_BINARY,
		data.get(),
		data.size(),
		data.size(),
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoPersistInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getAdaptivePerformanceConditionsTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_ADAPTIVE_PERFORMANCE_CONDITIONS_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getAdaptivePerformanceParticipantConditionTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_ADAPTIVE_PERFORMANCE_PARTICIPANT_CONDITION_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getAdaptivePerformanceActionsTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_ADAPTIVE_PERFORMANCE_ACTIONS_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getOemVariables(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_OEM_VARS, ESIF_DATA_BINARY);
}

UInt64 PolicyServicesPlatformConfigurationData::getHwpfState(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();

	UInt64 hwpfState = getEsifServices()->primitiveExecuteGetAsUInt64(
		esif_primitive_type::GET_HWPF_STATE, participantIndex, domainIndex);

	return hwpfState;
}

std::string PolicyServicesPlatformConfigurationData::readPlatformSettingValue(
	PlatformSettingType::Type platformSettingType,
	UInt8 index)
{
	throwIfNotWorkItemThread();

	switch (platformSettingType)
	{
	case PlatformSettingType::ConfigTdp:
	{
		return getEsifServices()->primitiveExecuteGetAsString(
			esif_primitive_type::GET_SYSTEM_CONFIGTDP_LEVEL_NAME,
			Constants::Esif::NoParticipant,
			Constants::Esif::NoDomain,
			index);
		break;
	}
	default:
	{
		throw dptf_exception("Invalid platform setting type referenced in call to read platform setting value.");
	}
	}
}

void PolicyServicesPlatformConfigurationData::writePlatformSettingValue(
	PlatformSettingType::Type platformSettingType,
	UInt8 index,
	const std::string& stringValue)
{
	throwIfNotWorkItemThread();

	switch (platformSettingType)
	{
	case PlatformSettingType::ConfigTdp:
	{
		getEsifServices()->primitiveExecuteSetAsString(
			esif_primitive_type::SET_SYSTEM_CONFIGTDP_LEVEL_NAME,
			stringValue,
			Constants::Esif::NoParticipant,
			Constants::Esif::NoDomain,
			index);
		break;
	}
	default:
	{
		throw dptf_exception("Invalid platform setting type referenced in call to write platform setting value.");
	}
	}
}

const Guid DptfDppeGroup(
	0x48,
	0xdf,
	0x9d,
	0x60,
	0x4f,
	0x68,
	0x11,
	0xdc,
	0x83,
	0x14,
	0x08,
	0x00,
	0x20,
	0x0c,
	0x9a,
	0x66);

void PolicyServicesPlatformConfigurationData::clearPlatformSettings(PlatformSettingType::Type platformSettingType)
{
	throwIfNotWorkItemThread();

	switch (platformSettingType)
	{
	case PlatformSettingType::ConfigTdp:
	{
		getEsifServices()->primitiveExecuteSet(
			esif_primitive_type::SET_SYSTEM_CONFIGTDP_CLEAR_LEVELS, esif_data_type::ESIF_DATA_VOID, nullptr, 0, 0);
		break;
	}
	default:
	{
		throw dptf_exception("Invalid platform setting type referenced in call to clear platform settings.");
	}
	}
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerBossConditionsTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_POWER_BOSS_CONDITIONS_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerBossActionsTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_POWER_BOSS_ACTIONS_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getEmergencyCallModeTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_EMERGENCY_CALL_MODE_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPidAlgorithmTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_PID_ALGORITHM_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setPidAlgorithmTable(DptfBuffer data)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSet(
		esif_primitive_type::SET_PID_ALGORITHM_TABLE,
		esif_data_type::ESIF_DATA_BINARY,
		data.get(),
		data.size(),
		data.size(),
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoPersistInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerBossMathTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_POWER_BOSS_MATH_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getVoltageThresholdMathTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_VOLTAGE_THRESHOLD_MATH_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getActiveControlPointRelationshipTable(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_ACTIVE_CONTROL_POINT_RELATIONSHIP_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setActiveControlPointRelationshipTable(DptfBuffer data)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSet(
		esif_primitive_type::SET_ACTIVE_CONTROL_POINT_RELATIONSHIP_TABLE,
		esif_data_type::ESIF_DATA_BINARY,
		data.get(),
		data.size(),
		data.size(),
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoPersistInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerShareAlgorithmTable()
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_POWER_SHARING_ALGORITHM_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setPowerShareAlgorithmTable(DptfBuffer data)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSet(
		esif_primitive_type::SET_POWER_SHARING_ALGORITHM_TABLE,
		esif_data_type::ESIF_DATA_BINARY,
		data.get(),
		data.size(),
		data.size(),
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoPersistInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerShareAlgorithmTable2()
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_POWER_SHARING_ALGORITHM_TABLE_2, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setPowerShareAlgorithmTable2(DptfBuffer data)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSet(
		esif_primitive_type::SET_POWER_SHARING_ALGORITHM_TABLE_2,
		esif_data_type::ESIF_DATA_BINARY,
		data.get(),
		data.size(),
		data.size(),
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoPersistInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::createResetPrimitiveTupleBinary(
	esif_primitive_type primitive,
	UInt8 instance) const
{
	esif_primitive_tuple_parameter tuple;
	tuple.id.integer.type = esif_data_type::ESIF_DATA_UINT16;
	tuple.id.integer.value = primitive;
	tuple.domain.integer.type = esif_data_type::ESIF_DATA_UINT16;
	UInt16 domainIndex = createTupleDomain();
	tuple.domain.integer.value = domainIndex;
	tuple.instance.integer.type = esif_data_type::ESIF_DATA_UINT16;
	tuple.instance.integer.value = instance;

	UInt32 sizeOfTuple = (UInt32)sizeof(tuple);
	DptfBuffer buffer(sizeOfTuple);
	buffer.put(0, (UInt8*)&tuple, sizeOfTuple);
	return buffer;
}

UInt16 PolicyServicesPlatformConfigurationData::createTupleDomain() const
{
	UInt16 tupleDomain;
	tupleDomain = (('0' + (UInt8)0) << 8) + 'D';
	return tupleDomain;
}

void PolicyServicesPlatformConfigurationData::resetActiveRelationshipTable(void)
{
	try
	{
		DptfBuffer tableBuffer = createResetPrimitiveTupleBinary(
			esif_primitive_type::SET_ACTIVE_RELATIONSHIP_TABLE, Constants::Esif::NoPersistInstance);

		getEsifServices()->primitiveExecuteSet(
			esif_primitive_type::SET_CONFIG_RESET,
			ESIF_DATA_BINARY,
			tableBuffer.get(),
			tableBuffer.size(),
			tableBuffer.size(),
			Constants::Esif::NoParticipant,
			Constants::Esif::NoDomain,
			Constants::Esif::NoInstance);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetThermalRelationshipTable(void)
{
	try
	{
		DptfBuffer tableBuffer = createResetPrimitiveTupleBinary(
			esif_primitive_type::SET_THERMAL_RELATIONSHIP_TABLE, Constants::Esif::NoPersistInstance);

		getEsifServices()->primitiveExecuteSet(
			esif_primitive_type::SET_CONFIG_RESET,
			ESIF_DATA_BINARY,
			tableBuffer.get(),
			tableBuffer.size(),
			tableBuffer.size(),
			Constants::Esif::NoParticipant,
			Constants::Esif::NoDomain,
			Constants::Esif::NoInstance);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetPassiveTable(void)
{
	try
	{
		DptfBuffer tableBuffer = createResetPrimitiveTupleBinary(
			esif_primitive_type::SET_PASSIVE_RELATIONSHIP_TABLE, Constants::Esif::NoPersistInstance);

		getEsifServices()->primitiveExecuteSet(
			esif_primitive_type::SET_CONFIG_RESET,
			ESIF_DATA_BINARY,
			tableBuffer.get(),
			tableBuffer.size(),
			tableBuffer.size(),
			Constants::Esif::NoParticipant,
			Constants::Esif::NoDomain,
			Constants::Esif::NoInstance);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetPidAlgorithmTable(void)
{
	try
	{
		DptfBuffer tableBuffer = createResetPrimitiveTupleBinary(
			esif_primitive_type::SET_PID_ALGORITHM_TABLE, Constants::Esif::NoPersistInstance);

		getEsifServices()->primitiveExecuteSet(
			esif_primitive_type::SET_CONFIG_RESET,
			ESIF_DATA_BINARY,
			tableBuffer.get(),
			tableBuffer.size(),
			tableBuffer.size(),
			Constants::Esif::NoParticipant,
			Constants::Esif::NoDomain,
			Constants::Esif::NoInstance);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetActiveControlPointRelationshipTable(void)
{
	try
	{
		DptfBuffer tableBuffer = createResetPrimitiveTupleBinary(
			esif_primitive_type::SET_ACTIVE_CONTROL_POINT_RELATIONSHIP_TABLE, Constants::Esif::NoPersistInstance);

		getEsifServices()->primitiveExecuteSet(
			esif_primitive_type::SET_CONFIG_RESET,
			ESIF_DATA_BINARY,
			tableBuffer.get(),
			tableBuffer.size(),
			tableBuffer.size(),
			Constants::Esif::NoParticipant,
			Constants::Esif::NoDomain,
			Constants::Esif::NoInstance);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetPowerShareAlgorithmTable(void)
{
	try
	{
		DptfBuffer tableBuffer = createResetPrimitiveTupleBinary(
			esif_primitive_type::SET_POWER_SHARING_ALGORITHM_TABLE, Constants::Esif::NoPersistInstance);

		getEsifServices()->primitiveExecuteSet(
			esif_primitive_type::SET_CONFIG_RESET,
			ESIF_DATA_BINARY,
			tableBuffer.get(),
			tableBuffer.size(),
			tableBuffer.size(),
			Constants::Esif::NoParticipant,
			Constants::Esif::NoDomain,
			Constants::Esif::NoInstance);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetPowerShareAlgorithmTable2(void)
{
	try
	{
		DptfBuffer tableBuffer = createResetPrimitiveTupleBinary(
			esif_primitive_type::SET_POWER_SHARING_ALGORITHM_TABLE_2, Constants::Esif::NoPersistInstance);

		getEsifServices()->primitiveExecuteSet(
			esif_primitive_type::SET_CONFIG_RESET,
			ESIF_DATA_BINARY,
			tableBuffer.get(),
			tableBuffer.size(),
			tableBuffer.size(),
			Constants::Esif::NoParticipant,
			Constants::Esif::NoDomain,
			Constants::Esif::NoInstance);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetAllTables(void)
{
	resetActiveRelationshipTable();
	resetThermalRelationshipTable();
	resetPassiveTable();
	resetPidAlgorithmTable();
	resetActiveControlPointRelationshipTable();
	resetPowerShareAlgorithmTable();
	resetPowerShareAlgorithmTable2();
}
