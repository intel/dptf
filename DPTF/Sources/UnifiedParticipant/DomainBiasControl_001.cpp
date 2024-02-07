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

#include "DomainBiasControl_001.h"
#include "XmlNode.h"

DomainBiasControl_001::DomainBiasControl_001(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: DomainBiasControlBase(participantIndex, domainIndex, participantServicesInterface)
{
}

DomainBiasControl_001::~DomainBiasControl_001()
{
}

void DomainBiasControl_001::setCpuOpboostEnableAC(Bool enabled)
{
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_CPU_OPBOOST_ENABLE_AC,
		static_cast<UInt32>(enabled),
		getDomainIndex()
	);
}

void DomainBiasControl_001::setCpuOpboostEnableDC(Bool enabled)
{
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_CPU_OPBOOST_ENABLE_DC,
		static_cast<UInt32>(enabled),
		getDomainIndex()
	);
}

void DomainBiasControl_001::setGpuOpboostEnableAC(Bool enabled)
{
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_GPU_OPBOOST_ENABLE_AC,
		static_cast<UInt32>(enabled),
		getDomainIndex()
	);
}

void DomainBiasControl_001::setGpuOpboostEnableDC(Bool enabled)
{
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_GPU_OPBOOST_ENABLE_DC,
		static_cast<UInt32>(enabled),
		getDomainIndex()
	);
}

void DomainBiasControl_001::setSplitRatio(const Percentage& splitRatio)
{
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_GPU_SPLIT_RATIO,
		static_cast<UInt32>(splitRatio.toCentiPercent()),
		getDomainIndex()
	);
}

void DomainBiasControl_001::setSplitRatioMax(const Percentage& splitRatio)
{
	getParticipantServices()->primitiveExecuteSetAsUInt32(
		esif_primitive_type::SET_GPU_SPLIT_RATIO_MAX, 
		static_cast<UInt32>(splitRatio.toCentiPercent()), 
		getDomainIndex()
	);
}

Bool DomainBiasControl_001::getCpuOpboostEnableAC()
{
	const auto cpuOpboostEnable =
		getParticipantServices()->primitiveExecuteGetAsUInt32(GET_CPU_OPBOOST_ENABLE_AC, getDomainIndex());
	return static_cast<Bool>(cpuOpboostEnable);
}

Bool DomainBiasControl_001::getCpuOpboostEnableDC()
{
	const auto cpuOpboostEnable =
		getParticipantServices()->primitiveExecuteGetAsUInt32(GET_CPU_OPBOOST_ENABLE_DC, getDomainIndex());
	return static_cast<Bool>(cpuOpboostEnable);
}

Bool DomainBiasControl_001::getGpuOpboostEnableAC()
{
	const auto gpuOpboostEnable =
		getParticipantServices()->primitiveExecuteGetAsUInt32(GET_GPU_OPBOOST_ENABLE_AC, getDomainIndex());
	return static_cast<Bool>(gpuOpboostEnable);
}

Bool DomainBiasControl_001::getGpuOpboostEnableDC()
{
	const auto gpuOpboostEnable =
		getParticipantServices()->primitiveExecuteGetAsUInt32(GET_GPU_OPBOOST_ENABLE_DC, getDomainIndex());
	return static_cast<Bool>(gpuOpboostEnable);
}

Percentage DomainBiasControl_001::getSplitRatio()
{
	const auto splitRatio =
		getParticipantServices()->primitiveExecuteGetAsUInt32(GET_GPU_SPLIT_RATIO, getDomainIndex());
	return Percentage::fromCentiPercent(splitRatio);
}

Percentage DomainBiasControl_001::getSplitRatioActive()
{
	const auto splitRatioActive =
		getParticipantServices()->primitiveExecuteGetAsUInt32(GET_GPU_SPLIT_RATIO_ACTIVE, getDomainIndex());
	return Percentage::fromCentiPercent(splitRatioActive);
}

Percentage DomainBiasControl_001::getSplitRatioMax()
{
	const auto splitRatioMax =
		getParticipantServices()->primitiveExecuteGetAsUInt32(GET_GPU_SPLIT_RATIO_MAX, getDomainIndex());
	return Percentage::fromCentiPercent(splitRatioMax);
}

Power DomainBiasControl_001::getReservedTgp()
{
	const auto reservedTgp =
		getParticipantServices()->primitiveExecuteGetAsUInt32(GET_RESERVED_TGP, getDomainIndex());
	return Power::createFromMilliwatts(reservedTgp);
}

OpportunisticBoostMode::Type DomainBiasControl_001::getOppBoostMode()
{
	const auto oppBoostMode =
		getParticipantServices()->primitiveExecuteGetAsUInt32(GET_OPBOOST_MODE, getDomainIndex());
	return OpportunisticBoostMode::fromUInt32(oppBoostMode);
}

std::string DomainBiasControl_001::getName(void)
{
	return "Bias Control";
}

std::shared_ptr<XmlNode> DomainBiasControl_001::getXml(UIntN domainIndex)
{
	std::shared_ptr<XmlNode> root = XmlNode::createRoot();

	std::shared_ptr<XmlNode> node = XmlNode::createWrapperElement("bias_control");
	node->addChild(XmlNode::createDataElement("control_name", getName()));
	node->addChild(XmlNode::createDataElement("control_knob_version", "001"));
	root->addChild(node);

	return root;
}