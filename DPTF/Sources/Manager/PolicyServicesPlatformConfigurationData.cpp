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

#include "PolicyServicesPlatformConfigurationData.h"
#include "EsifServicesInterface.h"
#include "esif_sdk_data_misc.h"
#include "TableObjectType.h"
#include "DataManager.h"
#include "EnvironmentProfile.h"
#include "DataVaultPath.h"

using namespace std;

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
	return getEsifServices()->readConfigurationUInt32(key);
}

UInt32 PolicyServicesPlatformConfigurationData::readConfigurationUInt32(
	const std::string& nameSpace,
	const std::string& key)
{
	throwIfNotWorkItemThread();
	return getEsifServices()->readConfigurationUInt32(nameSpace, key);
}

void PolicyServicesPlatformConfigurationData::writeConfigurationString(
	const std::string& key,
	const std::string& data)
{
	throwIfNotWorkItemThread();
	getEsifServices()->writeConfigurationString(key, data);
}

void PolicyServicesPlatformConfigurationData::writeConfigurationString(
	const std::string& nameSpace,
	const std::string& key,
	const std::string& data)
{
	throwIfNotWorkItemThread();
	getEsifServices()->writeConfigurationString(nameSpace, key, data);
}

void PolicyServicesPlatformConfigurationData::writeConfigurationUInt32(const std::string& key, UInt32 data)
{
	throwIfNotWorkItemThread();
	getEsifServices()->writeConfigurationUInt32(key, data);
}

void PolicyServicesPlatformConfigurationData::writeConfigurationUInt32(
	const std::string& nameSpace,
	const std::string& key,
	UInt32 data)
{
	throwIfNotWorkItemThread();
	getEsifServices()->writeConfigurationUInt32(nameSpace, key, data);
}

std::string PolicyServicesPlatformConfigurationData::readConfigurationString(
	const std::string& nameSpace,
	const std::string& key)
{
	throwIfNotWorkItemThread();
	return getEsifServices()->readConfigurationString(nameSpace, key);
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

TimeSpan PolicyServicesPlatformConfigurationData::getMinimumAllowableSamplePeriod()
{
	throwIfNotWorkItemThread();

	if (m_defaultSamplePeriod.isInvalid())
	{
		m_defaultSamplePeriod =
			getEsifServices()->primitiveExecuteGetAsTimeInMilliseconds(esif_primitive_type::GET_MINIMUM_SAMPLE_PERIOD);
	}

	return m_defaultSamplePeriod;
}

DptfBuffer PolicyServicesPlatformConfigurationData::getActiveRelationshipTable()
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_ACTIVE_RELATIONSHIP_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setActiveRelationshipTable(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Type::Art);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getThermalRelationshipTable()
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_THERMAL_RELATIONSHIP_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setThermalRelationshipTable(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Type::Trt);

}

DptfBuffer PolicyServicesPlatformConfigurationData::getPassiveTable()
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_PASSIVE_RELATIONSHIP_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setPassiveTable(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Psvt);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getAdaptivePerformanceConditionsTable(const std::string& uuid)
{
	throwIfNotWorkItemThread();
	return getDptfManager()->getDataManager()->getTableObject(TableObjectType::Type::Apct, uuid).getData();
}

DptfBuffer PolicyServicesPlatformConfigurationData::getAdaptivePerformanceActionsTable(const std::string& uuid)
{
	throwIfNotWorkItemThread();
	return getDptfManager()->getDataManager()->getTableObject(TableObjectType::Type::Apat, uuid).getData();
}

DptfBuffer PolicyServicesPlatformConfigurationData::getOemVariables()
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_OEM_VARS, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getSwOemVariables()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::SwOemVariables, Constants::EmptyString)
		.getData();
}

void PolicyServicesPlatformConfigurationData::setSwOemVariables(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(
		data, TableObjectType::SwOemVariables);
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

Power PolicyServicesPlatformConfigurationData::getProcessorTdp(UIntN participantIndex, UIntN domainIndex) const
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

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerBossConditionsTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Pbct, Constants::EmptyString)
		.getData();
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerBossActionsTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Pbat, Constants::EmptyString)
		.getData();
}

DptfBuffer PolicyServicesPlatformConfigurationData::getEmergencyCallModeTable()
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_EMERGENCY_CALL_MODE_TABLE, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPidAlgorithmTable()
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGet(esif_primitive_type::GET_PID_ALGORITHM_TABLE, ESIF_DATA_BINARY);
}

void PolicyServicesPlatformConfigurationData::setPidAlgorithmTable(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Type::Pida);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerBossMathTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Pbmt, Constants::EmptyString)
		.getData();
}

DptfBuffer PolicyServicesPlatformConfigurationData::getVoltageThresholdMathTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Vtmt, Constants::EmptyString)
		.getData();
}

DptfBuffer PolicyServicesPlatformConfigurationData::getActiveControlPointRelationshipTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Acpr, Constants::EmptyString)
		.getData();
}

void PolicyServicesPlatformConfigurationData::setActiveControlPointRelationshipTable(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Acpr);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getIntelligentThermalManagementTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Itmt, Constants::EmptyString)
		.getData();
}

void PolicyServicesPlatformConfigurationData::setIntelligentThermalManagementTable(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Itmt);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getIntelligentThermalManagementTable3()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Itmt3, Constants::EmptyString)
		.getData();
}

void PolicyServicesPlatformConfigurationData::setIntelligentThermalManagementTable3(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Itmt3);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getEnergyPerformanceOptimizerTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Epot, Constants::EmptyString)
		.getData();
}

void PolicyServicesPlatformConfigurationData::setEnergyPerformanceOptimizerTable(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Type::Epot);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerShareAlgorithmTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Psha, Constants::EmptyString)
		.getData();
}

void PolicyServicesPlatformConfigurationData::setPowerShareAlgorithmTable(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Psha);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPowerShareAlgorithmTable2()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Psh2, Constants::EmptyString)
		.getData();
}

void PolicyServicesPlatformConfigurationData::setPowerShareAlgorithmTable2(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Psh2);
}

DptfBuffer PolicyServicesPlatformConfigurationData::createResetPrimitiveTupleBinary(
	esif_primitive_type primitive,
	UInt8 instance)
{
	esif_primitive_tuple_parameter tuple{};
	tuple.id.integer.type = esif_data_type::ESIF_DATA_UINT16;
	tuple.id.integer.value = primitive;
	tuple.domain.integer.type = esif_data_type::ESIF_DATA_UINT16;
	const auto domainIndex = createTupleDomain();
	tuple.domain.integer.value = domainIndex;
	tuple.instance.integer.type = esif_data_type::ESIF_DATA_UINT16;
	tuple.instance.integer.value = instance;

	constexpr auto sizeOfTuple = static_cast<UInt32>(sizeof tuple);
	DptfBuffer buffer(sizeOfTuple);
	buffer.put(0, reinterpret_cast<UInt8*>(&tuple), sizeOfTuple);
	return buffer;
}

UInt16 PolicyServicesPlatformConfigurationData::createTupleDomain()
{
	return (('0' + static_cast<UInt8>(0)) << 8) + 'D';
}

void PolicyServicesPlatformConfigurationData::resetActiveRelationshipTable() const
{
	try
	{
		getDptfManager()->getDataManager()->deleteTableObjectKeyForNoPersist(TableObjectType::Art);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetThermalRelationshipTable() const
{
	try
	{
		getDptfManager()->getDataManager()->deleteTableObjectKeyForNoPersist(TableObjectType::Trt);
		
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetPassiveTable() const
{
	try
	{
		getDptfManager()->getDataManager()->deleteTableObjectKeyForNoPersist(TableObjectType::Psvt);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetPidAlgorithmTable() const
{
	try
	{
		getDptfManager()->getDataManager()->deleteTableObjectKeyForNoPersist(TableObjectType::Pida);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetActiveControlPointRelationshipTable() const
{
	try
	{
		const DptfBuffer tableBuffer = createResetPrimitiveTupleBinary(
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

void PolicyServicesPlatformConfigurationData::resetPowerShareAlgorithmTable() const
{
	try
	{
		const DptfBuffer tableBuffer = createResetPrimitiveTupleBinary(
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

void PolicyServicesPlatformConfigurationData::resetPowerShareAlgorithmTable2() const
{
	try
	{
		const DptfBuffer tableBuffer = createResetPrimitiveTupleBinary(
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

void PolicyServicesPlatformConfigurationData::resetIntelligentThermalManagementTable() const
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

void PolicyServicesPlatformConfigurationData::resetIntelligentThermalManagementTable3() const
{
	try
	{
		getDptfManager()->getDataManager()->deleteTableObjectKeyForNoPersist(TableObjectType::Itmt3);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetEnergyPerformanceOptimizerTable() const
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

void PolicyServicesPlatformConfigurationData::resetThirdPartyGraphicsTable() const
{
	try
	{
		getDptfManager()->getDataManager()->deleteTableObjectKeyForNoPersist(TableObjectType::Tpga);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetThirdPartyGraphicsTable2() const
{
	try
	{
		getDptfManager()->getDataManager()->deleteTableObjectKeyForNoPersist(TableObjectType::Opbt);
	}
	catch (...)
	{
		// best effort
	}
}

void PolicyServicesPlatformConfigurationData::resetAllTables() const
{
	resetActiveRelationshipTable();
	resetThermalRelationshipTable();
	resetPassiveTable();
	resetPidAlgorithmTable();
	resetActiveControlPointRelationshipTable();
	resetPowerShareAlgorithmTable();
	resetPowerShareAlgorithmTable2();
	resetIntelligentThermalManagementTable();
	resetIntelligentThermalManagementTable3();
	resetEnergyPerformanceOptimizerTable();
	resetThirdPartyGraphicsTable();
	resetThirdPartyGraphicsTable2();
}

Bool PolicyServicesPlatformConfigurationData::getDisplayRequired()
{
	throwIfNotWorkItemThread();
	const auto displayRequired = getEsifServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_ES_DISPLAY_REQUIRED,
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoInstance);
	return (displayRequired == 1);
}

void PolicyServicesPlatformConfigurationData::setPpmPackage(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	const auto pkgHeader = reinterpret_cast<EsifPpmParamValuesHeader*>(data.get());
	const auto numParams = pkgHeader->numberElement;
	if (numParams > 0)
	{
		const UInt32 ppmParameterSize =
			(numParams - 1) * sizeof(EsifPpmParamValues); // One less since package already accounts for one parameter
		const UInt32 ppmPackageSize = sizeof(EsifPpmParamValuesHeader) + ppmParameterSize;

		getEsifServices()->primitiveExecuteSet(
			esif_primitive_type::SET_PPM_PARAM_VALUES, ESIF_DATA_STRUCTURE, pkgHeader, ppmPackageSize, ppmPackageSize);
	}
}

void PolicyServicesPlatformConfigurationData::setPpmPackageForNonBalancedSchemePersonality(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	const auto pkgHeader = reinterpret_cast<EsifPpmParamValuesHeader*>(data.get());
	const auto numParams = pkgHeader->numberElement;
	if (numParams > 0)
	{
		const UInt32 ppmParameterSize =
			(numParams - 1) * sizeof(EsifPpmParamValues); // One less since package already accounts for one parameter
		const UInt32 ppmPackageSize = sizeof(EsifPpmParamValuesHeader) + ppmParameterSize;

		getEsifServices()->primitiveExecuteSet(
			esif_primitive_type::SET_PPM_NON_BALANCED_VALUES, ESIF_DATA_STRUCTURE, pkgHeader, ppmPackageSize, ppmPackageSize);
	}
}

DptfBuffer PolicyServicesPlatformConfigurationData::getPpmPackage(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();

	DptfBuffer buffer = getEsifServices()->primitiveExecuteGetWithArgument(
		esif_primitive_type::GET_PPM_PARAM_VALUES,
		data,
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoInstance);

	if (buffer.size() < sizeof(EsifPpmParamValues))
	{
		throw invalid_data("Invalid EsifPpmParamValues from GET_PPM_PARAM_VALUES");
	}
	return buffer;
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
	return getEsifServices()->primitiveExecuteGetAsTimeInMilliseconds(
			esif_primitive_type::GET_EXPECTED_BATTERY_LIFE);
}

UInt32 PolicyServicesPlatformConfigurationData::getAggressivenessLevel()
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_AGGRESSIVENESS_LEVEL,
			Constants::Esif::NoParticipant,
			Constants::Esif::NoDomain,
			Constants::Esif::NoInstance);
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

void PolicyServicesPlatformConfigurationData::setProcessAffinityMask(const std::string& processName, UInt32 maskValue)
{
	throwIfNotWorkItemThread();

	AffinityCommand affinityCommand;
	affinityCommand.version = AFFINITY_CMD_VERSION;
	affinityCommand.affinity_mask = maskValue;
	esif_ccb_strcpy(affinityCommand.process_name, processName.c_str(), sizeof(affinityCommand.process_name));

	getEsifServices()->primitiveExecuteSet(
		esif_primitive_type::SET_PROCESS_AFFINITY_MASK,
		ESIF_DATA_STRUCTURE,
		&affinityCommand,
		sizeof(affinityCommand),
		sizeof(affinityCommand),
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoInstance);
}

void PolicyServicesPlatformConfigurationData::setApplicationCompatibility(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();

	getEsifServices()->primitiveExecuteSet(
		esif_primitive_type::SET_APP_COMPAT,
		ESIF_DATA_STRUCTURE,
		data.get(),
		data.size(),
		data.size(),
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoInstance);
}

void PolicyServicesPlatformConfigurationData::deleteApplicationCompatibility()
{
	throwIfNotWorkItemThread();

	AppCompatDeleteCommand applicationCompatibilityDeleteCommand;
	esif_guid_t applicationCompatibilityGuid = APPLICATION_OPTIMIZATION_APPCOMPAT_GUID;

	applicationCompatibilityDeleteCommand.version = APP_COMPAT_DELETE_CMD_VERSION;
	esif_ccb_memcpy(
		&applicationCompatibilityDeleteCommand.app_compat_guid, &applicationCompatibilityGuid, sizeof(applicationCompatibilityDeleteCommand.app_compat_guid));

	getEsifServices()->primitiveExecuteSet(
		esif_primitive_type::SET_APP_COMPAT_DELETE,
		ESIF_DATA_STRUCTURE,
		&applicationCompatibilityDeleteCommand,
		(UInt32)sizeof(applicationCompatibilityDeleteCommand),
		(UInt32)sizeof(applicationCompatibilityDeleteCommand),
		Constants::Esif::NoParticipant,
		Constants::Esif::NoDomain,
		Constants::Esif::NoInstance);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getDdrfTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Ddrf, Constants::EmptyString)
		.getData();
}

DptfBuffer PolicyServicesPlatformConfigurationData::getRfimTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Rfim, Constants::EmptyString)
		.getData();
}

DptfBuffer PolicyServicesPlatformConfigurationData::getAggregateDisplayInformation()
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGet(
		esif_primitive_type::GET_AGGREGATE_DISPLAY_INFORMATION, ESIF_DATA_BINARY);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getTpgaTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Tpga, Constants::EmptyString)
		.getData();
}

void PolicyServicesPlatformConfigurationData::setTpgaTable(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Tpga);
}

void PolicyServicesPlatformConfigurationData::setOpbtTable(const DptfBuffer& data)
{
	throwIfNotWorkItemThread();
	getDptfManager()->getDataManager()->setTableObjectForNoPersist(data, TableObjectType::Opbt);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getOpbtTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Opbt, Constants::EmptyString)
		.getData();
}

UInt32 PolicyServicesPlatformConfigurationData::getDynamicBoostState(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_DYNAMIC_BOOST_STATE, participantIndex, domainIndex);
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

UInt32 PolicyServicesPlatformConfigurationData::getTpgPowerStateWithoutCache(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_TPG_POWER_STATE, participantIndex, domainIndex);
}

EnvironmentProfile PolicyServicesPlatformConfigurationData::getEnvironmentProfile() const
{
	throwIfNotWorkItemThread();
	return getDptfManager()->getEnvironmentProfile();
}

UInt32 PolicyServicesPlatformConfigurationData::getLogicalProcessorCount(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_PROC_LOGICAL_PROCESSOR_COUNT, participantIndex, domainIndex);

}

UInt32 PolicyServicesPlatformConfigurationData::getPhysicalCoreCount(UIntN participantIndex, UIntN domainIndex)
{
	throwIfNotWorkItemThread();
	return getEsifServices()->primitiveExecuteGetAsUInt32(
		esif_primitive_type::GET_PROC_PHYSICAL_CORE_COUNT, participantIndex, domainIndex);
}

DptfBuffer PolicyServicesPlatformConfigurationData::getSystemConfigurationFeatureTable()
{
	throwIfNotWorkItemThread();
	return getDptfManager()
		->getDataManager()
		->getTableObject(TableObjectType::Type::Scft, Constants::EmptyString)
		.getData();
}

string PolicyServicesPlatformConfigurationData::getActiveTableName(TableObjectType::Type tableObjectType)
{
	throwIfNotWorkItemThread();

	return getDptfManager()->getDataManager()->getActiveTableName(tableObjectType);
}