/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#pragma once

#include <string>
#include "hikyuu/utilities/datetime/Datetime.h"

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

void sysinfo_init();
void sysinfo_clean();

/**
 * Get the current version number of Hikyuu
 * @return version number
 */
std::string HKU_API getVersion();

/**
 * Get the detailed version number, including the build time
 */
std::string HKU_API getVersionWithBuild();

/* Get the version number including the Git commit information */
std::string HKU_API getVersionWithGit();

/**
 * Judge whether there is a newer version to upgrade to
 */
bool HKU_API CanUpgrade();

struct HKU_API LatestVersionInfo {
    int version{1003001};
    Datetime release_date;
    std::string remark;
    LatestVersionInfo() = default;
    LatestVersionInfo(const LatestVersionInfo&) = default;
    LatestVersionInfo(LatestVersionInfo&& rhs)
    : version(rhs.version), release_date(rhs.release_date), remark(std::move(rhs.remark)) {}
    LatestVersionInfo& operator=(LatestVersionInfo&& rhs) {
        if (this == &rhs) {
            return *this;
        }
        version = rhs.version;
        release_date = rhs.release_date;
        remark = std::move(rhs.remark);
        return *this;
    }
};

LatestVersionInfo HKU_API getLatestVersionInfo();

void updateSysInfoExpiredTime(Datetime time);

// License expiration reminder
void HKU_API reminderLicenseExpiration();

/** Send the feedback information */
void HKU_API sendFeedback();

/** Used to send the python version information */
void HKU_API sendPythonVersionFeedBack(int major, int minor, int micro);

/** Whether it is currently running in the python environment */
bool HKU_API runningInPython();

/** Whether it is currently running in the Jupyter environment */
bool HKU_API pythonInJupyter();

/** Whether python is running in the interactive mode */
bool HKU_API pythonInInteractive();

/** Set whether it is running under python */
void HKU_API setRunningInPython(bool inpython);

/** Set whether python runs in the interactive mode */
void HKU_API setPythonInInteractive(bool interactive);

/** Set whether it is running in the Jupyter environment */
void HKU_API setPythonInJupyter(bool injupyter);

}  // namespace hku
