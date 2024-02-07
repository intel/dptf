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

#include "MLModelConfiguration.h"
#include "StringConverter.h"
#include "PolicyLogger.h"

using namespace std;

const auto configMLModelKeyPath = "/Configuration/MLModel"s;
const auto versionKeyPath = "/Version"s;
const auto modelTypeKeyPath = "/ModelType"s;
const auto itmTableRowNumberKeyPath = "/ITMTableRowNo"s;
const auto LayersKeyPath = "/Layers"s;
const auto inputLayerKeyPath = "/Layers/0"s;
const auto hiddenLayerKeyPath = "/Layers/1"s;
const auto learningParametersKeyPath = "/ToleranceLearningParameters"s;
const auto nodesKeyPath = "/Nodes"s;
const auto weightsKeyPath = "/Weights"s;
const auto learningRateKeyPath = "/LearningRate"s;
const auto smoothingFactorKeyPath = "/SmoothingFactor"s;
const auto tempOffsetKeyPath = "/TempOffset"s;
const auto batchSizeKeyPath = "/BatchSize"s;
const auto regexAnyCharacter = "/.*"s;

MLModelConfiguration::MLModelConfiguration()
	: m_version(Constants::Invalid)
{
}

MLModelConfiguration::MLModelConfiguration(const DttConfigurationSegment& segment)
	: m_version(Constants::Invalid)
{
	try
	{
		parseMLModelFromConfigurationSegment(segment);
	}
	catch (...)
	{
		throw dptf_exception("Failed to create MLModel from configuration");
	}
}

UInt32 MLModelConfiguration::getVersion() const
{
	return m_version;
}

string MLModelConfiguration::getModelType() const
{
	return m_modelType;
}

string MLModelConfiguration::getITMTableRowNumber() const
{
	return m_itmTableRowNumber;
}

MLModelLayer MLModelConfiguration::getInputLayer() const
{
	return m_inputLayer;
}

MLModelLayer MLModelConfiguration::getHiddenLayer() const
{
	return m_hiddenLayer;
}

map<ToleranceLevel::Level, MLModelLearningParameter> MLModelConfiguration::getLearningParameters() const
{
	return m_learningParameters;
}

MLModelLearningParameter MLModelConfiguration::getLearningParameterByToleranceLevel(ToleranceLevel::Level level) const
{
	if (m_learningParameters.find(level) == m_learningParameters.end())
	{
		throw dptf_exception("Failed to get the LearningParameter for " + toString(level));
	}

	return m_learningParameters.at(level);
}

void MLModelConfiguration::parseMLModelFromConfigurationSegment(
	const DttConfigurationSegment& segment)
{
	m_version = segment.getValueAsUInt32(configMLModelKeyPath + versionKeyPath);
	m_modelType = segment.getValueAsString(configMLModelKeyPath + modelTypeKeyPath);
	m_itmTableRowNumber = segment.getValueAsString(configMLModelKeyPath + itmTableRowNumberKeyPath);
	m_inputLayer = createLayerFromSegment(segment, configMLModelKeyPath + inputLayerKeyPath, "Input Layer"s);
	m_hiddenLayer = createLayerFromSegment(segment, configMLModelKeyPath + hiddenLayerKeyPath, "Hidden Layer");
	m_learningParameters = createLearningParametersFromSegment(segment, configMLModelKeyPath + learningParametersKeyPath);
}

map<ToleranceLevel::Level, MLModelLearningParameter> MLModelConfiguration::createLearningParametersFromSegment(
	const DttConfigurationSegment& segment,
	const std::string& keyPrefix) const
{
	map<ToleranceLevel::Level, MLModelLearningParameter> learningParameters;
	auto learningParameter = createLearningParameterByToleranceLevelFromSegment(segment, keyPrefix, ToleranceLevel::High);
	learningParameters.insert({ToleranceLevel::High, learningParameter});
	learningParameter = createLearningParameterByToleranceLevelFromSegment(segment, keyPrefix, ToleranceLevel::Medium);
	learningParameters.insert({ToleranceLevel::Medium, learningParameter});
	learningParameter = createLearningParameterByToleranceLevelFromSegment(segment, keyPrefix, ToleranceLevel::Low);
	learningParameters.insert({ToleranceLevel::Low, learningParameter});

	return learningParameters;
}

MLModelNode MLModelConfiguration::createNodeFromSegment(
	const DttConfigurationSegment& segment,
	const std::string& keyPrefix) const
{
	vector<double> weights;
	UInt32 weightIndex = 0;
	string queryKey = keyPrefix + weightsKeyPath + "/" + to_string(weightIndex);
	auto keysMatch = segment.getKeysThatMatch(DttConfigurationQuery(queryKey));
	while (!keysMatch.empty())
	{
		const auto weightString = segment.getValueAsString(queryKey);
		weights.push_back(StringConverter::toDouble(weightString));

		weightIndex++;
		queryKey = keyPrefix + weightsKeyPath + "/" + to_string(weightIndex);
		keysMatch = segment.getKeysThatMatch(DttConfigurationQuery(queryKey));
	}

	return {weights};
}

MLModelLayer MLModelConfiguration::createLayerFromSegment(
	const DttConfigurationSegment& segment,
	const string& keyPrefix,
	const string& layerName) const
{
	vector<MLModelNode> nodes;
	UInt32 nodeIndex = 0;
	string nodeKey = keyPrefix + nodesKeyPath + "/" + to_string(nodeIndex);
	auto keysMatch = segment.getKeysThatMatch(DttConfigurationQuery(nodeKey + regexAnyCharacter));
	while (!keysMatch.empty())
	{
		auto newLayerNode = createNodeFromSegment(segment, nodeKey);
		nodes.push_back(newLayerNode);

		nodeIndex++;
		nodeKey = keyPrefix + nodesKeyPath + "/" + to_string(nodeIndex);
		keysMatch = segment.getKeysThatMatch(DttConfigurationQuery(nodeKey + regexAnyCharacter));
	}

	return {layerName, nodes};
}

MLModelLearningParameter MLModelConfiguration::createLearningParameterByToleranceLevelFromSegment(
	const DttConfigurationSegment& segment,
	const std::string& keyPrefix,
	const ToleranceLevel::Level level) const
{
	const auto newKeyString = keyPrefix + "/" + toString(level);

	const auto learningRate = segment.getValueAsString(newKeyString + learningRateKeyPath);
	const auto smoothingFactor = segment.getValueAsString(newKeyString + smoothingFactorKeyPath);
	const auto tempOffset = segment.getValueAsString(newKeyString + tempOffsetKeyPath);
	const auto batchSize = segment.getValueAsString(newKeyString + batchSizeKeyPath);

	return {learningRate, smoothingFactor, tempOffset, batchSize};
}

map<string, list<map<string, vector<double>>>> MLModelConfiguration::getNodeWeights(const MLModelLayer& layer) const
{
	std::list<std::map<std::string, std::vector<double>>> nodeList;
	std::map<std::string, std::list<std::map<std::string, std::vector<double>>>> finalNodeMap;
	const auto nodes = layer.nodes;
	for (UIntN nodeIndex = 0; nodeIndex < nodes.size(); nodeIndex++)
	{
		auto weights = nodes.at(nodeIndex).weights;
		std::map<std::string, std::vector<double>> weightList;
		weightList["Weights"] = weights;
		nodeList.push_back(weightList);
	}
	finalNodeMap["Nodes"] = nodeList;
	return {finalNodeMap};
}

map<string, map<string, double>> MLModelConfiguration::getLearningParametersData() const
{
	std::map<std::string, std::map<std::string, double>> toleranceLearningParameters;
	for (auto parameterIterator = m_learningParameters.begin(); parameterIterator != m_learningParameters.end();
		 ++parameterIterator)
	{
		auto toleranceLevel = ToleranceLevel::toString(parameterIterator->first);
		toleranceLearningParameters[toleranceLevel]["LearningRate"s] = parameterIterator->second.learningRate;
		toleranceLearningParameters[toleranceLevel]["SmoothingFactor"s] = parameterIterator->second.smoothingFactor;
		toleranceLearningParameters[toleranceLevel]["TempOffset"s] = parameterIterator->second.tempOffset;
		toleranceLearningParameters[toleranceLevel]["BatchSize"s] = parameterIterator->second.batchSize;
	}
	return toleranceLearningParameters;
}

map<string,list<map<string, vector<double>>>> MLModelConfiguration:: getInputLayerNodeWeights() const
{
	return getNodeWeights(m_inputLayer);
}

map<string, list<map<string, vector<double>>>> MLModelConfiguration:: getHiddenLayerNodeWeights() const
{
	return getNodeWeights(m_hiddenLayer);
}

bool MLModelConfiguration::operator==(const MLModelConfiguration& rhs) const
{
	return ((m_version == rhs.m_version) 
		&& (m_modelType == rhs.m_modelType)
		&& (m_itmTableRowNumber == rhs.m_itmTableRowNumber)
		&& (m_inputLayer == rhs.m_inputLayer)
		&& (m_hiddenLayer == rhs.m_hiddenLayer) 
		&& (m_learningParameters == rhs.m_learningParameters));
}
