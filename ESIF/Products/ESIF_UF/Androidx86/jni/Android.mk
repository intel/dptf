################################################################################
## Copyright (c) 2013 Intel Corporation All Rights Reserved
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

LOCAL_PATH := $(call my-dir)/../../..
include $(CLEAR_VARS)

LOCAL_CFLAGS    := -g -DESIF_ATTR_OS_ANDROID -DESIF_ATTR_DAEMON -DESIF_ATTR_USER -Wno-multichar -Wno-error=sequence-point
LOCAL_MODULE    := esif_ufd
LOCAL_SHARED_LIBRARIES := libdl liblog

LOCAL_C_INCLUDES := $(LOCAL_PATH)/ESIF_CM/Sources $(LOCAL_PATH)/ESIF_UF/Sources $(LOCAL_PATH)/ESIF_WS/Sources $(LOCAL_PATH)/ESIF_LIB/Sources

LOCAL_SRC_FILES := ESIF_UF/Sources/lin/main.c
LOCAL_SRC_FILES += ESIF_LF/Sources/lin/esif_ipc_os_lin.c

LOCAL_SRC_FILES += ESIF_CM/Sources/esif_debug.c
LOCAL_SRC_FILES += ESIF_CM/Sources/esif_ipc.c
LOCAL_SRC_FILES += ESIF_CM/Sources/esif_hash_table.c
LOCAL_SRC_FILES += ESIF_CM/Sources/esif_link_list.c

LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_action.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_action_config.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_action_constant.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_action_system.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_actmgr.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_app.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_appmgr.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_cnjmgr.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_conjure.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_dsp.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_dspmgr.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_event.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_ipc.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_pm.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_primitive.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_service.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_shell.c
LOCAL_SRC_FILES += ESIF_UF/Sources/esif_uf_test.c

LOCAL_SRC_FILES += ESIF_LIB/Sources/esif_lib_databank.c
LOCAL_SRC_FILES += ESIF_LIB/Sources/esif_lib_datacache.c
LOCAL_SRC_FILES += ESIF_LIB/Sources/esif_lib_datavault.c
LOCAL_SRC_FILES += ESIF_LIB/Sources/esif_lib_esifdata.c
LOCAL_SRC_FILES += ESIF_LIB/Sources/esif_lib_iostream.c
LOCAL_SRC_FILES += ESIF_LIB/Sources/esif_lib_istring.c

LOCAL_SRC_FILES += ESIF_WS/Sources/esif_ws_algo.c
LOCAL_SRC_FILES += ESIF_WS/Sources/esif_ws_http.c
LOCAL_SRC_FILES += ESIF_WS/Sources/esif_ws_server.c
LOCAL_SRC_FILES += ESIF_WS/Sources/esif_ws_socket.c

include $(BUILD_EXECUTABLE)
