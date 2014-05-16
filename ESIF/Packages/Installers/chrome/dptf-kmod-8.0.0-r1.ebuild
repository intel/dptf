# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="4"
EGIT_REPO_URI="git://github.com/01org/dptf.git"
EGIT_COMMIT="3d767c13b9567a2106691f86519c205874422fcd"

inherit git-2 toolchain-funcs

DESCRIPTION="Kernel modules for Dynamic Platform & Thermal Framework"
HOMEPAGE=""

LICENSE="GPLv2 or BSD"
SLOT="0"
KEYWORDS="amd64"
IUSE="kernel-next kernel-3_10 kernel-3_8"

RDEPEND="
kernel-3_10? ( sys-kernel/chromeos-kernel-3_10[kernel_sources] )
kernel-next? ( sys-kernel/chromeos-kernel-next[kernel_sources] )
kernel-3_8? ( sys-kernel/chromeos-kernel-next[kernel_sources] )"
KERNEL_DIR="${ROOT}usr/src/linux/build"
ESIF_LF_SRC_DIR=Products/ESIF_LF/Linuxx64Atom/Release
KERNBOOTDIR="${ROOT}boot"

update_depmod() {
	local tempvar=`emake -C "${KERNEL_DIR}" kernelrelease`
	local kernelrelease=`echo ${tempvar} | awk '{print $5}'`
	if [ -r "${KERNBOOTDIR}"/System.map-"${kernelrelease}" ]
		then
			depmod -ae -F "${KERNBOOTDIR}"/System.map-"${kernelrelease}" -b "${ROOT}" ${kernelrelease}
			eend $?
		else
			ewarn
			ewarn "${KERNBOOTDIR}/System.map not found."
			ewarn "You must manually update the kernel module dependencies using depmod."
			eend 1
			ewarn
	fi
}

src_compile() {
	local kernel_arch=${CHROMEOS_KERNEL_ARCH:-$(tc-arch-kernel)}
	cd ${ESIF_LF_SRC_DIR}
	export KERNEL_DIR=${KERNEL_DIR}
	export ARCH=${kernel_arch}
	emake || die "Make failed!"
}

src_install() {
	cd ${ESIF_LF_SRC_DIR}
	emake -C "${KERNEL_DIR}" INSTALL_MOD_PATH="${D}" M=`pwd` modules_install
#	local tempvar=`emake -C "${KERNEL_DIR}" kernelrelease`
#	kernelrelease=`echo ${tempvar} | awk '{print $5}'`
#	echo "Hello BEGIN"
#	echo "Hello ${kernelrelease}"
#	echo "Hello END"
	update_depmod
}
