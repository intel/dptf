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

#include "DomainPowerFilter.h"
#include "XmlNode.h"
#include "esif_ccb.h"
#include "StatusFormat.h"

DomainPowerFilter::DomainPowerFilter(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServices)
	: m_powerSeed(std::map<PowerControlType::Type, CachedValue<Power>>())
	, m_powerAlpha(std::map<PowerControlType::Type, CachedValue<double>>())
	, m_powerDelta(std::map<PowerControlType::Type, CachedValue<double>>())
	, m_lastPowerUsed(std::map<PowerControlType::Type, CachedValue<Power>>())
	, m_lastEwmaPower(std::map<PowerControlType::Type, CachedValue<Power>>())
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_participantServices(participantServices)
{
}

DomainPowerFilter::~DomainPowerFilter()
{
}

Power DomainPowerFilter::getAveragePower(const PowerControlDynamicCaps& capabilities, Power currentPower)
{
	auto type = capabilities.getPowerControlType();
	Power lastPowerUsed = getLastPowerUsed(type);
	double alpha = getPowerAlpha(type);

	Power newPowerUsed;
	Power newEwmaPower;

	if (lastPowerUsed.isValid() == false)
	{
		newEwmaPower = (UInt32)(alpha * currentPower + (1 - alpha) * getPowerSeed(capabilities));
		newPowerUsed = (currentPower > newEwmaPower) ? currentPower : newEwmaPower;
	}
	else
	{
		newEwmaPower = (UInt32)(alpha * currentPower + (1 - alpha) * lastPowerUsed);
		if (currentPower < newEwmaPower)
		{
			newPowerUsed = calculatePowerLow(capabilities, currentPower, lastPowerUsed);
		}
		else
		{
			newPowerUsed = calculatePowerHigh(type, currentPower, newEwmaPower);
		}
	}

	updateHistory(type, newPowerUsed, newEwmaPower);

	return newPowerUsed;
}

Power DomainPowerFilter::getPowerSeed(const PowerControlDynamicCaps& capabilities)
{
	auto type = capabilities.getPowerControlType();
	if (m_powerSeed[type].isInvalid())
	{
		Power seed = Power::createInvalid();
		try
		{
			seed = m_participantServices->primitiveExecuteGetAsPower(
				esif_primitive_type::GET_POWER_SEED, m_domainIndex, (UInt8)type);
		}
		catch (...)
		{
			seed = (Power)((capabilities.getMaxPowerLimit() + capabilities.getMinPowerLimit()) / 2);
		}
		m_powerSeed[type].set(seed);
	}
	return m_powerSeed[type].get();
}

double DomainPowerFilter::getPowerAlpha(PowerControlType::Type type)
{
	if (m_powerAlpha[type].isInvalid())
	{
		UInt32 hundredthAlpha = m_participantServices->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_POWER_ALPHA, m_domainIndex, (UInt8)type);
		m_powerAlpha[type].set(createFromHundredth(hundredthAlpha));
	}
	return m_powerAlpha[type].get();
}

Power DomainPowerFilter::getMaxPowerChange(const PowerControlDynamicCaps& capabilities)
{
	auto type = capabilities.getPowerControlType();
	if (m_powerDelta[type].isInvalid())
	{
		UInt32 hundredthDelta = m_participantServices->primitiveExecuteGetAsUInt32(
			esif_primitive_type::GET_POWER_DELTA, m_domainIndex, (UInt8)type);
		m_powerDelta[type].set(createFromHundredth(hundredthDelta));
	}

	Power maxPowerChange =
		(UInt32)(m_powerDelta[type].get() * (capabilities.getMaxPowerLimit() - capabilities.getMinPowerLimit()));

	return maxPowerChange;
}

Power DomainPowerFilter::getLastEwmaPower(PowerControlType::Type type)
{
	if (m_lastEwmaPower[type].isInvalid())
	{
		try
		{
			m_lastEwmaPower[type].set(m_participantServices->primitiveExecuteGetAsPower(
				esif_primitive_type::GET_LAST_EWMA_POWER, m_domainIndex, (UInt8)type));
		}
		catch (...)
		{
			m_lastEwmaPower[type].set(Power::createInvalid());
		}
	}

	return m_lastEwmaPower[type].get();
}

Power DomainPowerFilter::getLastPowerUsed(PowerControlType::Type type)
{
	if (m_lastPowerUsed[type].isInvalid())
	{
		try
		{
			m_lastPowerUsed[type].set(m_participantServices->primitiveExecuteGetAsPower(
				esif_primitive_type::GET_LAST_POWER_USED, m_domainIndex, (UInt8)type));
		}
		catch (...)
		{
			m_lastPowerUsed[type].set(Power::createInvalid());
		}
	}

	return m_lastPowerUsed[type].get();
}

void DomainPowerFilter::setLastEwmaPower(PowerControlType::Type type, Power value)
{
	m_lastEwmaPower[type].set(value);
	try
	{
		m_participantServices->primitiveExecuteSetAsPower(
			esif_primitive_type::SET_LAST_EWMA_POWER, value, m_domainIndex, (UInt8)type);
	}
	catch (file_open_create_failure&)
	{
		// system is in read-only mode, cannot save historical data on disk
	}
}

void DomainPowerFilter::setLastPowerUsed(PowerControlType::Type type, Power value)
{
	m_lastPowerUsed[type].set(value);
	try
	{
		m_participantServices->primitiveExecuteSetAsPower(
			esif_primitive_type::SET_LAST_POWER_USED, value, m_domainIndex, (UInt8)type);
	}
	catch (file_open_create_failure&)
	{
		// system is in read-only mode, cannot save historical data on disk
	}
}

Power DomainPowerFilter::calculatePowerHigh(PowerControlType::Type type, Power currentPower, Power newEwmaPower)
{
	if (currentPower > getLastEwmaPower(type))
	{
		return (Power)((currentPower + newEwmaPower) / 2);
	}
	else
	{
		return currentPower;
	}
}

Power DomainPowerFilter::calculatePowerLow(
	const PowerControlDynamicCaps& capabilities,
	Power currentPower,
	Power lastPowerUsed)
{
	Power maxPowerChange = getMaxPowerChange(capabilities);
	if ((lastPowerUsed - currentPower) >= maxPowerChange)
	{
		return (lastPowerUsed - maxPowerChange);
	}
	else
	{
		return currentPower;
	}
}

double DomainPowerFilter::createFromHundredth(UInt32 hundredthValue)
{
	return (hundredthValue / 100.0);
}

void DomainPowerFilter::updateHistory(PowerControlType::Type type, Power newPowerUsed, Power newEwmaPower)
{
	setLastPowerUsed(type, newPowerUsed);
	setLastEwmaPower(type, newEwmaPower);
}

void DomainPowerFilter::clearCachedData(void)
{
	m_powerSeed.clear();
	m_powerAlpha.clear();
	m_powerDelta.clear();
	m_lastPowerUsed.clear();
	m_lastEwmaPower.clear();
}

std::shared_ptr<XmlNode> DomainPowerFilter::getXml()
{
	std::shared_ptr<XmlNode> root = XmlNode::createWrapperElement("average_power_set");
	// For now: only displaying XML for PL1
	root->addChild(createStatusNode(PowerControlType::PL1));

	return root;
}

std::shared_ptr<XmlNode> DomainPowerFilter::createStatusNode(PowerControlType::Type type)
{
	std::shared_ptr<XmlNode> status = XmlNode::createWrapperElement("average_power");
	auto power = getLastPowerUsed(type);
	if (power.isValid())
	{
		status->addChild(XmlNode::createDataElement("value", power.toString()));
	}
	else
	{
		status->addChild(XmlNode::createDataElement("value", StatusFormat::friendlyValue(Constants::Invalid)));
	}

	return status;
}
