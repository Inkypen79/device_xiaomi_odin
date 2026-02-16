/*
 * Copyright 2021, The Android Open Source Project
 *
 * Copyright 2023 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HARDWARE_UWB_UWBCHIP
#define ANDROID_HARDWARE_UWB_UWBCHIP

#include <vector>
#include <android-base/logging.h>


#include <aidl/android/hardware/uwb/BnUwbChip.h>
#include <aidl/android/hardware/uwb/IUwbClientCallback.h>

namespace android {
namespace hardware {
namespace uwb {
namespace impl {
using namespace ::aidl::android::hardware::uwb;
// Default implementation mean't to be used on simulator targets.
class UwbChip : public BnUwbChip {
  public:
    UwbChip(const std::string& name);
    virtual ~UwbChip();

    ::ndk::ScopedAStatus getName(std::string* name) override;
    ::ndk::ScopedAStatus open(const std::shared_ptr<IUwbClientCallback>& clientCallback) override;
    ::ndk::ScopedAStatus close() override;
    ::ndk::ScopedAStatus coreInit() override;
    ::ndk::ScopedAStatus getSupportedAndroidUciVersion(int32_t* version) override;
    ::ndk::ScopedAStatus sendUciMessage(const std::vector<uint8_t>& data,
                                        int32_t* bytes_written) override;

    ::ndk::ScopedAStatus sessionInit(int32_t sessionId) override;
  static void eventCallback(uint8_t event, uint8_t status) {
  if (mClientCallback != nullptr) {
      auto ret = mClientCallback->onHalEvent((UwbEvent)event,
                                      (UwbStatus)status);
      if (!ret.isOk()) {
          LOG(ERROR) << "Failed to call back into uwb process";
      }
    }
  }
  static void dataCallback(uint16_t data_len, uint8_t* p_data) {
      std::vector<uint8_t> data;
      data.assign(p_data, p_data + data_len);
      if (mClientCallback != nullptr) {
          auto ret = mClientCallback->onUciMessage(data);
          if (!ret.isOk()) {
              LOG(ERROR) << "Failed to data call back into uwb process";
          }
      }
  }


  private:
    std::string name_;

  public:
    static std::shared_ptr<IUwbClientCallback> mClientCallback;
};
}  // namespace impl
}  // namespace uwb
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_UWB_UWBCHIP
