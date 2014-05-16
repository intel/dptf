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

#include "XmlGenerator_001.h"
#include "XmlNode.h"

XmlGenerator_001::XmlGenerator_001()
{
}

XmlGenerator_001::XmlGenerator_001(XmlNode* node)
{
    addNode(node);
}

void XmlGenerator_001::addNode(XmlNode* node)
{
    if (node->getNodeType() != NodeType::Root)
    {
        RapidXml::xml_node<>* root = allocateRapidXmlNode(node);
        m_doc.append_node(root);

        std::vector<XmlNode*> children = node->getChildren();

        for (UIntN i = 0; i < children.size(); i++)
        {
            RapidXml::xml_node<>* currentChild = allocateRapidXmlNode(children[i]);
            root->append_node(currentChild);
            addChildren(children[i], currentChild);
        }
    }
    else
    {
        // Add children (and children of children, etc.)
        std::vector<XmlNode*> children = node->getChildren();

        for (UIntN i = 0; i < children.size(); i++)
        {
            RapidXml::xml_node<>* currentChild = allocateRapidXmlNode(children[i]);
            m_doc.append_node(currentChild);
            addChildren(children[i], currentChild);
        }
    }
}

std::string XmlGenerator_001::toString(void)
{
    std::string s;
    RapidXml::print(std::back_inserter(s), m_doc, 0);
    return s;
}

void XmlGenerator_001::addChildren(XmlNode* node, RapidXml::xml_node<>* tempNode)
{
    // Add children (and children of children, etc.)
    std::vector<XmlNode*> children = node->getChildren();

    for (UIntN i = 0; i < children.size(); i++)
    {
        RapidXml::xml_node<>* currentChild = allocateRapidXmlNode(children[i]);
        tempNode->append_node(currentChild);

        addChildren(children[i], currentChild);
    }
}

RapidXml::xml_node<>* XmlGenerator_001::allocateRapidXmlNode(XmlNode* node)
{
    switch (node->getNodeType())
    {
        case NodeType::Type::Comment:
            return m_doc.allocate_node(RapidXml::node_comment,
                m_doc.allocate_string(node->getXmlTag().c_str()),
                m_doc.allocate_string(node->getData().c_str()));
        case NodeType::Element:
            return m_doc.allocate_node(RapidXml::node_element,
                m_doc.allocate_string(node->getXmlTag().c_str()),
                m_doc.allocate_string(node->getData().c_str()));
        default:
            throw dptf_exception("Bad node type.");
    }
}