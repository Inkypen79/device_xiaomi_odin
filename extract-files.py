#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixup_remove,
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/xiaomi/odin',
    'hardware/qcom-caf/sm8350',
    'hardware/qcom-caf/wlan',
    'hardware/xiaomi',
    'vendor/qcom/opensource/commonsys/display',
    'vendor/qcom/opensource/commonsys-intf/display',
    'vendor/qcom/opensource/dataservices',
    'vendor/qcom/opensource/display',
]


def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'vendor' else None


lib_fixups: lib_fixups_user_type = {
    **lib_fixups,
    (
        'com.qualcomm.qti.dpm.api@1.0',
        'vendor.qti.hardware.fm@1.0'
        'com.qualcomm.qti.imscmservice@1.0'
        'com.qualcomm.qti.imscmservice@2.0'
        'com.qualcomm.qti.imscmservice@2.1'
        'com.qualcomm.qti.imscmservice@2.2'
        'com.qualcomm.qti.uceservice@2.0'
        'com.qualcomm.qti.uceservice@2.1'
        'com.qualcomm.qti.uceservice@2.2'
        'com.qualcomm.qti.uceservice@2.3'
        'vendor.display.color@1.0'
        'vendor.display.color@1.1'
        'vendor.display.color@1.2'
        'vendor.display.color@1.3'
        'vendor.display.postproc@1.0'
        'vendor.qti.data.factory@2.0'
        'vendor.qti.data.factory@2.1'
        'vendor.qti.data.factory@2.2'
        'vendor.qti.data.factory@2.3'
        'vendor.qti.data.mwqem@1.0'
        'vendor.qti.data.slm@1.0'
        'vendor.qti.diaghal@1.0'
        'vendor.qti.hardware.data.cne.internal.api@1.0'
        'vendor.qti.hardware.data.cne.internal.constants@1.0'
        'vendor.qti.hardware.data.cne.internal.server@1.0'
        'vendor.qti.hardware.data.connection@1.0'
        'vendor.qti.hardware.data.connection@1.1'
        'vendor.qti.hardware.data.dynamicdds@1.0'
        'vendor.qti.hardware.data.iwlan@1.0'
        'vendor.qti.hardware.data.latency@1.0'
        'vendor.qti.hardware.data.lce@1.0'
        'vendor.qti.hardware.data.qmi@1.0'
        'vendor.qti.hardware.embmssl@1.0'
        'vendor.qti.hardware.embmssl@1.1'
        'vendor.qti.hardware.latency@2.0'
        'vendor.qti.hardware.latency@2.1'
        'vendor.qti.hardware.secureprocessor.common@1.0-helper'
        'vendor.qti.hardware.qccsyshal@1.0'
        'vendor.qti.hardware.qccvndhal@1.0'
        'vendor.qti.hardware.qconfig@1.0'
        'vendor.qti.hardware.qdutils_disp@1.0'
        'vendor.qti.hardware.qseecom@1.0'
        'vendor.qti.hardware.qteeconnector@1.0'
        'vendor.qti.hardware.secureprocessor.common@1.0'
        'vendor.qti.hardware.secureprocessor.config@1.0'
        'vendor.qti.hardware.secureprocessor.device@1.0'
        'vendor.qti.hardware.slmadapter@1.0'
        'vendor.qti.hardware.trustedui@1.0'
        'vendor.qti.ims.callcapability@1.0'
        'vendor.qti.ims.callinfo@1.0'
        'vendor.qti.ims.factory@1.0'
        'vendor.qti.ims.factory@1.1'
        'vendor.qti.ims.rcsconfig@1.0'
        'vendor.qti.ims.rcsconfig@1.1'
        'vendor.qti.ims.rcsconfig@2.0'
        'vendor.qti.ims.rcsconfig@2.1'
        'vendor.qti.imsrtpservice@3.0'
        'vendor.qti.latency@2.0'
        'vendor.qti.qesdhal@1.0'
        'vendor.qti.qesdhal@1.1'
        'vendor.qti.voiceprint@1.0'
    ): lib_fixup_vendor_suffix,
}

blob_fixups: blob_fixups_user_type = {
    ('system_ext/etc/permissions/vendor.qti.hardware.data.connection-V1.0-java.xml', 'system_ext/etc/permissions/vendor.qti.hardware.data.connection-V1.1-java.xml', 'system_ext/etc/permissions/vendor.qti.hardware.data.connectionaidl-V1-java.xml'): blob_fixup()
        .regex_replace('system/product', 'system_ext')
        .regex_replace('xml version="2.0"', 'xml version="1.0"'),
    'system_ext/lib64/libsystemhelper_jni.so': blob_fixup()
        .add_needed('libgui_shim.so'),
    'system_ext/lib64/vendor.qti.hardware.qccsyshal@1.2-halimpl.so': blob_fixup()
        .replace_needed ('libprotobuf-cpp-full.so', 'libprotobuf-cpp-full-21.7.so'),
    'system_ext/lib64/vendor.qti.hardware.qxr-V1-ndk_platform.so': blob_fixup()
        .replace_needed('android.hardware.common-V2-ndk_platform.so', 'android.hardware.common-V2-ndk.so'),
    'vendor/etc/camera/odin_motiontuning.xml': blob_fixup()
        .regex_replace('xml=version', 'xml version'),
    'vendor/etc/camera/pureShot_parameter.xml': blob_fixup()
        .regex_replace(r'=(\d+)>', r'="\1">'),
    ('vendor/etc/media_codecs.xml', 'vendor/etc/media_codecs_lahaina.xml', 'vendor/etc/media_codecs_system_default_lahaina.xml'): blob_fixup()
        .regex_replace('.+media_codecs_(c2_audio|google_audio|google_c2|google_telephony|vendor_audio).+\n', '')
        .regex_replace('\n        <Setting name="max-video-encoder-input-buffers" value="11" />', '\n        <Domain name="telephony" enabled="true" />\n        <Setting name="max-video-encoder-input-buffers" value="11" />'),
    'vendor/etc/permissions/vendor-qti-hardware-sensorscalibrate.xml': blob_fixup()
        .regex_replace('system', 'system_ext'),
    'vendor/etc/seccomp_policy/atfwd@2.0.policy': blob_fixup()
        .add_line_if_missing('gettid: 1'),
    'vendor/etc/vintf/manifest/c2_manifest_vendor.xml': blob_fixup()
        .regex_replace('.*ozoaudio.*\n?', '')
        .regex_replace('.*dolby.*\n?', ''),
    ('vendor/lib/hw/audio.primary.lahaina.so', 'vendor/lib/libaudioroute_ext.so'): blob_fixup()
        .replace_needed('libaudioroute.so', 'libaudioroute-v34.so'),
    ('vendor/lib64/hw/camera.qcom.so', 'vendor/lib64/libmialgoengine.so'): blob_fixup()
        .add_needed('libprocessgroup_shim.so'),
    'vendor/lib64/hw/camera.xiaomi.so': blob_fixup()
        .sig_replace('50 07 00 94 88 16 40 F9', '1F 20 03 D5 88 16 40 F9'),
    'vendor/lib64/hw/com.qti.chi.override.so': blob_fixup()
        .binary_regex_replace(b'/system/lib64/libion.so', b'/vendor/lib64/libion.so')
        .add_needed ('libprocessgroup_shim.so'),
    ('vendor/lib64/mediadrm/libwvdrmengine.so', 'vendor/lib64/libwvhidl.so'): blob_fixup()
        .add_needed('libcrypto_shim.so'),
    'vendor/lib64/android.hardware.secure_element@1.0-impl.so': blob_fixup()
        .remove_needed('android.hidl.base@1.0.so'),
    ('vendor/lib64/libalAILDC.so', 'vendor/lib64/libalLDC.so', 'vendor/lib64/libalhLDC.so', 'vendor/lib64/libcup_preview.so'): blob_fixup()
        .clear_symbol_version('AHardwareBuffer_allocate')
        .clear_symbol_version('AHardwareBuffer_describe')
        .clear_symbol_version('AHardwareBuffer_lock')
        .clear_symbol_version('AHardwareBuffer_release')
        .clear_symbol_version('AHardwareBuffer_unlock'),
    ('vendor/lib64/libarcsoft_hdrplus_hvx_stub.so','vendor/lib64/libarcsoft_super_night_raw.so', 'vendor/lib64/libhap_power.so'): blob_fixup()
        .clear_symbol_version('remote_handle_close')
        .clear_symbol_version('remote_handle_invoke')
        .clear_symbol_version('remote_handle_open')
        .clear_symbol_version('rpcmem_alloc')
        .clear_symbol_version('rpcmem_free')
        .clear_symbol_version('rpcmem_to_fd'),
    ('vendor/lib64/libmialgo_pureShot.so', 'vendor/lib64/libmialgo_rfs.so'): blob_fixup()
        .clear_symbol_version('remote_handle64_close')
        .clear_symbol_version('remote_handle64_invoke')
        .clear_symbol_version('remote_handle64_open')
        .clear_symbol_version('remote_register_buf_attr')
        .clear_symbol_version('remote_session_control'),
    'vendor/lib64/libwa_sat.so': blob_fixup()
        .binary_regex_replace(b'/system/lib64\x00', b'/vendor/lib64\x00'),
    'vendor/lib64/vendor.xiaomi.hardware.cameraperf@1.0-impl.so': blob_fixup()
        .sig_replace('7c 00 00 94 88 16 40 F9', '1F 20 03 D5 88 16 40 F9'),
}  # fmt: skip

module = ExtractUtilsModule(
    'odin',
    'xiaomi',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
    add_firmware_proprietary_file=True,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
