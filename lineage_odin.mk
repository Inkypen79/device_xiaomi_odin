# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common Lineage stuff
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Inherit from odin device
$(call inherit-product, $(LOCAL_PATH)/device.mk)

PRODUCT_BRAND := xiaomi
PRODUCT_DEVICE := odin
PRODUCT_MANUFACTURER := xiaomi
PRODUCT_NAME := lineage_odin
PRODUCT_MODEL := 2106118C

PRODUCT_GMS_CLIENTID_BASE := android-xiaomi
TARGET_VENDOR := xiaomi
TARGET_VENDOR_PRODUCT_NAME := odin
PRODUCT_BUILD_PROP_OVERRIDES += PRIVATE_BUILD_DESC="missi-user 11 RKQ1.210503.001 V12.5.4.0.RKMCNXM release-keys"

# Set BUILD_FINGERPRINT variable to be picked up by both system and vendor build.prop
BUILD_FINGERPRINT := Xiaomi/odin/odin:11/RKQ1.210503.001/V12.5.4.0.RKMCNXM:user/release-keys
