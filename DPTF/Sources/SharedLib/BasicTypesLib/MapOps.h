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

template <typename K, typename V> class MapOps
{
public:
	static std::set<K> getKeys(const std::map<K, V>& aMap);
	static std::list<std::pair<K, V>> getAsList(const std::map<K, V>& aMap);
};

template <typename K, typename V>
std::set<K> MapOps<K, V>::getKeys(const std::map<K, V>& aMap)
{
	std::set<K> keys;
	for (auto aPair = aMap.begin(); aPair != aMap.end(); ++aPair)
	{
		keys.insert(aPair->first);
	}
	return keys;
}

template <typename K, typename V>
std::list<std::pair<K, V>> MapOps<K, V>::getAsList(const std::map<K, V>& aMap)
{
	std::list<std::pair<K, V>> result;
	for (const auto& item : aMap)
	{
		result.emplace_back(item);
	}
	return result;
}
