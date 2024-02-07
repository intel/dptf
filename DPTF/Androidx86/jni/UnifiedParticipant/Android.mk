################################################################################
## Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

DPTF_SRC = ../../../Sources
DPTF_INC = $(LOCAL_PATH)/$(DPTF_SRC)

LOCAL_MODULE := DptfParticipant
LOCAL_MODULE_OWNER := intel

LOCAL_C_INCLUDES := \
        prebuilts/ndk/current/sources/cxx-stl/llvm-libc++/include \
	$(DPTF_INC) \
	$(DPTF_INC)/../../Common \
	$(DPTF_INC)/SharedLib \
	$(DPTF_INC)/SharedLib/BasicTypesLib \
	$(DPTF_INC)/SharedLib/DptfObjectsLib \
	$(DPTF_INC)/SharedLib/DptfTypesLib \
	$(DPTF_INC)/SharedLib/EsifTypesLib \
	$(DPTF_INC)/SharedLib/ParticipantControlsLib \
	$(DPTF_INC)/SharedLib/ParticipantLib \
	$(DPTF_INC)/SharedLib/EventsLib \
	$(DPTF_INC)/SharedLib/MessageLoggingLib \
	$(DPTF_INC)/SharedLib/XmlLib \
	$(DPTF_INC)/UnifiedParticipant \
	$(DPTF_INC)/ThirdParty

LOCAL_CFLAGS += -std=c++11 -fexceptions -frtti -Wall -fPIC -DESIF_ATTR_OS_ANDROID -DESIF_ATTR_USER \
	-Wno-error=non-virtual-dtor -Wno-ignored-qualifiers -Wno-unused-parameter -Wno-non-virtual-dtor

FILE_LIST := $(wildcard $(LOCAL_PATH)/$(DPTF_SRC)/UnifiedParticipant/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_SDK_VERSION := 21
LOCAL_NDK_STL_VARIANT := c++_shared
include $(BUILD_STATIC_LIBRARY)
