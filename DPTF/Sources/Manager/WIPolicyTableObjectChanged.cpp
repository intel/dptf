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

#include "WIPolicyTableObjectChanged.h"
#include "PolicyManagerInterface.h"
#include "StringConverter.h"

WIPolicyTableObjectChanged::WIPolicyTableObjectChanged(
	DptfManagerInterface* dptfManager,
	TableObjectType::Type tableType,
	const std::string& uuid,
	UIntN participantIndex)
	: WorkItem(dptfManager, FrameworkEvent::PolicyTableObjectChanged)
	, m_tableType(tableType)
	, m_uuid(uuid)
	, m_participantIndex(participantIndex)
{
}

void WIPolicyTableObjectChanged::onExecute()
{
	writeWorkItemStartingInfoMessage();

	const auto policyManager = getPolicyManager();
	const auto policyIndexes = policyManager->getPolicyIndexes();

	for (const unsigned int policyIndex : policyIndexes)
	{
		try
		{
			const auto policy = policyManager->getPolicyPtr(policyIndex);

			if (tableIsForAllPolicies() ||
			    tableIsForThisPolicyOnly(policy->getDynamicPolicyUuidString()))
			{
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
				case TableObjectType::Itmt3:
					policy->executePolicyIntelligentThermalManagementTable3Changed();
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
				case TableObjectType::Rfim:
					policy->executePolicyRfimTableChanged();
					break;
				case TableObjectType::Tpga:
					policy->executePolicyTpgaTableChanged();
					break;
				case TableObjectType::Opbt:
					policy->executePolicyOpbtTableChanged();
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
				case TableObjectType::Scft:
					policy->executePolicySystemConfigurationFeatureTableChanged();
					break;
				case TableObjectType::Dynamic_Idsp:
				default:
					break;
				}
			}
		}
		catch (policy_index_invalid&)
		{
			// do nothing.  No item in the policy list at this index.
		}
		catch (std::exception& ex)
		{
			writeWorkItemErrorMessagePolicy(ex, "Policy::executePolicyTableChanged", policyIndex);
		}
	}
}

bool WIPolicyTableObjectChanged::tableIsForAllPolicies() const
{
	return m_uuid.empty();
}

bool WIPolicyTableObjectChanged::tableIsForThisPolicyOnly(const std::string& policyGuid) const
{
	return policyGuid == StringConverter::toLower(m_uuid);
}