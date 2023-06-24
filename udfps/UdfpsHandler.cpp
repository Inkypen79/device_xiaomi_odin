/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "UdfpsHandler.xiaomi_sm8350"

#include <android-base/logging.h>

#include <fcntl.h>
#include <fstream>
#include <poll.h>
#include <thread>

#include "UdfpsHandler.h"

// Fingerprint hwmodule commands
#define COMMAND_NIT 10
#define PARAM_NIT_UDFPS 1
#define PARAM_NIT_NONE 0

// Touchscreen and HBM
#define FOD_HBM_PATH "/sys/devices/platform/soc/soc:qcom,dsi-display-primary/fod_hbm"
#define FOD_STATUS_PATH "/sys/devices/virtual/touch/tp_dev/fod_status"
#define FOD_UI_PATH "/sys/devices/platform/soc/soc:qcom,dsi-display-primary/fod_ui"

#define FOD_HBM_OFF 0
#define FOD_HBM_ON 1
#define FOD_STATUS_OFF 0
#define FOD_STATUS_ON 1

template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value;
}

static bool readBool(int fd) {
    char c;
    int rc;

    rc = lseek(fd, 0, SEEK_SET);
    if (rc) {
        LOG(ERROR) << "failed to seek fd, err: " << rc;
        return false;
    }

    rc = read(fd, &c, sizeof(char));
    if (rc != 1) {
        LOG(ERROR) << "failed to read bool from fd, err: " << rc;
        return false;
    }

    return c != '0';
}

class XiaomiUdfpsHander : public UdfpsHandler {
  public:
    void init(fingerprint_device_t* device) {
        mDevice = device;

        std::thread([this]() {
            int fd = open(FOD_UI_PATH, O_RDONLY);
            if (fd < 0) {
                LOG(ERROR) << "failed to open fd, err: " << fd;
                return;
            }

            struct pollfd fodUiPoll = {
                    .fd = fd,
                    .events = POLLERR | POLLPRI,
                    .revents = 0,
            };

            while (true) {
                int rc = poll(&fodUiPoll, 1, -1);
                if (rc < 0) {
                    LOG(ERROR) << "failed to poll fd, err: " << rc;
                    continue;
                }

                mDevice->extCmd(mDevice, COMMAND_NIT, readBool(fd) ? PARAM_NIT_UDFPS : PARAM_NIT_NONE);
            }
        }).detach();
    }

    void onFingerDown(uint32_t /*x*/, uint32_t /*y*/, float /*minor*/, float /*major*/) {
        // nothing
    }

    void onFingerUp() {
        // nothing
    }

    void onAcquired(int32_t result, int32_t vendorCode) {
        if (result == FINGERPRINT_ACQUIRED_GOOD) {
            set(FOD_HBM_PATH, FOD_HBM_OFF);
            set(FOD_STATUS_PATH, FOD_STATUS_OFF);
        } else if (vendorCode == 21 || vendorCode == 23) {
            /*
             * vendorCode = 21 waiting for fingerprint authentication
             * vendorCode = 23 waiting for fingerprint enroll
             */
            set(FOD_STATUS_PATH, FOD_STATUS_ON);
        }
    }

    void cancel() {
        set(FOD_STATUS_PATH, FOD_STATUS_OFF);
        set(FOD_HBM_PATH, FOD_HBM_OFF);
    }

  private:
    fingerprint_device_t* mDevice;
};

static UdfpsHandler* create() {
    return new XiaomiUdfpsHander();
}

static void destroy(UdfpsHandler* handler) {
    delete handler;
}

extern "C" UdfpsHandlerFactory UDFPS_HANDLER_FACTORY = {
        .create = create,
        .destroy = destroy,
};
