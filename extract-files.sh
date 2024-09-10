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

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

KANG=
SECTION=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        -n | --no-cleanup )
                CLEAN_VENDOR=false
                ;;
        -k | --kang )
                KANG="--kang"
                ;;
        -s | --section )
                SECTION="${2}"; shift
                CLEAN_VENDOR=false
                ;;
        * )
                SRC="${1}"
                ;;
    esac
    shift
done

if [ -z "${SRC}" ]; then
    SRC="adb"
fi

function blob_fixup() {
    case "${1}" in
        system_ext/etc/permissions/vendor.qti.hardware.data.connection-V1.0-java.xml|system_ext/etc/permissions/vendor.qti.hardware.data.connection-V1.1-java.xml|system_ext/etc/permissions/vendor.qti.hardware.data.connectionaidl-V1-java.xml)
            sed -i 's/version="2.0"/version="1.0"/g' "${2}"
            sed -i 's/system\/product/system_ext/g' "${2}"
            ;;
        vendor/etc/camera/odin_motiontuning.xml)
            sed -i 's/xml=version/xml\ version/g' "${2}"
	    ;;
        vendor/etc/camera/pureShot_parameter.xml)
            sed -i 's/=\([0-9]\+\)>/="\1">/g' "${2}"
            ;;
        vendor/etc/media_codecs.xml|vendor/etc/media_codecs_lahaina.xml|vendor/etc/media_codecs_system_default_lahaina.xml)
            sed -Ei "/media_codecs_(google_audio|google_c2|google_telephony|vendor_audio)/d" "${2}"
            ;;
        vendor/etc/permissions/vendor-qti-hardware-sensorscalibrate.xml)
            sed -i 's/system/system_ext/g' "${2}"
            ;;
        vendor/etc/vintf/manifest/c2_manifest_vendor.xml)
            sed -ni '/ozoaudio/!p' "${2}"
            sed -ni '/dolby/!p' "${2}"
            ;;
        vendor/lib64/android.hardware.secure_element@1.0-impl.so)
            "${PATCHELF}" --remove-needed "android.hidl.base@1.0.so" "${2}"
            ;;
        vendor/lib64/hw/camera.xiaomi.so)
            hexdump -ve '1/1 "%.2X"' "${2}" | sed "s/50070094881640F9/1F2003D5881640F9/g" | xxd -r -p > "${EXTRACT_TMP_DIR}/${1##*/}"
            mv "${EXTRACT_TMP_DIR}/${1##*/}" "${2}"
            ;;
        vendor/lib64/hw/com.qti.chi.override.so)
            sed -i 's/\/system\/lib64\/libion.so/\/vendor\/lib64\/libion.so/g' "${2}"
            ;;
        vendor/lib64/libwa_sat.so)
            sed -i 's/\/system\/lib64\([^\/]\)/\/vendor\/lib64\1/g' "${2}"
            ;;
        vendor/lib64/libwvhidl.so)
            grep -q libcrypto_shim.so "${2}" || "${PATCHELF}" --add-needed "libcrypto_shim.so" "${2}"
            ;;
        vendor/lib64/vendor.xiaomi.hardware.cameraperf@1.0-impl.so)
            hexdump -ve '1/1 "%.2X"' "${2}" | sed "s/7C000094881640F9/1F2003D5881640F9/g" | xxd -r -p > "${EXTRACT_TMP_DIR}/${1##*/}"
            mv "${EXTRACT_TMP_DIR}/${1##*/}" "${2}"
            ;;
    esac
}

# Initialize the helper
setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" false "${CLEAN_VENDOR}"

extract "${MY_DIR}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"

"${MY_DIR}/setup-makefiles.sh"
