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
#define FOD_UI_PATH "/sys/devices/virtual/mi_display/disp_feature/disp-DSI-0/fod_ui"
#define FOD_PRESS_STATUS_PATH "/sys/class/touch/touch_dev/fod_press_status"

#define FOD_HBM_OFF 0
#define FOD_HBM_ON 1
#define FOD_STATUS_OFF 0
#define FOD_STATUS_ON 1

#define COMMAND_FOD_PRESS_STATUS 1
#define PARAM_FOD_PRESSED 1
#define PARAM_FOD_RELEASED 0

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
            int fodUiFd = open(FOD_UI_PATH, O_RDONLY);
            int fodPressStatusFd = open(FOD_PRESS_STATUS_PATH, O_RDONLY);
            if (fodUiFd < 0) {
                LOG(ERROR) << "failed to open fodUiFd, err: " << fodUiFd;
                return;
            }

            if (fodPressStatusFd < 0) {
                LOG(ERROR) << "failed to open fodPressStatusFd, err: " << fodPressStatusFd;
                return;
            }

            struct pollfd fds[2] = {
                {fodUiFd, .events = POLLERR | POLLPRI, .revents = 0},
                {fodPressStatusFd, .events = POLLERR | POLLPRI, .revents = 0},
            };

            while (true) {
                int rc = poll(fds, 2, -1);
                if (rc < 0) {
                    if (fds[0].revents & POLLERR) {
                        LOG(ERROR) << "failed to poll fodUiFd, err: " << rc;
                    }
                    if (fds[1].revents & POLLERR) {
                        LOG(ERROR) << "failed to poll fodPressStatusFd, err: " << rc;
                    }
                    continue;
                }

                if (fds[0].revents & (POLLERR | POLLPRI)) {
                    bool nitState = readBool(fodUiFd);
                    mDevice->extCmd(mDevice, COMMAND_NIT, nitState ? PARAM_NIT_UDFPS : PARAM_NIT_NONE);
                }

                if (fds[1].revents & (POLLERR | POLLPRI)) {
                    bool pressState = readBool(fodPressStatusFd);
                    mDevice->extCmd(mDevice, COMMAND_FOD_PRESS_STATUS, pressState ? PARAM_FOD_PRESSED : PARAM_FOD_RELEASED);
                }
            }
        }).detach();
    }

    void onFingerDown(uint32_t /*x*/, uint32_t /*y*/, float /*minor*/, float /*major*/) {
        set(FOD_STATUS_PATH, FOD_STATUS_ON);
    }

    void onFingerUp() {
        set(FOD_STATUS_PATH, FOD_STATUS_OFF);
    }

    void onAcquired(int32_t result, int32_t vendorCode) {
        if (result == FINGERPRINT_ACQUIRED_GOOD) {
            set(FOD_HBM_PATH, FOD_HBM_OFF);
            set(FOD_STATUS_PATH, FOD_STATUS_OFF);
        } else if (vendorCode == 21) {
            /*
             * vendorCode = 21 waiting for finger
             * vendorCode = 22 finger down
             * vendorCode = 23 finger up
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
