/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

namespace NodeType
{
	enum Type
	{
		Comment,
		Element,
		Root
	};
}

class XmlNode
{
public:
	~XmlNode(void);

	static std::shared_ptr<XmlNode> createRoot();
	static std::shared_ptr<XmlNode> createComment(std::string comment);
	static std::shared_ptr<XmlNode> createWrapperElement(std::string tagName);
	static std::shared_ptr<XmlNode> createDataElement(std::string tagName, std::string data);

	void addChild(std::shared_ptr<XmlNode> child);
	void addAttribute(std::string attribute);
	std::vector<std::shared_ptr<XmlNode>> getChildren();
	std::vector<std::string> getAttributes();
	std::string getXmlTag();
	std::string getData();
	NodeType::Type getNodeType();
	std::string toString();

private:
	XmlNode(NodeType::Type type, std::string rootXmlTag);
	XmlNode(NodeType::Type type, std::string rootXmlTag, std::string data);

	NodeType::Type m_type;
	std::string m_rootXmlTag;
	std::string m_data;
	std::vector<std::string> m_attributes;
	std::vector<std::shared_ptr<XmlNode>> m_children;
};
