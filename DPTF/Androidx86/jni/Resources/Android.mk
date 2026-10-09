################################################################################
## Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
LOCAL_PATH := $(call my-dir)/../SharedLib/

# Install libc++_shared.so
include $(CLEAR_VARS)
LOCAL_MODULE := libc++_shared.so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
# 64b-specific C++ STL libraries:
LOCAL_SRC_FILES_64 := x86_64/libc++_shared.so
# 32b-specific C++ STL libraries:
LOCAL_SRC_FILES_32 := x86/libc++_shared.so
LOCAL_MULTILIB := both
include $(BUILD_PREBUILT)
