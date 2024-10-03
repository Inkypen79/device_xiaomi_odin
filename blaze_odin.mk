# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common Project Blaze stuff
$(call inherit-product, vendor/blaze/config/common_full_phone.mk)

TARGET_SUPPORTS_QUICK_TAP := true

BLAZE_MAINTAINER := Inkypen
TARGET_SUPPORTS_BLUR := true
TARGET_FACE_UNLOCK_SUPPORTED := true
WITH_GAPPS := true

# Boot animation
TARGET_BOOT_ANIMATION_RES := 1080

# Inherit from odin device
$(call inherit-product, $(LOCAL_PATH)/device.mk)

PRODUCT_BRAND := Xiaomi
PRODUCT_DEVICE := odin
PRODUCT_MANUFACTURER := Xiaomi
PRODUCT_NAME := blaze_odin
PRODUCT_MODEL := 2106118C
PRODUCT_SHIPPING_API_LEVEL := 30

PRODUCT_GMS_CLIENTID_BASE := android-xiaomi
TARGET_VENDOR := xiaomi
TARGET_VENDOR_PRODUCT_NAME := odin
PRODUCT_BUILD_PROP_OVERRIDES += PRIVATE_BUILD_DESC="odin-user 13 RKQ1.211001.001 V816.0.4.0.UKMCNXM release-keys"

# Set BUILD_FINGERPRINT variable to be picked up by both system and vendor build.prop
BUILD_FINGERPRINT := Xiaomi/odin/odin:13/RKQ1.211001.001/V816.0.4.0.UKMCNXM:user/release-keys
