/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include "TableObjectType.h"
#include "DataManager.h"

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

UInt32 PolicyServicesPlatformConfigurationData::readConfigurationUInt32(
	const std::string& nameSpace,
	const std::string& key)
{
	throwIfNotWorkItemThread();
	UInt32 value = getEsifServices()->readConfigurationUInt32(nameSpace, key);
	return value;
}

void PolicyServicesPlatformConfigurationData::writeConfigurationUInt32(const std::string& key, UInt32 data)
{
	throwIfNotWorkItemThread();
	getEsifServices()->writeConfigurationUInt32(key, data);
}

std::string PolicyServicesPlatformConfigurationData::readConfigurationString(
	const std::string& nameSpace,
	const std::string& key)
{
	throwIfNotWorkItemThread();
	std::string value = getEsifServices()->readConfigurationString(nameSpace, key);
	return value;
}

DptfBuffer PolicyServicesPlatformConfigurationData::readConfigurationBinary(
	const std::string& nameSpace,
	const std::string& key)
{
	throwIfNotWorkItemThread();
	return getEsifServices()->readConfigurationBinary(nameSpace, key);
}

void PolicyServicesPlatformConfigurationData::writeConfigurationBinary(
	void* bufferPtr,
	UInt32 bufferLength,
	UInt32 dataLength,
	const std::string& nameSpace,
	const std::string& key)
{
	throwIfNotWorkItemThread();
	return getEsifServices()->writeConfigurationBinary(bufferPtr, bufferLength, dataLength, nameSpace, key);
}

void PolicyServicesPlatformConfigurationData::deleteConfigurationBinary(
	const std::string& nameSpace,
	const std::string& key)
{
	throwIfNotWorkItemThread();
	return getEsifServices()->deleteConfigurationBinary(nameSpace, key);
}

eEsifError PolicyServicesPlatformConfigurationData::sendCommand(UInt32 argc, const std::string& argv)
{
	throwIfNotWorkItemThread();
	return getEsifServices()->sendCommand(argc, argv);
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

DptfBuffer PolicyServicesPlatformConfigurationData::getAdaptivePerformanceConditionsTable(std::string uuid)
{
	throwIfNotWorkItemThread();

	return getDptfManager()->getDataManager()->getTableObject(TableObjectType::Type::Apct, uuid).getData();
}

DptfBuffer PolicyServicesPlatformConfigurationData::getAdaptivePerformanceActionsTable(std::string uuid)
{
	throwIfNotWorkItemThread();

	return getDptfManager()->getDataManager()->getTableObject(TableObjectType::Type::Apat, uuid).getData();
}

DptfBuffer PolicyServicesPlatformConfigurationData::getOemVariables(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_OEM_VARS, ESIF_DATA_BINARY);
}

UInt64 PolicyServicesPlatformConfigurationData::getHwpfState(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();

	const UInt64 hwpfState = getEsifServices()->primitiveExecuteGetAsUInt64(
		esif_primitive_type::GET_HWPF_STATE, participantIndex, domainIndex);

	return hwpfState;
}

// TODO: Move it to ConfigTDP domain control
UInt32 PolicyServicesPlatformConfigurationData::getProcessorConfigTdpControl(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_CONFIG_TDP_CONTROL, participantIndex, domainIndex);
}

// TODO: Move it to ConfigTDP domain control
Power PolicyServicesPlatformConfigurationData::getProcessorConfigTdpLevel(
	UIntN participantIndex,
	UIntN domainIndex,
	UIntN configTdpControl)
{
	throwIfNotWorkItemThread();

	switch (configTdpControl)
	{
	case 0:
		return getEsifServices()->primitiveExecuteGetAsPower(
			esif_primitive_type::GET_CONFIG_TDP_LEVEL0, participantIndex, domainIndex);
	case 1:
		return getEsifServices()->primitiveExecuteGetAsPower(
			esif_primitive_type::GET_CONFIG_TDP_LEVEL1, participantIndex, domainIndex);
	case 2:
		return getEsifServices()->primitiveExecuteGetAsPower(
			esif_primitive_type::GET_CONFIG_TDP_LEVEL2, participantIndex, domainIndex);
	default:
		throw dptf_exception("Config Tdp Control value not supported.");
	}
}

// TODO: Move it to ConfigTDP domain control
UInt32 PolicyServicesPlatformConfigurationData::getProcessorConfigTdpLock(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_CONFIG_TDP_LOCK, participantIndex, domainIndex);
}

Power PolicyServicesPlatformConfigurationData::getProcessorTdp(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGetAsPower(
		esif_primitive_type::GET_PROC_THERMAL_DESIGN_POWER, participantIndex, domainIndex);
}

Temperature PolicyServicesPlatformConfigurationData::getProcessorTjMax(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGetAsTemperatureTenthK(
		esif_primitive_type::GET_PROC_TJMAX, participantIndex, domainIndex);
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

	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_VOLTAGE_THRESHOLD_MATH_TABLE, ESIF_DATA_BINARY);
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

DptfBuffer PolicyServicesPlatformConfigurationData::getIntelligentThermalManagementTable()
{
	throwIfNotWorkItemThread();

	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Itmt, Constants::EmptyString)
		.getData();
}

void PolicyServicesPlatformConfigurationData::setIntelligentThermalManagementTable(DptfBuffer data)
{
	throwIfNotWorkItemThread();

	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Itmt);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getEnergyPerformanceOptimizerTable()
{
	throwIfNotWorkItemThread();

	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Epot, Constants::EmptyString)
		.getData();
}

void PolicyServicesPlatformConfigurationData::setEnergyPerformanceOptimizerTable(DptfBuffer data)
{
	throwIfNotWorkItemThread();

	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Type::Epot);
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

void PolicyServicesPlatformConfigurationData::resetIntelligentThermalManagementTable(void)
{
	try
	{
		getDptfManager()->getDataManager()->deleteTableObjectKeyForNoPersist(TableObjectType::Itmt);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetEnergyPerformanceOptimizerTable()
{
	try
	{
		getDptfManager()->getDataManager()->deleteTableObjectKeyForNoPersist(TableObjectType::Epot);
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
	resetIntelligentThermalManagementTable();
	resetEnergyPerformanceOptimizerTable();
}

Bool PolicyServicesPlatformConfigurationData::getDisplayRequired(void)
{
	throwIfNotWorkItemThread();
	UInt32 displayRequired = getEsifServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_ES_DISPLAY_REQUIRED,
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoInstance);
	if (displayRequired == 1)
	{
		return true;
	}

	return false;
}

void PolicyServicesPlatformConfigurationData::setPpmPackage(UInt32 value)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_ACTIVE_PPM_PACKAGE,
		value,
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoInstance);
}

void PolicyServicesPlatformConfigurationData::setPpmPackageSettings(PpmPackage::PpmParam param)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSet(
		SET_PPM_PACKAGE_PARAM, ESIF_DATA_STRUCTURE, &param, sizeof(param), sizeof(param));
}

void PolicyServicesPlatformConfigurationData::setPowerSchemeEpp(UInt32 value)
{
	throwIfNotWorkItemThread();

	try
	{
		getEsifServices()->primitiveExecuteSetAsUInt32(esif_primitive_type::SET_POWER_SCHEME_EPP, value);
	}
	catch (...)
	{
	}
}

void PolicyServicesPlatformConfigurationData::setActivePowerScheme()
{
	throwIfNotWorkItemThread();

	try
	{
		// Pass in bogus 0 value to prevent nullptr exception for SET primitive
		getEsifServices()->primitiveExecuteSetAsUInt32(esif_primitive_type::SET_ACTIVE_POWER_SCHEME, 0);
	}
	catch (...)
	{
	}
}

void PolicyServicesPlatformConfigurationData::clearPpmPackageSettings()
{
	throwIfNotWorkItemThread();

	try
	{
		// Pass in 0 to prevent nullptr exception for SET primitive
		getEsifServices()->primitiveExecuteSetAsUInt32(SET_PPM_PARAMS_TO_NULL, 0);
	}
	catch (...)
	{
	}
}

TimeSpan PolicyServicesPlatformConfigurationData::getExpectedBatteryLife()
{
	throwIfNotWorkItemThread();
	TimeSpan expectedBatteryLife =
		getEsifServices()->primitiveExecuteGetAsTimeInMilliseconds(esif_primitive_type::GET_EXPECTED_BATTERY_LIFE);

	return expectedBatteryLife;
}

UInt32 PolicyServicesPlatformConfigurationData::getAggressivenessLevel()
{
	throwIfNotWorkItemThread();

	const UInt32 aggressivenessLevel = getEsifServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_AGGRESSIVENESS_LEVEL,
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoInstance);

	return aggressivenessLevel;
}

void PolicyServicesPlatformConfigurationData::setForegroundAppRatioPeriod(UInt32 value)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_FOREGROUND_APP_RATIO_PERIOD,
		value,
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getDdrfTable(void)
{
	throwIfNotWorkItemThread();

	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Ddrf, Constants::EmptyString)
		.getData();
}

DptfBuffer PolicyServicesPlatformConfigurationData::getAggregateDisplayInformation(void)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_AGGREGATE_DISPLAY_INFORMATION, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getTpgaTable(void)
{
	throwIfNotWorkItemThread();
	
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Tpga, Constants::EmptyString)
		.getData();
}

UInt32 PolicyServicesPlatformConfigurationData::getDynamicBoostState(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();

	const UInt32 dynamicBoostState = getEsifServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_DYNAMIC_BOOST_STATE, participantIndex, domainIndex);

	return dynamicBoostState;
}

void PolicyServicesPlatformConfigurationData::setDynamicBoostState(
	UIntN participantIndex,
	UIntN domainIndex,
	UInt32 value)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_DYNAMIC_BOOST_STATE, value, participantIndex, domainIndex);
}

UInt32 PolicyServicesPlatformConfigurationData::getTpgPowerState(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();

	return getEsifServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_TPG_POWER_STATE, participantIndex, domainIndex);
}
