#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

DEVICE=odin
VENDOR=xiaomi

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

function vendor_imports() {
    cat << EOF >> "$1"
		"device/xiaomi/odin",
		"hardware/qcom-caf/sm8350",
		"hardware/qcom-caf/wlan",
		"hardware/xiaomi",
		"vendor/qcom/opensource/commonsys/display",
		"vendor/qcom/opensource/commonsys-intf/display",
		"vendor/qcom/opensource/dataservices",
		"vendor/qcom/opensource/display",
EOF
}

function lib_to_package_fixup_vendor_variants() {
    if [ "$2" != "vendor" ]; then
        return 1
    fi

    case "$1" in
        com.qualcomm.qti.dpm.api@1.0 | \
            vendor.qti.hardware.fm@1.0 | \
            com.qualcomm.qti.imscmservice@1.0 | \
            com.qualcomm.qti.imscmservice@2.0 | \
            com.qualcomm.qti.imscmservice@2.1 | \
            com.qualcomm.qti.imscmservice@2.2 | \
            com.qualcomm.qti.uceservice@2.0 | \
            com.qualcomm.qti.uceservice@2.1 | \
            com.qualcomm.qti.uceservice@2.2 | \
            com.qualcomm.qti.uceservice@2.3 | \
            vendor.display.color@1.0 | \
            vendor.display.color@1.1 | \
            vendor.display.color@1.2 | \
            vendor.display.color@1.3 | \
            vendor.display.postproc@1.0 | \
            vendor.qti.data.factory@2.0 | \
            vendor.qti.data.factory@2.1 | \
            vendor.qti.data.factory@2.2 | \
            vendor.qti.data.factory@2.3 | \
            vendor.qti.data.mwqem@1.0 | \
            vendor.qti.data.slm@1.0 | \
            vendor.qti.diaghal@1.0 | \
            vendor.qti.hardware.data.cne.internal.api@1.0 | \
            vendor.qti.hardware.data.cne.internal.constants@1.0 | \
            vendor.qti.hardware.data.cne.internal.server@1.0 | \
            vendor.qti.hardware.data.connection@1.0 | \
            vendor.qti.hardware.data.connection@1.1 | \
            vendor.qti.hardware.data.dynamicdds@1.0 | \
            vendor.qti.hardware.data.iwlan@1.0 | \
            vendor.qti.hardware.data.latency@1.0 | \
            vendor.qti.hardware.data.lce@1.0 | \
            vendor.qti.hardware.data.qmi@1.0 | \
            vendor.qti.hardware.embmssl@1.0 | \
            vendor.qti.hardware.embmssl@1.1 | \
            vendor.qti.hardware.latency@2.0 | \
            vendor.qti.hardware.latency@2.1 | \
            vendor.qti.hardware.secureprocessor.common@1.0-helper | \
            vendor.qti.hardware.qccsyshal@1.0 | \
            vendor.qti.hardware.qccvndhal@1.0 | \
            vendor.qti.hardware.qconfig@1.0 | \
            vendor.qti.hardware.qdutils_disp@1.0 | \
            vendor.qti.hardware.qseecom@1.0 | \
            vendor.qti.hardware.qteeconnector@1.0 | \
            vendor.qti.hardware.secureprocessor.common@1.0 | \
            vendor.qti.hardware.secureprocessor.config@1.0 | \
            vendor.qti.hardware.secureprocessor.device@1.0 | \
            vendor.qti.hardware.slmadapter@1.0 | \
            vendor.qti.hardware.trustedui@1.0 | \
            vendor.qti.ims.callcapability@1.0 | \
            vendor.qti.ims.callinfo@1.0 | \
            vendor.qti.ims.factory@1.0 | \
            vendor.qti.ims.factory@1.1 | \
            vendor.qti.ims.rcsconfig@1.0 | \
            vendor.qti.ims.rcsconfig@1.1 | \
            vendor.qti.ims.rcsconfig@2.0 | \
            vendor.qti.ims.rcsconfig@2.1 | \
            vendor.qti.imsrtpservice@3.0 | \
            vendor.qti.latency@2.0 | \
            vendor.qti.qesdhal@1.0 | \
            vendor.qti.qesdhal@1.1 | \
            vendor.qti.voiceprint@1.0 | \
            vendor.xiaomi.hardware.campostproc@1.0)
            echo "$1_vendor"
            ;;
        libprotobuf-cpp-full)
            echo "libprotobuf-cpp-full-vendorcompat"
            ;;
        libprotobuf-cpp-lite)
            echo "libprotobuf-cpp-lite-vendorcompat"
            ;;
        *)
            return 1
            ;;
    esac
}

function lib_to_package_fixup() {
    lib_to_package_fixup_clang_rt_ubsan_standalone "$1" ||
        lib_to_package_fixup_proto_3_9_1 "$1" ||
        lib_to_package_fixup_vendor_variants "$@"
}

# Initialize the helper
setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}"

# Warning headers and guards
write_headers

write_makefiles "${MY_DIR}/proprietary-files.txt" true

# Finish
write_footers
