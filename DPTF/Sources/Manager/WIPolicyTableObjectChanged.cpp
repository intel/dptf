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

#include "WIPolicyTableObjectChanged.h"
#include "PolicyManagerInterface.h"
#include "EsifServicesInterface.h"
#include "StringConverter.h"

WIPolicyTableObjectChanged::WIPolicyTableObjectChanged(
	DptfManagerInterface* dptfManager,
	TableObjectType::Type tableType,
	std::string uuid,
	UIntN participantIndex)
	: WorkItem(dptfManager, FrameworkEvent::PolicyTableObjectChanged)
	, m_tableType(tableType)
	, m_uuid(uuid)
	, m_participantIndex(participantIndex)
{
}

WIPolicyTableObjectChanged::~WIPolicyTableObjectChanged(void)
{
}

void WIPolicyTableObjectChanged::onExecute(void)
{
	writeWorkItemStartingInfoMessage();

	auto policyManager = getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();

	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		try
		{
			auto policy = policyManager->getPolicyPtr(*i);
			if (policy->getDynamicPolicyUuidString() != StringConverter::toLower(m_uuid))
			{
				continue;
			}

			switch (m_tableType)
			{
			case TableObjectType::Acpr:
				policy->executePolicyActiveControlPointRelationshipTableChanged();
				break;
			case TableObjectType::Apat:
				policy->executePolicyAdaptivePerformanceActionsTableChanged();
				break;
			case TableObjectType::Apct:
				policy->executePolicyAdaptivePerformanceConditionsTableChanged();
				break;
			case TableObjectType::Art:
				policy->executePolicyActiveRelationshipTableChanged();
				break;
			case TableObjectType::Ddrf:
				policy->executePolicyDdrfTableChanged();
				break;
			case TableObjectType::Epot:
				policy->executePolicyEnergyPerformanceOptimizerTableChanged();
				break;
			case TableObjectType::Itmt:
				policy->executePolicyIntelligentThermalManagementTableChanged();
				break;
			case TableObjectType::Odvp:
				policy->executePolicyOemVariablesChanged();
				break;
			case TableObjectType::Pbat:
				policy->executePolicyPowerBossActionsTableChanged();
				break;
			case TableObjectType::Pbct:
				policy->executePolicyPowerBossConditionsTableChanged();
				break;
			case TableObjectType::Pbmt:
				policy->executePolicyPowerBossMathTableChanged();
				break;
			case TableObjectType::Pida:
				policy->executePolicyPidAlgorithmTableChanged();
				break;
			case TableObjectType::Psh2:
				policy->executePolicyPowerShareAlgorithmTable2Changed();
				break;
			case TableObjectType::Psha:
				policy->executePolicyPowerShareAlgorithmTableChanged();
				break;
			case TableObjectType::Psvt:
				policy->executePolicyPassiveTableChanged();
				break;
			case TableObjectType::Tpga:
				policy->executePolicyTpgaTableChanged();
				break;
			case TableObjectType::Trt:
				policy->executePolicyThermalRelationshipTableChanged();
				break;
			case TableObjectType::Vsct:
				policy->executeDomainVirtualSensorCalibrationTableChanged(m_participantIndex);
				break;
			case TableObjectType::Vspt:
				policy->executeDomainVirtualSensorPollingTableChanged(m_participantIndex);
				break;
			case TableObjectType::Vtmt:
				policy->executePolicyVoltageThresholdMathTableChanged();
				break;
			case TableObjectType::SwOemVariables:
				policy->executeSwOemVariablesChanged();
				break;
			default:
				break;
			}
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "Policy::executePolicyTableChanged", *i);
		}
	}
}
