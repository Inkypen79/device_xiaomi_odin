/*
 * Copyright (C) 2020 The Android Open Source Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/light/BnLights.h>
#include <hardware/hardware.h>
#include <hardware/lights.h>
#include <map>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class Lights : public BnLights {
    public:
      Lights();
      ndk::ScopedAStatus setLightState(int id, const HwLightState& state) override;
      ndk::ScopedAStatus getLights(std::vector<HwLight>* types) override;

    private:
      std::map<int, light_device_t*> mLights;
      std::vector<HwLight> mAvailableLights;
      int maxLights;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
