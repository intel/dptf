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
#include "PolicyListCaptureDataGenerator.h"
#include "PolicyManagerInterface.h"
using namespace std;

PolicyListCaptureDataGenerator::PolicyListCaptureDataGenerator(DptfManagerInterface* dptfManager)
	: CaptureDataGenerator(dptfManager)
{

}

/*
Example:
<idsp>
	<fld>0F27BE63-111C-FD48-A6F7-3AF253FF3E2D</fld>
	<fld>78563412-BBAA-DDCC-EEFF-ABCDEF123456</fld>
</idsp>
*/

std::shared_ptr<XmlNode> PolicyListCaptureDataGenerator::generate() const
{
	const auto supportedPolicyList = m_dptfManager->getPolicyManager()->getSupportedPolicyList();
	const auto categoryRoot = XmlNode::createWrapperElement("idsp");

	for (const auto& policyGuid : supportedPolicyList->getGuids())
	{
		categoryRoot->addChild(XmlNode::createDataElement("fld", policyGuid.toClassicString()));
	}

	auto root = XmlNode::createRoot();
	root->addChild(categoryRoot);
	return root;
}