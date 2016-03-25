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
#include "ControlFactoryInterface.h"
#include "ControlFactoryType.h"
#include <map>
#include <memory>

class ControlFactoryList
{
public:

    ControlFactoryList(void);
    ~ControlFactoryList(void);

    std::shared_ptr<ControlFactoryInterface> getFactory(ControlFactoryType::Type factoryType) const;

private:

    std::map<ControlFactoryType::Type, std::shared_ptr<ControlFactoryInterface>> m_factories;
    std::shared_ptr<ControlFactoryInterface> makeFactory(ControlFactoryType::Type factoryType);
};