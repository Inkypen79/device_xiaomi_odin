/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <libinit_utils.h>
#include <libinit_device_names.h>

void set_device_names() {
    property_override("bluetooth.device.default_name", "Xiaomi MIX 4");
    property_override("ro.product.marketname", "Xiaomi MIX 4");
    property_override("vendor.usb.product_string", "Xiaomi MIX 4");
}
