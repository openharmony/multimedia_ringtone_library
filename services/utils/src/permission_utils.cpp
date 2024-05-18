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

#include "permission_utils.h"

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "privacy_kit.h"
#include "ringtone_log.h"
#include "ringtone_tracer.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace Media {
using namespace std;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::AppExecFwk::Constants;

sptr<AppExecFwk::IBundleMgr> RingtonePermissionUtils::bundleManager_ = nullptr;
mutex RingtonePermissionUtils::bundleManagerMutex_;
sptr<AppExecFwk::IBundleMgr> RingtonePermissionUtils::GetSysBundleManager()
{
    RINGTONE_ERR_LOG("ldl GetSysBundleManager 0000000000000000000000000000.");
    if (bundleManager_ != nullptr) {
        RINGTONE_ERR_LOG("ldl GetSysBundleManager 11111111111111111111111111.");
        return bundleManager_;
    }

    lock_guard<mutex> lock(bundleManagerMutex_);
    if (bundleManager_ != nullptr) {
        RINGTONE_ERR_LOG("ldl GetSysBundleManager 2222222222222222222222222222222.");
        return bundleManager_;
    }

    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        RINGTONE_ERR_LOG("ldl GetSysBundleManager 3333333333333333333333333.");
        RINGTONE_ERR_LOG("Failed to get SystemAbilityManager.");
        return nullptr;
    }

    auto bundleObj = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        RINGTONE_ERR_LOG("ldl GetSysBundleManager 444444444444444444444444444.");
        RINGTONE_ERR_LOG("Remote object is nullptr.");
        return nullptr;
    }

    auto bundleManager = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    if (bundleManager == nullptr) {
        RINGTONE_ERR_LOG("ldl GetSysBundleManager 55555555555555555555555555555555555.");
        RINGTONE_ERR_LOG("Failed to iface_cast");
        return nullptr;
    }
    bundleManager_ = bundleManager;
    RINGTONE_ERR_LOG("ldl GetSysBundleManager 66666666666666666666666666666666.");

    return bundleManager_;
}

bool inline ShouldAddPermissionRecord(const AccessTokenID &token)
{
    return (AccessTokenKit::GetTokenTypeFlag(token) == TOKEN_HAP);
}

void RingtonePermissionUtils::GetClientBundle(const int uid, string &bundleName)
{
    bundleManager_ = GetSysBundleManager();
    if (bundleManager_ == nullptr) {
        bundleName = "";
        return;
    }
    auto result = bundleManager_->GetBundleNameForUid(uid, bundleName);
    if (!result) {
        RINGTONE_ERR_LOG("GetBundleNameForUid fail");
        bundleName = "";
    }
}

void AddPermissionRecord(const AccessTokenID &token, const string &perm, const bool permGranted)
{
    if (!ShouldAddPermissionRecord(token)) {
        return;
    }

    int result = PrivacyKit::AddPermissionUsedRecord(token, perm, !!permGranted, !permGranted, true);
    if (result != 0) {
        /* Failed to add permission used record, not fatal */
        RINGTONE_WARN_LOG("Failed to add permission used record: %{public}s, permGranted: %{public}d, err: %{public}d",
            perm.c_str(), permGranted, result);
    }
}

bool RingtonePermissionUtils::CheckCallerPermission(const string &permission)
{
    RingtoneTracer tracer;
    tracer.Start("CheckCallerPermission");

    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    int result = AccessTokenKit::VerifyAccessToken(tokenCaller, permission);
    if (result != PermissionState::PERMISSION_GRANTED) {
        RINGTONE_ERR_LOG("Have no media permission: %{public}s", permission.c_str());
        AddPermissionRecord(tokenCaller, permission, false);
        return false;
    }
    AddPermissionRecord(tokenCaller, permission, true);

    return true;
}

/* Check whether caller has at least one of @permsVec */
bool RingtonePermissionUtils::CheckHasPermission(const vector<string> &permsVec)
{
    if (permsVec.empty()) {
        return false;
    }

    for (const auto &perm : permsVec) {
        if (CheckCallerPermission(perm)) {
            return true;
        }
    }

    return false;
}

string RingtonePermissionUtils::GetPackageNameByBundleName(const string &bundleName)
{
    const static int32_t  invalidUid = -1;
    const static int32_t baseUserRange = 200000;

    int uid = IPCSkeleton::GetCallingUid();
    if (uid <= invalidUid) {
        RINGTONE_ERR_LOG("Get invalidUid UID %{public}d", uid);
        return "";
    }
    int32_t userId = uid / baseUserRange;
    RINGTONE_DEBUG_LOG("uid:%{private}d, userId:%{private}d", uid, userId);

    AAFwk::Want want;
    auto bundleManager_ = GetSysBundleManager();
    if (bundleManager_ == nullptr) {
        RINGTONE_ERR_LOG("Get BundleManager failed");
        return "";
    }
    int ret = bundleManager_->GetLaunchWantForBundle(bundleName, want, userId);
    if (ret != ERR_OK) {
        RINGTONE_ERR_LOG("Can not get bundleName by want, err=%{public}d, userId=%{private}d",
            ret, userId);
        return "";
    }
    string abilityName = want.GetOperation().GetAbilityName();
    return bundleManager_->GetAbilityLabel(bundleName, abilityName);
}

/* Check whether caller has all the @permsVec */
bool RingtonePermissionUtils::CheckCallerPermission(const vector<string> &permsVec)
{
    if (permsVec.empty()) {
        return false;
    }

    for (const auto &perm : permsVec) {
        if (!CheckCallerPermission(perm)) {
            return false;
        }
    }
    return true;
}

uint32_t RingtonePermissionUtils::GetTokenId()
{
    return IPCSkeleton::GetCallingTokenID();
}

bool RingtonePermissionUtils::IsSystemApp()
{
    uint64_t tokenId = IPCSkeleton::GetCallingFullTokenID();
    return TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

bool RingtonePermissionUtils::CheckIsSystemAppByUid()
{
    int uid = IPCSkeleton::GetCallingUid();
    bundleManager_ = GetSysBundleManager();
    if (bundleManager_ == nullptr) {
        RINGTONE_ERR_LOG("Can not get bundleMgr");
        return false;
    }
    return bundleManager_->CheckIsSystemAppByUid(uid);
}

bool RingtonePermissionUtils::IsNativeSAApp()
{
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    RINGTONE_DEBUG_LOG("check if native sa token, tokenId:%{public}d, tokenType:%{public}d", tokenId, tokenType);
    if (tokenType == ATokenTypeEnum::TOKEN_NATIVE) {
        return true;
    }
    return false;
}
}  // namespace Media
}  // namespace OHOS
