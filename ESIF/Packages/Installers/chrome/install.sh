#!/bin/bash

DPTF_ROOT=/usr/share/dptf
DPTF_ETC=/etc/dptf
LIBRARY_PATH=/usr/lib64

# This will be integrated into future OTC/Google builds
cp drivers/iosf_mbi.ko /lib/modules/`uname -r`/extra
cp drivers/esif_lf.ko /lib/modules/`uname -r`/extra
cp drivers/dptf_acpi.ko /lib/modules/`uname -r`/extra
cp drivers/dptf_cpu.ko /lib/modules/`uname -r`/extra
cp drivers/dptf_pch.ko /lib/modules/`uname -r`/extra

depmod -a

if [ ! -d "$DPTF_ETC" ]; then
	mkdir $DPTF_ETC
	mkdir $DPTF_ETC/dsp
	mkdir $DPTF_ETC/cmd
fi

if [ ! -d "$DPTF_ROOT" ]; then
	mkdir $DPTF_ROOT
	mkdir $DPTF_ROOT/ui
fi

# Create startup ESIF script
#echo "appstart Dptf" >> $DPTF_ETC/cmd/start

# Copy DSPs
cp dsp/*.edp $DPTF_ETC/dsp

# Copy Policy SOs
cp ufx64/DptfPolicyCritical.so $DPTF_ROOT
cp ufx64/DptfPolicyPassive.so $DPTF_ROOT

# Copy DPTF
cp ufx64/Dptf.so $LIBRARY_PATH/libdptf.so

# The following will create a symlink to Dptf.so in the lib folder,
# and rebuild the cache
ldconfig

# Copy index.html
cp index.html $DPTF_ROOT/ui

# Copy combined.xsl
cp ufx64/combined.xsl $DPTF_ROOT

# Copy ESIF executable
cp ufx64/esif_ufd /usr/bin

# Move initscript
cp dptf.conf /etc/init/

echo "Installation complete!"
echo "Please reboot the system if there was an older build of DPTF previously installed."
