/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
	virtual ~XmlNode(void);

	static std::shared_ptr<XmlNode> createRoot();
	static std::shared_ptr<XmlNode> createComment(const std::string& comment);
	static std::shared_ptr<XmlNode> createWrapperElement(const std::string& tag);
	static std::shared_ptr<XmlNode> createDataElement(const std::string& tag, const std::string& data);

	void addChild(std::shared_ptr<XmlNode> child);
	std::vector<std::shared_ptr<XmlNode>> getChildren();
	std::string getTag();
	std::string getData();
	NodeType::Type getNodeType();
	std::string toString(UInt8 tabDepth = 0);

private:
	XmlNode(NodeType::Type type, std::string tag);
	XmlNode(NodeType::Type type, std::string tag, std::string data);
	std::string generateXmlForChildren(UInt8 tabDepth);
	std::string generateXmlForComment(UInt8 tabDepth);
	std::string generateXmlForElement(UInt8 tabDepth);
	bool hasNoData() const;
	bool hasNoChildren() const;

	NodeType::Type m_type;
	std::string m_tag;
	std::string m_data;
	std::vector<std::shared_ptr<XmlNode>> m_children;
};
