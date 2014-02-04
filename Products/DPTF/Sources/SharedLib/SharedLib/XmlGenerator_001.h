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

#pragma once

#include "XmlGeneratorInterface.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

//
// Implements RapidXML
//

class XmlGenerator_001 final : public XmlGeneratorInterface
{
public:

    XmlGenerator_001();
    XmlGenerator_001(XmlNode* node);

    // XmlGeneratorInterface
    virtual void addNode(XmlNode* node) override final;
    virtual std::string toString(void) override final;

private:

    void addChildren(XmlNode* node, RapidXml::xml_node<>* tempNode);
    RapidXml::xml_node<>* allocateRapidXmlNode(XmlNode* node);
    RapidXml::xml_document<> m_doc;
};