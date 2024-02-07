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

#include "XmlNode.h"
#include "StringParser.h"

using namespace std;

XmlNode::XmlNode(NodeType::Type type, std::string tag)
	: m_type(type)
	, m_tag(tag)
	, m_data("")
	, m_children()
{
}

XmlNode::XmlNode(NodeType::Type type, std::string tag, std::string data)
	: m_type(type)
	, m_tag(tag)
	, m_data(data)
	, m_children()
{
}

XmlNode::~XmlNode(void)
{
}

std::shared_ptr<XmlNode> XmlNode::createRoot()
{
	return std::make_shared<XmlNode>(XmlNode(NodeType::Root, ""));
}

std::shared_ptr<XmlNode> XmlNode::createComment(const std::string& comment)
{
	return std::make_shared<XmlNode>(XmlNode(NodeType::Comment, "", comment));
}

std::shared_ptr<XmlNode> XmlNode::createWrapperElement(const std::string& tag)
{
	return std::make_shared<XmlNode>(XmlNode(NodeType::Element, tag));
}

std::shared_ptr<XmlNode> XmlNode::createDataElement(const std::string& tag, const std::string& data)
{
	return std::make_shared<XmlNode>(XmlNode(NodeType::Element, tag, data));
}

void XmlNode::addChild(std::shared_ptr<XmlNode> child)
{
	m_children.push_back(child);
}

std::vector<std::shared_ptr<XmlNode>> XmlNode::getChildren()
{
	return m_children;
}

NodeType::Type XmlNode::getNodeType()
{
	return m_type;
}

std::string XmlNode::getTag()
{
	return m_tag;
}

std::string XmlNode::getData()
{
	return m_data;
}

std::string tab(UInt32 numberOfTabs)
{
	string value;
	value.insert(0, numberOfTabs, '\t');
	return value;
}

std::string tagOpen(const std::string& name)
{
	stringstream stream;
	stream << "<" << name << ">";
	return stream.str();
}

std::string tagClose(const std::string& name)
{
	stringstream stream;
	stream << "</" << name << ">";
	return stream.str();
}

std::string tagOpenAndClose(const std::string& name)
{
	stringstream stream;
	stream << "<" << name << "/>";
	return stream.str();
}

std::string newLine()
{
	return "\n";
}

std::string commentOpen()
{
	return "<!-- ";
}

std::string commentClose()
{
	return " -->";
}

std::string sanitizeData(const std::string& input)
{
	string modified = input;
	modified = StringParser::replaceAll(modified, "&", "&amp;");
	modified = StringParser::replaceAll(modified, "<", "&lt;");
	modified = StringParser::replaceAll(modified, ">", "&gt;");
	modified = StringParser::replaceAll(modified, "'", "&apos;");
	modified = StringParser::replaceAll(modified, "\"", "&quot;");
	modified = StringParser::removeCharacter(modified, '\0');
	return modified;
}

std::string XmlNode::toString(UInt8 tabDepth)
{
	stringstream stream;

	switch (m_type)
	{
	case NodeType::Root:
		stream << generateXmlForChildren(tabDepth);
		break;
	case NodeType::Element:
		stream << generateXmlForElement(tabDepth);
		break;
	case NodeType::Comment:
		stream << generateXmlForComment(tabDepth);
		break;
	default:
		break;
	}

	return stream.str();
}

std::string XmlNode::generateXmlForElement(UInt8 tabDepth)
{
	stringstream stream;
	if (hasNoData())
	{
		if (hasNoChildren())
		{
			stream << tab(tabDepth) << tagOpenAndClose(m_tag);
		}
		else
		{
			stream << tab(tabDepth) << tagOpen(m_tag) << newLine();
			stream << generateXmlForChildren(tabDepth + 1);
			stream << tab(tabDepth) << tagClose(m_tag);
		}
	}
	else
	{
		stream << tab(tabDepth) << tagOpen(m_tag) << sanitizeData(m_data) << tagClose(m_tag);
	}
	return stream.str();
}

std::string XmlNode::generateXmlForComment(UInt8 tabDepth)
{
	stringstream stream;
	stream << tab(tabDepth) << commentOpen() << sanitizeData(m_data) << commentClose();
	return stream.str();
}

std::string XmlNode::generateXmlForChildren(UInt8 tabDepth)
{
	stringstream stream;
	for (auto child = m_children.begin(); child != m_children.end(); ++child)
	{
		stream << (*child)->toString(tabDepth) << newLine();
	}
	return stream.str();
}

bool XmlNode::hasNoChildren() const
{
	return m_children.empty();
}

bool XmlNode::hasNoData() const
{
	return m_data.empty();
}
