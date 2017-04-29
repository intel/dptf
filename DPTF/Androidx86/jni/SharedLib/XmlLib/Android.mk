################################################################################
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

DPTF_SRC = ../../../../Sources
DPTF_INC = $(LOCAL_PATH)/$(DPTF_SRC)

LOCAL_MODULE := DptfXmlLib
LOCAL_MODULE_OWNER := intel

LOCAL_C_INCLUDES := \
        prebuilts/ndk/current/sources/cxx-stl/llvm-libc++/include \
	$(DPTF_INC) \
	$(DPTF_INC)/../../Common \
	$(DPTF_INC)/SharedLib/BasicTypesLib \
	$(DPTF_INC)/SharedLib/XmlLib \
	$(DPTF_INC)/ThirdParty

LOCAL_CFLAGS += -std=c++11 -fexceptions -frtti -Wall -fPIC \
	-DESIF_ATTR_OS_ANDROID \
	-DESIF_ATTR_USER

FILE_LIST := $(wildcard $(LOCAL_PATH)/$(DPTF_SRC)/SharedLib/XmlLib/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_SDK_VERSION := 21
LOCAL_NDK_STL_VARIANT := c++_shared
include $(BUILD_STATIC_LIBRARY)
