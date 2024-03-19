################################################################################
## Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

# Default Build Options unless overriden on command line: make PLAT=x86 BUILD=Debug OS=Chrome
PLAT  := x64
BUILD := Release
OS    := Linux

# Default compiler = gcc
ifneq ($(filter default undefined,$(origin CC)),)
CC=gcc
endif
AR := ar

IPF_SOURCES := ../Sources
ESIF_SDK_SOURCES := ../../Common

# TODO: Remove dependencies on these and move esif_lib_istring.c to /Common
ESIF_LIB_SOURCES := ../../ESIF/Products/ESIF_LIB/Sources
ESIF_CM_SOURCES := ../../ESIF/Products/ESIF_CM/Sources
ESIF_UF_SOURCES := ../../ESIF/Products/ESIF_UF/Sources

CPPFLAGS += -DESIF_ATTR_USER -DNO_IPFCORE_SUPPORT
CPPFLAGS += -I$(IPF_SOURCES)/Common
CPPFLAGS += -I$(ESIF_SDK_SOURCES)

# TODO: Remove dependencies on these and move esif_lib_istring.c to /Common
CPPFLAGS += -I$(ESIF_LIB_SOURCES)
CPPFLAGS += -I$(ESIF_CM_SOURCES)
CPPFLAGS += -I$(ESIF_UF_SOURCES)

CFLAGS += -Werror
CFLAGS += -Wno-multichar
CFLAGS += -Wno-typedef-redefinition

# Compile Flags added for security purposes
CFLAGS  += -fPIE
CFLAGS  += -fPIC
CFLAGS  += -O2 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2
CFLAGS  += -Wformat -Wformat-security -Werror

# Some flags are not directly compatible with Clang, but work out okay if the following flags are added
ifeq ($(CC), clang)
	CFLAGS += -Wno-ignored-optimization-argument -Wno-unused-command-line-argument
endif
CFLAGS  += -fno-strict-overflow
CFLAGS  += -fno-delete-null-pointer-checks
CFLAGS  += -fwrapv

# Linker Flags added for security purposes
LDFLAGS += -z noexecstack 
LDFLAGS += -z relro -z now 

# Executables only
LDFLAGS_EXE := -pie 

# Linker Libraries
LDLIBS := -ldl -lpthread

# Secure Compile options if GCC version >= 4.9
GCC_STRONG := $(shell expr `($(CC) -dumpfullversion 2>/dev/null || $(CC) -dumpversion) | cut -d'.' -f1,2,3 | sed -e 's/\.\([0-9]\)$$/0\1/' | sed -e 's/\.//g' ` \>= 409)
ifeq ($(GCC_STRONG), 1)
       CFLAGS += -fstack-protector-strong
else
       CFLAGS += -fstack-protector
endif

# Hide Non-Exported Symbols
CFLAGS += -fvisibility=hidden

# make OS=<Chrome|Linux>
ifeq ($(OS), Chrome)
	CPPFLAGS += -DESIF_ATTR_OS_CHROME
	CPPFLAGS += -std=gnu99
else
	CPPFLAGS += -DESIF_ATTR_OS_LINUX
	CPPFLAGS += -std=gnu99

	# make PLAT=<x64|x86>
	ifeq ($(PLAT), x64)
		CFLAGS += -m64
	endif
	ifeq ($(PLAT), x86)
		CFLAGS += -m32
		CFLAGS += -msse2
	endif

endif
#Compile with -g by default and strip it at the end
CFLAGS   += -g

# make BUILD=Debug
ifeq ($(BUILD), Debug)
	CFLAGS   += -DESIF_ATTR_DEBUG
endif

$(info ++++ )
$(info ++++ IPF $(PLAT) $(BUILD) $(OS))
$(info ++++ )

###############################################################################
# IPF Applications
###############################################################################

# ipfhostd
IPFHOSTD_OBJ := $(IPF_SOURCES)/ipfhostd/ipfhostd.o
IPFHOSTD_OBJ += $(IPF_SOURCES)/Common/ipf_apploader.o
IPFHOSTD_OBJ += $(IPF_SOURCES)/Common/ipf_trace.o
IPFHOSTD_OBJ += $(IPF_SOURCES)/ipfcorelib/ipf_dll.o
IPFHOSTD_OBJ += $(IPF_SOURCES)/ipfcorelib/ipf_dll_esif.o
IPFHOSTD_OBJ += $(IPF_SOURCES)/ipfcorelib/ipf_dll_ipc.o

# Common Shared Modules between Server and SDK
IPFSHARED_OBJ := $(ESIF_SDK_SOURCES)/esif_link_list.o
IPFSHARED_OBJ += $(IPF_SOURCES)/Common/ipf_lifecycle.o
IPFSHARED_OBJ += $(IPF_SOURCES)/Common/ipf_trace.o

# ipfsrv.so
IPFSRV_OBJ := $(IPFSHARED_OBJ)
IPFSRV_OBJ += $(ESIF_LIB_SOURCES)/esif_lib_istring.o
IPFSRV_OBJ += $(ESIF_SDK_SOURCES)/esif_sdk_base64_enc.o
IPFSRV_OBJ += $(IPF_SOURCES)/Common/ipf_ibinary.o
IPFSRV_OBJ += $(IPF_SOURCES)/Common/ipf_ipc_codec.o
IPFSRV_OBJ += $(IPF_SOURCES)/Common/ipf_ipc_trxmgr.o
IPFSRV_OBJ += $(IPF_SOURCES)/Common/ipf_ipc_clisrv.o
IPFSRV_OBJ += $(IPF_SOURCES)/ipfsrv/ipfsrv.o
IPFSRV_OBJ += $(IPF_SOURCES)/ipfsrv/ipfsrv_irpc.o
IPFSRV_OBJ += $(IPF_SOURCES)/ipfsrv/ipfsrv_ws_http.o
IPFSRV_OBJ += $(IPF_SOURCES)/ipfsrv/ipfsrv_ws_server.o
IPFSRV_OBJ += $(IPF_SOURCES)/ipfsrv/ipfsrv_ws_socket.o
IPFSRV_OBJ += $(IPF_SOURCES)/ipfsrv/ipfsrv_appmgr.o
IPFSRV_OBJ += $(IPF_SOURCES)/ipfsrv/ipfsrv_authmgr.o
IPFSRV_OBJ += $(IPF_SOURCES)/ipfsrv/ipfsrv_iface_cli.o
IPFSRV_OBJ += $(IPF_SOURCES)/ipfsrv/ipfsrv_iface_srv.o

# ipfipc.so
IPFIPC_OBJ := $(IPF_SOURCES)/Common/ipf_ibinary.o
IPFIPC_OBJ += $(IPF_SOURCES)/Common/ipf_ipc_codec.o
IPFIPC_OBJ += $(IPF_SOURCES)/Common/ipf_ipc_trxmgr.o
IPFIPC_OBJ += $(IPF_SOURCES)/Common/ipf_ipc_clisrv.o
IPFIPC_OBJ += $(IPF_SOURCES)/Common/ipf_trace.o
IPFIPC_OBJ += $(IPF_SOURCES)/ipfipc/ipfipc_irpc.o
IPFIPC_OBJ += $(IPF_SOURCES)/ipfipc/ipfipc_ws.o
IPFIPC_OBJ += $(IPF_SOURCES)/ipfipc/ipfipc_trace.o
IPFIPC_OBJ += $(ESIF_SDK_SOURCES)/esif_sdk_base64_enc.o

ifeq ($(BUILD), Debug)
	CFLAGS += -DESIF_ATTR_MEMTRACE
	IPFCORELIB_OBJ += $(IPF_SOURCES)/Common/ipf_memtrace.o
	IPFSRV_OBJ += $(IPF_SOURCES)/Common/ipf_memtrace.o
	IPFIPC_OBJ += $(IPF_SOURCES)/Common/ipf_memtrace.o
endif

generate_debug_file = false
ifeq ($(OS), Linux)
ifeq ($(BUILD), Release)
	generate_debug_file = true
endif
endif

define generate_debug
	cp $(1) $(1).debug
	strip --strip-unneeded $(1)
	objcopy --add-gnu-debuglink=$(1).debug $(1)
endef

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $(CPPFLAGS) $(EXTRA_CPPFLAGS) -o $@ $<

all: ipfsrv.so ipfipc.so ipfhostd

ipfsrv.so: $(IPFSRV_OBJ)
	$(CC) $(CFLAGS) -shared $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	$(if $(filter true,$(generate_debug_file)),$(call generate_debug,$@))

ipfipc.so: $(IPFIPC_OBJ)
	$(CC) $(CFLAGS) -shared $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	$(if $(filter true,$(generate_debug_file)),$(call generate_debug,$@))

ipfhostd: $(IPFHOSTD_OBJ)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) $(LDLIBS_EXE)
	$(if $(filter true,$(generate_debug_file)),$(call generate_debug,$@))

clean:
	rm -f $(IPFSRV_OBJ) $(IPFIPC_OBJ) $(IPFHOSTD_OBJ) ipf* *.debug
