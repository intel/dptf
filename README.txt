Intel (R) Dynamic Platform and Thermal Framework (Intel (R) DPTF) 
for Chromium OS - 8.1.x Release 

README

-------------------------------------------------------------------------------

This document describes how to build and integrate DPTF to Chromimum OS. Please
note that DPTF is already fully integrated to "rambi" and "strago" derived 
overlays by default if you build from the upstream code from chromium.org.
On overlays where DPTF is not enabled by default, you can use this guide and
follow the necessary steps to build and enable it.

This document assumes that the user has already set up the Chromium OS SDK on
their development machine. Otherwise, please refer to Google's "Chromium OS 
Developer Guide" available at:

	http://www.chromium.org/chromium-os/developer-guide

-------------------------------------------------------------------------------
MANUALLY INSTALL DPTF TO OVERLAYS
-------------------------------------------------------------------------------

Step 1 - Enter the Chromium OS SDK chroot environment by running the command:

	sudo <chromimumos root>/chromite/bin/cros_sdk

Step 2 - Check the kernel version of the overlay of interest. Currently DPTF
only supports 3.10 and 3.18 kernel on chromium.org. You must use either one 
in order to proceed further.

Step 3 - The following kernel config flags must be enabled/disabled in order
for DPTF to be functional.

For 3.18 kernel based overlays:
CONFIG_ACPI_THERMAL_REL=y
CONFIG_INT340X_THERMAL=y
CONFIG_INTEL_RAPL=y
CONFIG_INTEL_SOC_DTS_THERMAL=y
CONFIG_IOSF_MBI=y
CONFIG_POWERCAP=y

For 3.10 kernel based overlays:

# CONFIG_CPU_THERMAL is not set
CONFIG_INTEL_DPTF=m
CONFIG_INTEL_DPTF_ACPI=m
CONFIG_INTEL_DPTF_CPU=m	# Set only if overlay has a PCI CPU Reporting device
CONFIG_INTEL_DPTF_PCH=m	# Set only if overlay has a PCI PCH Reporting device
CONFIG_IOSF_MBI=y

Step 4 - Check if the overlay of interest has DPTF enabled or not. Edit the 
src/overlay/overlay-${BOARD}/make.conf and add this line if DPTF is not enabled
yet on this overlay:

USE="${USE} dptf"

Step 5 - Check if CMake has been installed on host Portage, and if not, run 
this command to install:

	sudo emerge cmake

Step 6 - Manually run emerge-${BOARD} to build and merge DPTF to target. Please
note that by default, the cross compiler disables C++ exception handling. You 
will have to pass the CXXFLAGS that enables C++ exception handling to ebuild:

	CXXFLAGS='-fexceptions' emerge-${BOARD} virtual/dptf

Step 7 - Build the target image and you should see DPTF integrated to the 
target image after the build is done. To build the target image, under the
src/scripts directory, run:

	./build_image --board=${BOARD} --noenable_rootfs_verification dev

-------------------------------------------------------------------------------
BUILD ARTIFACTS
-------------------------------------------------------------------------------
Emerge of DPTF will install the following components to the target file system:

/etc/init/dptf.conf	# This is the upstart DPTF service configuration file
/etc/dptf/dsp.dv	# This is the DPTF data vault file
/usr/bin/esif_ufd	# This is the main DPTF service executable
/usr/lib64/Dptf.so	# DPTF manager shared object
/usr/lib64/DptfPolicyCritical.so	# DPTF Critical Policy shared object
/usr/lib64/DptfPolicyPassive.so		# DPTF Passive Policy shared object
/usr/lib64/DptfPolicyActive.so	# (optional), DPTF Active Policy shared object
/usr/share/dptf/combined.xsl	# Various DPTF tables in XML format

Please note that if your overlay runs 3.10 kernel, you will also see the
following additional kernel modules installed under 
/lib/modules/$(uname -r)/kernel/drivers/platform/x66/intel_dptf/

esif_lf_driver.ko	# ESIF (Eco-System Independent Framework) driver
dptf_acpi.ko		# DPTF driver for ACPI participants
dptf_cpu.ko		# (optional depending on overlays) - DPTF CPU driver
dptf_pch.ko		# (optional depending on overlays) - DPTF PCH driver

-------------------------------------------------------------------------------
UNINSTALL DPTF
-------------------------------------------------------------------------------
Uninstalling DPTF is fairly straightforward, you just need to know the kernel
version of the overlay, and if it is 3.10, then type:

emerge-${BOARD} dptf-3_10 --unmerge

Otherwise, type:

emerge-${BOARD} dptf --unmerge

-------------------------------------------------------------------------------
VERIFY INSTALL/CONFIGURATION
-------------------------------------------------------------------------------
Upon system boot, you should be able to observe the esif_ufd process that has
been launched by upstart.

-------------------------------------------------------------------------------
KNOWN ISSUES / LIMITATIONS
-------------------------------------------------------------------------------
* This build may not correctly detect charger's AC/DC change event, and as a
result, DPTF may not be able to correctly throttle charger's charging rate when
necessary.

* Testing has only been performed on Intel BayTrail-M and Braswell based
Chromebook platforms and the 4th generation Intel® Core ™ processor based
development platforms using the UEFI BIOS.

* Compilation warnings will be noticed during the build for overlays that run
3.10 kernel. No warnings should be observed though for overlays that are based
on 3.18 kernel.
