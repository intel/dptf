################################################################################
## Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
##
## The source code, information and material ("Material") contained herein is
## owned by Intel Corporation or its suppliers or licensors, and title to such
## Material remains with Intel Corporation or its suppliers or licensors. The
## Material contains proprietary information of Intel or its suppliers and
## licensors. The Material is protected by worldwide copyright laws and treaty
## provisions. No part of the Material may be used, copied, reproduced,
## modified, published, uploaded, posted, transmitted, distributed or disclosed
## in any way without Intel's prior express written permission. No license
## under any patent, copyright or other intellectual property rights in the
## Material is granted to or conferred upon you, either expressly, by
## implication, inducement, estoppel or otherwise. Any license under such
## intellectual property rights must be express and approved by Intel in
## writing.
##
## Copyright (C) 2006 The Android Open Source Project
## 
## Unless otherwise agreed by Intel in writing, you may not remove or alter
## this notice or any other notice embedded in Materials by Intel or Intel’s
## suppliers or licensors in any way.
##
## Licensed under the Apache License, Version 2.0 (the "License"); you may not 
## use this file except in compliance with the License.
##
## You may obtain a copy of the License at
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
## WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## 
## See the License for the specific language governing permissions and
## limitations under the License.
##
################################################################################
TOP_PATH := $(call my-dir)

include $(TOP_PATH)/SharedLib/Android.mk
include $(TOP_PATH)/SharedLib/BasicTypesLib/Android.mk
include $(TOP_PATH)/SharedLib/DptfObjectsLib/Android.mk
include $(TOP_PATH)/SharedLib/DptfTypesLib/Android.mk
include $(TOP_PATH)/SharedLib/EsifTypesLib/Android.mk
include $(TOP_PATH)/SharedLib/EventsLib/Android.mk
include $(TOP_PATH)/SharedLib/MessageLoggingLib/Android.mk
include $(TOP_PATH)/SharedLib/ParticipantControlsLib/Android.mk
include $(TOP_PATH)/SharedLib/ParticipantLib/Android.mk
include $(TOP_PATH)/SharedLib/XmlLib/Android.mk
include $(TOP_PATH)/SharedLib/ResourceLib/Android.mk
include $(TOP_PATH)/PolicyLib/Android.mk
include $(TOP_PATH)/UnifiedParticipant/Android.mk
include $(TOP_PATH)/Policies/ActivePolicy/Android.mk
#include $(TOP_PATH)/Policies/ConditionalLogicLib/Android.mk
#include $(TOP_PATH)/Policies/AdaptivePerformancePolicy/Android.mk
include $(TOP_PATH)/Policies/CriticalPolicy/Android.mk
#include $(TOP_PATH)/Policies/EmergencyCallModePolicy/Android.mk
#include $(TOP_PATH)/Policies/PassivePolicy2/Android.mk
#include $(TOP_PATH)/Policies/VirtualSensorPolicy/Android.mk
include $(TOP_PATH)/Manager/Android.mk
include $(TOP_PATH)/Resources/Android.mk
