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
ESIF_ROOT := $(LOCAL_PATH)/../..

include $(CLEAR_VARS)

LOCAL_CFLAGS    := -g -DESIF_ATTR_DEBUG -DESIF_ATTR_OS_ANDROID -DESIF_FEAT_OPT_ACTION_SYSFS -DESIF_ATTR_DAEMON -DESIF_ATTR_USER -Wno-multichar -Wno-error=sequence-point -Wno-unused-parameter -Wno-comment -Wno-missing-field-initializers
LOCAL_MODULE    := upe_java
LOCAL_MODULE_OWNER := intel
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SHARED_LIBRARIES := libdl liblog libcutils libicuuc libicui18n libbinder libutils

LOCAL_C_INCLUDES += \
	$(ESIF_ROOT)/ESIF_CM/Sources \
	$(ESIF_ROOT)/ESIF_UF/Sources \
	$(ESIF_ROOT)/ESIF_WS/Sources \
	$(ESIF_ROOT)/ESIF_LIB/Sources \
	$(ESIF_ROOT)/JHS/include \
	$(ESIF_ROOT)/../../Common

LOCAL_SRC_FILES := ../Sources/upe_iface.cpp
LOCAL_SRC_FILES += ../Sources/upe_trace.cpp
LOCAL_SRC_FILES += ../Sources/jhs_binder_client.cpp
LOCAL_SRC_FILES += ../Sources/jhs_binder_server.cpp
LOCAL_SRC_FILES += ../Sources/cnjr_jhs_iface.cpp
LOCAL_SRC_FILES += ../../ESIF_LIB/Sources/esif_lib_esifdata.c

include $(BUILD_SHARED_LIBRARY)
