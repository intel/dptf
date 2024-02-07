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

#pragma once

#include "Dptf.h"
#include "DttConfigurationSegment.h"
#include "DomainProxyInterface.h"
#include "MLModelLearningParameter.h"
#include "MLModelNode.h"
#include "MLModelLayer.h"
#include "ToleranceLevel.h"

class MLModelConfiguration
{
public:
	MLModelConfiguration();
	MLModelConfiguration(const DttConfigurationSegment& segment);

	~MLModelConfiguration() = default;

	MLModelConfiguration(const MLModelConfiguration& other) = default;
	MLModelConfiguration& operator=(const MLModelConfiguration& other) = default;
	MLModelConfiguration(MLModelConfiguration&& other) = default;
	MLModelConfiguration& operator=(MLModelConfiguration&& other) = default;

	UInt32 getVersion() const;
	std::string getModelType() const;
	std::string getITMTableRowNumber() const;
	MLModelLayer getInputLayer() const;
	MLModelLayer getHiddenLayer() const;
	std::map<ToleranceLevel::Level, MLModelLearningParameter> getLearningParameters() const;
	MLModelLearningParameter getLearningParameterByToleranceLevel(ToleranceLevel::Level level) const;
	std::map<std::string, std::map<std::string, double>> getLearningParametersData() const;
	std::map<std::string, std::list<std::map<std::string, std::vector<double>>>> getInputLayerNodeWeights() const;
	std::map<std::string, std::list<std::map<std::string, std::vector<double>>>> getHiddenLayerNodeWeights() const;

	bool operator==(const MLModelConfiguration& rhs) const;

private:
	void parseMLModelFromConfigurationSegment(
		const DttConfigurationSegment& segment);
	std::map<ToleranceLevel::Level, MLModelLearningParameter> createLearningParametersFromSegment(
		const DttConfigurationSegment& segment,
		const std::string& keyPrefix) const;
	MLModelNode createNodeFromSegment(
		const DttConfigurationSegment& segment,
		const std::string& keyPrefix) const;
	MLModelLayer createLayerFromSegment(
		const DttConfigurationSegment& segment,
		const std::string& keyPrefix,
		const std::string& layerName) const;
	MLModelLearningParameter createLearningParameterByToleranceLevelFromSegment(
		const DttConfigurationSegment& segment,
		const std::string& keyPrefix,
		const ToleranceLevel::Level level) const;

	std::map<std::string, std::list<std::map<std::string, std::vector<double>>>> getNodeWeights(
		const MLModelLayer& layer) const;

	UInt32 m_version;
	std::string m_modelType;
	std::string m_itmTableRowNumber;
	std::vector<MLModelLayer> m_layers;
	MLModelLayer m_inputLayer;
	MLModelLayer m_hiddenLayer;
	std::map<ToleranceLevel::Level, MLModelLearningParameter> m_learningParameters;
};
