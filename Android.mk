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
INTEL_DPTF_ROOT_PATH:= $(call my-dir)
include $(INTEL_DPTF_ROOT_PATH)/ESIF/Products/ESIF_UF/Androidx86/jni/Android.mk
include $(INTEL_DPTF_ROOT_PATH)/ESIF/Products/ESIF_CMP/Android/Android.mk
include $(INTEL_DPTF_ROOT_PATH)/ESIF/Products/ESIF_WS/Android/Android.mk
include $(INTEL_DPTF_ROOT_PATH)/ESIF/Products/UPE_IOC/Android/Android.mk
include $(INTEL_DPTF_ROOT_PATH)/DPTF/Androidx86/jni/Android.mk
