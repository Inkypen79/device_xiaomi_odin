# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Inherit some common Lineage stuff
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Boot animation
TARGET_BOOT_ANIMATION_RES := 1080

# Inherit from odin device
$(call inherit-product, $(LOCAL_PATH)/device.mk)

PRODUCT_BRAND := Xiaomi
PRODUCT_DEVICE := odin
PRODUCT_MANUFACTURER := Xiaomi
PRODUCT_NAME := lineage_odin
PRODUCT_MODEL := M2102K1G
PRODUCT_SHIPPING_API_LEVEL := 30

PRODUCT_GMS_CLIENTID_BASE := android-xiaomi
TARGET_VENDOR := xiaomi
TARGET_VENDOR_PRODUCT_NAME := odin
PRODUCT_BUILD_PROP_OVERRIDES += PRIVATE_BUILD_DESC="missi-user 12 SKQ1.211006.001 V13.0.8.0.SKMCNXM release-keys"

# Set BUILD_FINGERPRINT variable to be picked up by both system and vendor build.prop
BUILD_FINGERPRINT := Xiaomi/odin/odin:12/RKQ1.211001.001/V13.0.8.0.SKMCNXM:user/release-keys
