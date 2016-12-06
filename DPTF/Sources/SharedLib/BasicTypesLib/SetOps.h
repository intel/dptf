/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

template <typename T>
class SetOps 
{
public:

    static Bool hasValue(const std::set<T>& aSet, T value);
    static std::set<T> getMissingFromLeft(const std::set<T>& left, const std::set<T>& right);
    static std::set<T> subset(const std::set<T>& keep, const std::set<T>& remove);
    static std::set<T> unionSet(const std::set<T>& leftSet, const std::set<T>& rightSet);
};

template <typename T>
std::set<T> SetOps<T>::unionSet(const std::set<T>& leftSet, const std::set<T>& rightSet)
{
    std::set<T> newSet;
    newSet.insert(leftSet.begin(), leftSet.end());
    newSet.insert(rightSet.begin(), rightSet.end());
    return newSet;
}

template <typename T>
Bool SetOps<T>::hasValue(const std::set<T>& aSet, T value)
{
    return (aSet.find(value) != aSet.end());
}

template <typename T>
std::set<T> SetOps<T>::getMissingFromLeft(const std::set<T>& left, const std::set<T>& right)
{
    std::set<T> result;
    for (auto value = right.begin(); value != right.end(); ++value)
    {
        if (!(SetOps<T>::hasValue(left, *value)))
        {
            result.insert(*value);
        }
    }

    return result;
}

template <typename T>
std::set<T> SetOps<T>::subset(const std::set<T>& keep, const std::set<T>& remove)
{
    std::set<T> theSubset = keep;
    for (auto value = remove.begin(); value != remove.end(); ++value)
    {
        theSubset.erase(*value);
    }
    return theSubset;
}
