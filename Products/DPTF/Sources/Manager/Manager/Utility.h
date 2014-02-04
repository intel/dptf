/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#include "Dptf.h"

template <typename T>
UIntN getFirstNonNullIndex(T& someStlContainer)
{
    UIntN index = 0;

    for (index = 0; index < someStlContainer.size(); index++)
    {
        if (someStlContainer[index] == nullptr)
        {
            break;
        }
    }

    if (index == someStlContainer.size())
    {
        someStlContainer.push_back(nullptr);
    }

    return index;
}

template <typename T>
void increaseVectorSizeIfNeeded(T& someStlContainer, UIntN maxVectorIndex)
{
    // make sure there are enough rows in the vector
    while (maxVectorIndex >= someStlContainer.size())
    {
        someStlContainer.push_back(nullptr);
    }
}

template <typename T>
void increaseVectorSizeIfNeeded(T& someStlContainer, UIntN maxVectorIndex, UIntN defaultValue)
{
    // make sure there are enough rows in the vector
    while (maxVectorIndex >= someStlContainer.size())
    {
        someStlContainer.push_back(defaultValue);
    }
}

template <typename containerType, typename parameterType>
void increaseVectorSizeIfNeeded(containerType& someStlContainer, UIntN maxVectorIndex, parameterType& defaultValue)
{
    // make sure there are enough rows in the vector
    while (maxVectorIndex >= someStlContainer.size())
    {
        someStlContainer.push_back(defaultValue);
    }
}