DEVICE_PATH := device/xiaomi/odin
BOARD_VENDOR := xiaomi

BUILD_BROKEN_DUP_RULES := true

# Architecture
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-2a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 :=
TARGET_CPU_VARIANT := cortex-a76
TARGET_CPU_VARIANT_RUNTIME := kryo385

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv8-a
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi
TARGET_2ND_CPU_VARIANT := cortex-a76
TARGET_2ND_CPU_VARIANT_RUNTIME := kryo385

# Bootloader
TARGET_BOOTLOADER_BOARD_NAME := odin
TARGET_NO_BOOTLOADER := true

# HIDL
DEVICE_MANIFEST_FILE := $(DEVICE_PATH)/vintf/manifest.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/AHBF@2.1-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.atrace@1.0-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.boot@1.1.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.cas@1.2-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.dumpstate@1.1-service.xiaomi.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.gnss@2.1-service-qti.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.graphics.mapper-impl-qti-display.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.health@2.1.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.ir@1.0-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.lights-qti.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.neuralnetworks@1.3-service-qti.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.sensors@2.0-multihal.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.thermal@2.0-service.qti.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.usb@1.2-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.wifi.hostapd.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/android.hardware.wifi@1.0-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/c2_manifest_vendor.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/fod.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/manifest.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/manifest_android.hardware.drm@1.3-service.clearkey.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/manifest_android.hardware.drm@1.3-service.widevine.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/manifest_vendor.xiaomi.hardware.cld.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/manifest_vendor.xiaomi.hardware.mfidoca.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/manifest_vendor.xiaomi.hardware.misecurity.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/manifest_vendor.xiaomi.hardware.mlipay.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/manifest_vendor.xiaomi.hardware.mtdservice.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/manifest_vendor.xiaomi.hardware.otrpagent.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/manifest_vendor.xiaomi.hardware.tidaservice.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/manifest_vendor.xiaomi.hardware.vsimapp.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/power.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.fs.fsalsps@1.0-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.qti.diag.hal.service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.qti.gnss@4.0-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.qti.hardware.display.allocator-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.qti.hardware.display.composer-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.qti.hardware.servicetracker@1.2-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.qti.hardware.vibrator.service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.xiaomi.cit.bluetooth@1.0_manifest.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.xiaomi.cit.wifi@1.0_manifest.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.xiaomi.hardware.citsensorservice@1.1-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.xiaomi.hardware.micharge@1.0.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.xiaomi.hardware.misys@1.0.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.xiaomi.hardware.misys@2.0.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.xiaomi.hardware.misys@3.0.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.xiaomi.hardware.vibratorfeature@1.0-service.xml
DEVICE_MANIFEST_FILE += $(DEVICE_PATH)/vintf/vendor.xiaomi.hw.touchfeature@1.0-service.xml
DEVICE_MATRIX_FILE := $(DEVICE_PATH)/vintf/compatibility_matrix.xml
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE := $(DEVICE_PATH)/vintf/framework_compatibility_matrix.xml

# Init
TARGET_INIT_VENDOR_LIB := //$(DEVICE_PATH):init_odin
TARGET_RECOVERY_DEVICE_MODULES := init_odin

# Platform
TARGET_BOARD_PLATFORM := lahaina

# QCOM
BOARD_USES_QCOM_HARDWARE := true

# Releasetools
TARGET_RELEASETOOLS_EXTENSIONS := $(DEVICE_PATH)

# Security patch level
VENDOR_SECURITY_PATCH := 2021-07-01

-include vendor/xiaomi/odin/BoardConfigVendor.mk
