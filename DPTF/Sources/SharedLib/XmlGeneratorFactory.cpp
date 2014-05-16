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

#include "XmlGeneratorFactory.h"
#include "XmlGenerator_000.h"
#include "XmlGenerator_001.h"

XmlGeneratorInterface* XmlGeneratorFactory::createXmlGeneratorObject(UIntN version)
{
    switch (version)
    {
        case 0:
            return new XmlGenerator_000();
            break;
        case 1: // RapidXML
            return new XmlGenerator_001();
            break;
        default:
            std::stringstream message;
            message << "Received request for XmlGenerator version that isn't defined: " << version;
            throw dptf_exception(message.str());
            break;
    }
}