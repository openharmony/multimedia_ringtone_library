/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define MLOG_TAG "scan_demo"

#include <iostream>
#include <unistd.h>
#include "accesstoken_kit.h"
#include "datashare_helper.h"
#include "get_self_permissions.h"
#include "iservice_registry.h"
#include "ringtone_log.h"
#include "ringtone_db_const.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "ringtone_asset.h"
#include "ringtone_fetch_result.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::DataShare;

namespace OHOS {
namespace Media {
namespace {
constexpr int STORAGE_MANAGER_MANAGER_ID = 5003;
} // namespace
} // namespace Media
} // namespace OHOS

static shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(int32_t systemAbilityId)
{
    RINGTONE_INFO_LOG("CreateDataShareHelper::CreateFileExtHelper ");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        RINGTONE_INFO_LOG("CreateFileExtHelper Get system ability mgr failed.");
        return nullptr;
    }
    auto remoteObj = saManager->GetSystemAbility(systemAbilityId);
    if (remoteObj == nullptr) {
        RINGTONE_INFO_LOG("CreateDataShareHelper GetSystemAbility Service Failed.");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, RINGTONE_URI);
}

/*
 * Feature: RingtoneScanner
 * Function: Strat scanner
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 */
int32_t main()
{
    vector<string> perms;
    perms.push_back("ohos.permission.WRITE_RINGTONE");

    uint64_t tokenId = 0;
    RingtonePermissionUtilsUnitTest::SetAccessTokenPermission("RingtoneScan", perms, tokenId);
    if (tokenId == 0) {
        RINGTONE_ERR_LOG("Set Access Token Permisson Failed.");
        return -1;
    }

    auto ringtoneDataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    if (ringtoneDataShareHelper == nullptr) {
        RINGTONE_ERR_LOG("ringtoneDataShareHelper fail");
        return -1;
    }

    return 0;
}
