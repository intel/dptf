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

#include "TargetSourceRelationship.h"

using namespace std;

TargetSourceRelationship::TargetSourceRelationship()
    : target(Constants::Invalid), source(Constants::Invalid), m_valid(false)
{

}

TargetSourceRelationship::TargetSourceRelationship(UIntN newTarget, UIntN newSource)
    : target(newTarget), source(newSource), m_valid(true)
{

}

Bool TargetSourceRelationship::operator<(const TargetSourceRelationship& rhs) const
{
    throwIfNotValid();
    if (target < rhs.target)
    {
        return true;
    }
    else if (target == rhs.target)
    {
        return (source < rhs.source);
    }
    else
    {
        return false;
    }
}

Bool TargetSourceRelationship::operator==(const TargetSourceRelationship& rhs) const
{
    throwIfNotValid();
    return ((target == rhs.target) && (source == rhs.source));
}

void TargetSourceRelationship::throwIfNotValid() const
{
    if (m_valid == false)
    {
        throw dptf_exception("Invalid target-source relationship.");
    }
}