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
#include "nativetoken_kit.h"
#include "ringtone_asset.h"
#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_fetch_result.h"
#include "ringtone_log.h"
#include "token_setproc.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::DataShare;

namespace OHOS {
namespace Media {
namespace {
constexpr int STORAGE_MANAGER_MANAGER_ID = 5003;
static const string RINGTONE_LIBRARY_PATH =
    "/storage/media/local/data/com.ohos.ringtonelibrary.ringtonelibrarydata";
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

static void RingtoneIndex()
{
    Uri ringtoneUri(RINGTONE_PATH_URI);
    int32_t index = 0;
    const int32_t count = 10;
    const int64_t ringtoneSize = 1022;
    const int type = 2;
    const int64_t addedTime = 1559276453;
    const int64_t modifiedTime = 1559276455;
    const int64_t takenTime = 1559276457;
    const int durationTime = 112;
    while (index++ < count) {
        DataShareValuesBucket valuesBucket;
        valuesBucket.Put(RINGTONE_COLUMN_DATA, static_cast<string>(RINGTONE_LIBRARY_PATH + "/" +
            "test_insert_ringtone_library" + to_string(index) + ".ogg"));
        valuesBucket.Put(RINGTONE_COLUMN_SIZE, static_cast<int64_t>(ringtoneSize));
        valuesBucket.Put(RINGTONE_COLUMN_DISPLAY_NAME, static_cast<string>("rainning") + ".ogg");
        valuesBucket.Put(RINGTONE_COLUMN_TITLE, static_cast<string>("rainning"));
        valuesBucket.Put(RINGTONE_COLUMN_MEDIA_TYPE, static_cast<int>(type));
        valuesBucket.Put(RINGTONE_COLUMN_TONE_TYPE, static_cast<int>(type));
        valuesBucket.Put(RINGTONE_COLUMN_MIME_TYPE, static_cast<string>("ogg"));
        valuesBucket.Put(RINGTONE_COLUMN_SOURCE_TYPE, static_cast<int>(1));
        valuesBucket.Put(RINGTONE_COLUMN_DATE_ADDED, static_cast<int64_t>(addedTime));
        valuesBucket.Put(RINGTONE_COLUMN_DATE_MODIFIED, static_cast<int64_t>(modifiedTime));
        valuesBucket.Put(RINGTONE_COLUMN_DATE_TAKEN, static_cast<int64_t>(takenTime));
        valuesBucket.Put(RINGTONE_COLUMN_DURATION, static_cast<int>(durationTime));
        valuesBucket.Put(RINGTONE_COLUMN_SHOT_TONE_TYPE, static_cast<int>(1));
        valuesBucket.Put(RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, static_cast<int>(type));
        valuesBucket.Put(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE, static_cast<int>(1));
        valuesBucket.Put(RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, static_cast<int>(type));
        valuesBucket.Put(RINGTONE_COLUMN_RING_TONE_TYPE, static_cast<int>(1));
        valuesBucket.Put(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, static_cast<int>(type));
        valuesBucket.Put(RINGTONE_COLUMN_ALARM_TONE_TYPE, static_cast<int>(1));
        valuesBucket.Put(RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE, static_cast<int>(type));
        valuesBucket.Put(RINGTONE_COLUMN_SCANNER_FLAG, static_cast<int>(0));

        g_dataShareHelper->Insert(ringtoneUri, valuesBucket);
    }
}

static int RingtoneDeleteAndUpdate(int32_t toneId)
{
    Uri ringtoneUri(RINGTONE_PATH_URI);
    // Ringtone library Delete api demo
    int32_t deleteId = toneId;
    DataShare::DataSharePredicates deletePredicates;
    deletePredicates.SetWhereClause(RINGTONE_COLUMN_TONE_ID + " < ? ");
    deletePredicates.SetWhereArgs({ to_string(deleteId) });

    // Ringtone library Update api demo
    DataSharePredicates updatePredicates;

    vector<string> updateIds;
    updateIds.push_back(to_string(toneId - 1));
    deletePredicates.In(RINGTONE_COLUMN_TONE_ID, updateIds);

    DataShareValuesBucket updateValuesBucket;
    updateValuesBucket.Put(RINGTONE_COLUMN_SHOT_TONE_TYPE, 0);
    updateValuesBucket.Put(RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, 0);

    int32_t changedRows = g_dataShareHelper->Update(ringtoneUri, deletePredicates, updateValuesBucket);
    if (changedRows < 0) {
        cout << "Update error" << endl;
        return E_ERR;
    }
    cout << "Update " << changedRows << " rows from ringtone library." << endl;

    changedRows = g_dataShareHelper->Delete(ringtoneUri, deletePredicates);
    cout << "delete " << changedRows << " rows from ringtone library." << endl;
    return E_SUCCESS;
}

static int32_t RingtoneQuery()
{
    // Ringtone library Query api demo
    Uri ringtoneUri(RINGTONE_PATH_URI);
    int errCode = 0;
    DatashareBusinessError businessError;
    DataSharePredicates queryPredicates;
    queryPredicates.EqualTo(RINGTONE_COLUMN_SHOT_TONE_TYPE, to_string(1));
    vector<string> columns = {};
    auto resultSet = g_dataShareHelper->Query(ringtoneUri, queryPredicates, columns, &businessError);
    errCode = businessError.GetCode();
    cout << "Query errCode=" << errCode << endl;

    auto results = make_unique<RingtoneFetchResult<RingtoneAsset>>(move(resultSet));
    cout << "query count = " << to_string(results->GetCount()) << endl;
    unique_ptr<RingtoneAsset> ringtoneAsset = results->GetFirstObject();
    do {
        cout << "GetId()                            = " << to_string(ringtoneAsset->GetId()) << endl;
        cout << "GetSize()                          = " << to_string(ringtoneAsset->GetSize()) << endl;
        cout << "GetPath()                          = " << ringtoneAsset->GetPath() << endl;
        cout << "GetDisplayName()                   = " << ringtoneAsset->GetDisplayName() << endl;
        cout << "GetTitle()                         = " << ringtoneAsset->GetTitle() << endl;
        cout << "GetMimeType()                      = " << ringtoneAsset->GetMimeType() << endl;
        cout << "GetSourceType()                    = " << to_string(ringtoneAsset->GetSourceType()) << endl;
        cout << "GetDateAdded()                     = " << to_string(ringtoneAsset->GetDateAdded()) << endl;
        cout << "GetDateModified()                  = " << to_string(ringtoneAsset->GetDateModified()) << endl;
        cout << "GetDateTaken()                     = " << to_string(ringtoneAsset->GetDateTaken()) << endl;
        cout << "GetDuration()                      = " << to_string(ringtoneAsset->GetDuration()) << endl;
        cout << "GetShottoneType()                  = " << to_string(ringtoneAsset->GetShottoneType()) << endl;
        cout << "GetShottoneSourceType()            = " << to_string(ringtoneAsset->GetShottoneSourceType()) << endl;
        cout << "GetNotificationtoneType()          = " << to_string(ringtoneAsset->GetNotificationtoneType()) << endl;
        cout << "GetNotificationtoneSourceType()    = " << to_string(ringtoneAsset->GetNotificationtoneSourceType())
            << endl;
        cout << "GetRingtoneType()                  = " << to_string(ringtoneAsset->GetRingtoneType()) << endl;
        cout << "GetRingtoneSourceType()            = " << to_string(ringtoneAsset->GetRingtoneSourceType()) << endl;
        cout << "GetAlarmtoneType()                 = " << to_string(ringtoneAsset->GetAlarmtoneType()) << endl;
        cout << "GetAlarmtoneSourceType()           = " << to_string(ringtoneAsset->GetAlarmtoneSourceType()) << endl;
        cout << "GetScannerFlag()                   = " << to_string(ringtoneAsset->GetScannerFlag()) << endl;
        ringtoneAsset = results->GetNextObject();
    } while (ringtoneAsset != nullptr);
    ringtoneAsset = results->GetLastObject();
    return ringtoneAsset->GetId();
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

    g_dataShareHelper = CreateDataShareHelper(STORAGE_MANAGER_MANAGER_ID);
    if (g_dataShareHelper == nullptr) {
        RINGTONE_ERR_LOG("g_dataShareHelper fail");
        return -1;
    }

    RingtoneIndex();
    int32_t toneId = RingtoneQuery();
    cout << "query last tone id = " << toneId << endl;

    string uriStr = RINGTONE_PATH_URI + "/" + to_string(toneId);
    Uri ofUri(uriStr);
    auto fd = g_dataShareHelper->OpenFile(ofUri, "rw");
    cout << "OpenFile fd = " << fd << endl;

    if (fd >= 0) {
        string dataBuffer = "hello world";
        ssize_t written = write(fd, dataBuffer.c_str(), dataBuffer.size());
        if (written < 0) {
            cout << "Failed to copy data buffer, return " << static_cast<int>(written) << endl;
            return -1;
        }
    } else {
        cout << "OpenFile error" << endl;
        return -fd;
    }
    close(fd);

    int ret = RingtoneDeleteAndUpdate(toneId);
    if (ret != E_SUCCESS) {
        return -1;
    }
    system("rm -rf /storage/media/100/local/data/com.ohos.ringtonelibrary.ringtonelibrarydata/*");
    return 0;
}
