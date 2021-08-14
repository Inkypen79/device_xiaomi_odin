/*
 * Copyright (C) 2021 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LIBINIT_UTILS_H
#define LIBINIT_UTILS_H

#include <string>

void property_override(std::string prop, std::string value, bool add = true);

#endif // LIBINIT_UTILS_H
