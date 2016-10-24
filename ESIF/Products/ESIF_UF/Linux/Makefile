################################################################################
## Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

# Default Build Options unless overriden on command line: make PLAT=x86 BUILD=Debug OS=Chrome ...
# OPT_GMIN=1 = Use Upstreamed GMIN (SYSFS) Drivers [required for Chrome/Android and Default for Generic Linux]
# OPT_GMIN=0 = Use Out of Tree ESIF_LF Drivers [optional for Generic Linux]
PLAT      := x64
BUILD     := Release
OS        := Linux
OPT_GMIN  := 1
OPT_DBUS  := 0

ifneq ($(filter default undefined,$(origin CC)),)
CC := gcc
endif

ESIF_SOURCES := ../..
ESIF_UF_SOURCES := $(ESIF_SOURCES)/ESIF_UF/Sources
ESIF_CM_SOURCES := $(ESIF_SOURCES)/ESIF_CM/Sources
ESIF_WS_SOURCES := $(ESIF_SOURCES)/ESIF_WS/Sources
ESIF_LIB_SOURCES := $(ESIF_SOURCES)/ESIF_LIB/Sources
ESIF_SDK_SOURCES := $(ESIF_SOURCES)/../../Common

LDLIBS := -lpthread -ldl -lrt -lreadline

CPPFLAGS += -I$(ESIF_UF_SOURCES) -I$(ESIF_CM_SOURCES) -I$(ESIF_WS_SOURCES) -I$(ESIF_LIB_SOURCES) -I$(ESIF_SDK_SOURCES)
CPPFLAGS += -DESIF_ATTR_USER 
CPPFLAGS += -DESIF_ATTR_DAEMON

CFLAGS  += -Wno-multichar
CFLAGS  += -Werror

# Compile Flags added for security purposes
CFLAGS  += -fPIE
CFLAGS  += -fPIC
CFLAGS  += -O2 -D_FORTIFY_SOURCE=2
CFLAGS  += -Wformat -Wformat-security

# Linker Flags added for security purposes
LDFLAGS += -z noexecstack 
LDFLAGS += -z relro -z now 
LDFLAGS += -pie 

# Secure Compile options if GCC version >= 4.9
GCC_STRONG := $(shell expr `$(CC) -dumpversion | cut -d'.' -f1,2 | sed -e 's/\.\([0-9]\)$$/0\1/' | sed -e 's/\.//' ` \>= 409)
ifeq ($(GCC_STRONG), 1)
       CFLAGS += -fstack-protector-strong
else
       CFLAGS += -fstack-protector
endif


# make OS=<Chrome|Linux>
ifeq ($(OS), Android)
$(error For Android build please use Android.mk in ESIF_UF/Androidx86/jni directory)
else ifeq ($(OS), Chrome)
	CPPFLAGS += -DESIF_ATTR_OS_CHROME
	CPPFLAGS += -DESIF_ATTR_OS_LINUX_HAVE_READLINE 
ifeq ($(OPT_GMIN), 1)
	OPT_DBUS = 1
endif
else
	CPPFLAGS += -DESIF_ATTR_OS_LINUX
	CPPFLAGS += -DESIF_ATTR_OS_LINUX_HAVE_READLINE 

	# make PLAT=<x64|x86>
	ifeq ($(PLAT), x64)
		CFLAGS += -m64
	endif
	ifeq ($(PLAT), x86)
		CFLAGS += -m32
	endif
endif

# make OPT_GMIN=1
ifeq ($(OPT_GMIN), 1)
	CPPFLAGS += -DESIF_FEAT_OPT_ACTION_SYSFS
endif

# make OPT_DBUS=1
ifeq ($(OPT_DBUS), 1)
	# Enable D-Bus
	PKG_CONFIG ?= pkg-config
	CPPFLAGS += -DESIF_FEAT_OPT_DBUS
	CFLAGS += $(shell $(PKG_CONFIG) --cflags dbus-1)
	LDFLAGS += $(shell $(PKG_CONFIG) --libs dbus-1)
endif

# make BUILD=Debug
ifeq ($(BUILD), Debug)
	CFLAGS   += -g
	CPPFLAGS += -DESIF_ATTR_DEBUG
	CPPFLAGS += -DESIF_ATTR_MEMTRACE
endif

$(info ++++ )
$(info ++++ ESIF_UF $(PLAT) $(BUILD) $(OS) GMIN=$(OPT_GMIN))
$(info ++++ )

###############################################################################
# ESIF_UF 
###############################################################################

# OS Specific
OBJ := $(ESIF_UF_SOURCES)/lin/main.o
OBJ += $(ESIF_UF_SOURCES)/lin/esif_uf_ipc_os_lin.o
OBJ += $(ESIF_UF_SOURCES)/lin/esif_uf_action_sysfs_os_lin.o
OBJ += $(ESIF_UF_SOURCES)/lin/esif_uf_sysfs_enumerate_os_lin.o

# Common Source 
OBJ += $(ESIF_CM_SOURCES)/esif_ipc.o
OBJ += $(ESIF_CM_SOURCES)/esif_hash_table.o
OBJ += $(ESIF_CM_SOURCES)/esif_queue.o

# SDK Source
OBJ += $(ESIF_SDK_SOURCES)/esif_link_list.o
OBJ += $(ESIF_SDK_SOURCES)/esif_ccb_timer.o

OBJ += $(ESIF_UF_SOURCES)/esif_uf.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_action.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_action_config.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_action_constant.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_action_delegate.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_action_system.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_actmgr.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_app.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_appmgr.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_cnjmgr.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_conjure.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_control_action_broadcast.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_domain.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_dsp.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_dspmgr.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_event.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_eventmgr.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_ipc.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_loggingmgr.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_pm.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_primitive.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_service.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_shell.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_tableobject.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_trace.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_participant.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_xform.o

OBJ += $(ESIF_LIB_SOURCES)/esif_lib_databank.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_datacache.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_datavault.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_esifdata.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_iostream.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_istring.o

OBJ += $(ESIF_WS_SOURCES)/esif_ws_algo.o
OBJ += $(ESIF_WS_SOURCES)/esif_ws_http.o
OBJ += $(ESIF_WS_SOURCES)/esif_ws_server.o
OBJ += $(ESIF_WS_SOURCES)/esif_ws_socket.o


###############################################################################
# BUILD 
###############################################################################

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $(CPPFLAGS) -o $@ $<

all: esif_ufd

esif_ufd: $(OBJ)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) 

clean:
	rm -f $(OBJ) esif_ufd
