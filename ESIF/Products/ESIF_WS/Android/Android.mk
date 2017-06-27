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

LOCAL_PATH := $(call my-dir)/../..

include $(CLEAR_VARS)
LOCAL_CFLAGS    := -g -DESIF_ATTR_OS_ANDROID -DESIF_ATTR_USER -Wno-error=sequence-point
LOCAL_MODULE    := esif_ws
LOCAL_MODULE_OWNER := intel
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SHARED_LIBRARIES := libdl liblog libcutils

LOCAL_C_INCLUDES := $(LOCAL_PATH)/ESIF_WS/Sources $(LOCAL_PATH)/../../Common

LOCAL_SRC_FILES := ESIF_WS/Sources/esif_ws.c
LOCAL_SRC_FILES += ESIF_WS/Sources/esif_ws_algo.c
LOCAL_SRC_FILES += ESIF_WS/Sources/esif_ws_http.c
LOCAL_SRC_FILES += ESIF_WS/Sources/esif_ws_server.c
LOCAL_SRC_FILES += ESIF_WS/Sources/esif_ws_socket.c

include $(BUILD_SHARED_LIBRARY)
