/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef RINGTONE_PERMISSION_UTILS_H
#define RINGTONE_PERMISSION_UTILS_H

#include "bundle_mgr_interface.h"

namespace OHOS {
namespace Media {
const std::string PERM_WRITE_RINGTONE = "ohos.permission.WRITE_RINGTONE";

class RingtonePermissionUtils {
public:
    static bool CheckCallerPermission(const std::string &permission);
    static bool CheckCallerPermission(const std::vector<std::string> &permsVec);
    static bool CheckHasPermission(const std::vector<std::string> &permsVec);
    static void GetClientBundle(const int uid, std::string &bundleName);
    static uint32_t GetTokenId();
    static bool CheckIsSystemAppByUid();
    static bool IsSystemApp();
    static bool IsNativeSAApp();
    static std::string GetPackageNameByBundleName(const std::string &bundleName);

private:
    static sptr<AppExecFwk::IBundleMgr> GetSysBundleManager();
    __attribute__ ((visibility ("hidden"))) static sptr<AppExecFwk::IBundleMgr> bundleManager_;
    __attribute__ ((visibility ("hidden"))) static std::mutex bundleManagerMutex_;
};
}  // namespace Media
}  // namespace OHOS
#endif // RINGTONE_PERMISSION_UTILS_H
