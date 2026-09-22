/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"

namespace hku {

/**
 * @brief Bind the license code
 * @param email e-mail
 * @param active_code license code
 */
void HKU_API bindEmail(const std::string& email, const std::string& active_code);

/**
 * @brief Activate the device
 * @param active_code license code
 * @param replace when the device limit is exceeded, forcefully replace the earliest activated
 * device
 */
void HKU_API activeDevice(const std::string& active_code, bool replace = false);

/** View the license information */
std::string HKU_API viewLicense();

/** Remove the license */
void HKU_API removeLicense();

/** Get the trial license */
std::string HKU_API fetchTrialLicense(const std::string& email);

/** Check whether the license is valid */
bool HKU_API isValidLicense();

/** Get the license expiration time */
Datetime HKU_API getExpireDate();

}  // namespace hku
