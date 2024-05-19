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
#include "ringtone_errno.h"
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
static const string RINGTONE_LIBRARY_PATH = "/data/storage/el2/base/files";
std::shared_ptr<DataShare::DataShareHelper> g_dataShareHelper = nullptr;
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

static int SetPermission()
{
    vector<string> perms;
    perms.push_back("ohos.permission.READ_MEDIA");
    perms.push_back("ohos.permission.WRITE_MEDIA");
    perms.push_back("ohos.permission.FILE_ACCESS_MANAGER");
    perms.push_back("ohos.permission.GET_BUNDLE_INFO_PRIVILEGED");
    perms.push_back("ohos.permission.WRITE_RINGTONE");

    uint64_t tokenId = 0;
    RingtonePermissionUtilsUnitTest::SetAccessTokenPermission("RingtoneScan", perms, tokenId);
    if (tokenId == 0) {
        RINGTONE_ERR_LOG("Set Access Token Permisson Failed.");
        return E_ERR;
    }
    return E_SUCCESS;
}

static int RingtoneRead(const unique_ptr<RingtoneAsset> &ringtoneAsset)
{
    if (ringtoneAsset != nullptr && ringtoneAsset->GetPath() != RINGTONE_DEFAULT_STR) {
        string uriStr = RINGTONE_PATH_URI + RINGTONE_SLASH_CHAR + to_string(ringtoneAsset->GetId());
        Uri ofUri(uriStr);
        auto fd = g_dataShareHelper->OpenFile(ofUri, "rw");
        cout << "OpenFile fd = " << fd << endl;
        if (fd < 0) {
            cout << "OpenFile error" << endl;
            return -fd;
        }
        string dataBuffer = "hello world";

        ssize_t written = write(fd, dataBuffer.c_str(), dataBuffer.size());
        cout << "OpenFile written = " << written << endl;
        if (written < 0) {
            cout << "Failed to copy data buffer, return " << static_cast<int>(written) << endl;
            close(fd);
            return E_ERR;
        }
        close(fd);

        fd = g_dataShareHelper->OpenFile(ofUri, "rw");
        cout << "OpenFile fd = " << fd << endl;
        if (fd < 0) {
            cout << "OpenFile error" << endl;
            return -fd;
        }
        const int maxSize = 30;
        char readBuffer[maxSize];
        ssize_t readRet = read(fd, readBuffer, maxSize);
        cout << "OpenFile readRet = " << readRet << endl;
        if (readRet < 0) {
            cout << "Failed to copy data buffer, return " << static_cast<int>(readRet) << endl;
            close(fd);
            return E_ERR;
        }
        if (strcmp(readBuffer, dataBuffer.c_str())) {
            cout << "Read failure readBuffer != dataBuffer,  readBuffer = " << static_cast<int>(readRet) <<
                "written = " << static_cast<int>(written) << endl;
            close(fd);
            return E_ERR;
        }
        close(fd);
    }
    return E_SUCCESS;
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
    int ret = SetPermission();
    if (ret != E_SUCCESS) {
        return ret;
    }
    g_dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    if (g_dataShareHelper == nullptr) {
        RINGTONE_ERR_LOG("g_dataShareHelper fail");
        return ret;
    }

    Uri ringtoneUri(RINGTONE_PATH_URI);
    // Ringtone library Insert api demo
    const int size = 1022;
    DataShareValuesBucket valuesBucket;
    valuesBucket.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + "/" +
        "test_insert_ringtone_library" + to_string(0) + ".ogg"));
    valuesBucket.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(size));
    valuesBucket.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>("rainning") + ".ogg");
    valuesBucket.Put(RINGTONE_COLUMN_TITLE, static_cast<string>("rainning"));
    valuesBucket.Put(RINGTONE_COLUMN_MIME_TYPE, static_cast<string>("ogg"));
    valuesBucket.Put(RINGTONE_COLUMN_SOURCE_TYPE, static_cast<int>(1));
    valuesBucket.Put(RINGTONE_COLUMN_SHOT_TONE_TYPE, static_cast<int>(1));
    g_dataShareHelper->Insert(ringtoneUri, valuesBucket);

    // Ringtone library Query api demo
    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_SHOT_TONE_TYPE, to_string(1));
    vector<string> columns = { {RINGTONE_COLUMN_TONE_ID}, {RINGTONE_COLUMN_DATA}};
    auto resultSet = g_dataShareHelper->Query(ringtoneUri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
    cout << "query count = " << to_string(results->GetCount()) << endl;
    unique_ptr<RingtoneAsset> ringtoneAsset = results->GetLastObject();

    ret = RingtoneRead(ringtoneAsset);
    if (ret != E_SUCCESS) {
        return E_ERR;
    }

    system("rm -rf /data/storage/el2/base/files/*");
    return 0;
}
