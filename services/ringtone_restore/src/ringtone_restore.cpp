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

#define MLOG_TAG "RingtoneRestore"

#include "ringtone_restore.h"

#include <sys/stat.h>

#include "datashare_ext_ability.h"
#include "datashare_ext_ability_context.h"
#include "result_set_utils.h"
#include "ringtone_restore_type.h"
#include "ringtone_restore_db_utils.h"
#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_type.h"
#include "ringtone_fetch_result.h"

namespace OHOS {
namespace Media {
using namespace std;
static const int32_t QUERY_COUNT = 500;
static const int32_t INVALID_QUERY_OFFSET = -1;
int32_t RingtoneRestore::Init(const std::string &backupPath)
{
    RINGTONE_INFO_LOG("Init db start");
    if (backupPath.empty()) {
        RINGTONE_ERR_LOG("error: backup path is null");
        return E_INVALID_ARGUMENTS;
    }
    dbPath_ = backupPath + RINGTONE_LIBRARY_DB_PATH_EL1 + "/rdb" + "/" + RINGTONE_LIBRARY_DB_NAME;
    if (!RingtoneFileUtils::IsFileExists(dbPath_)) {
        RINGTONE_ERR_LOG("ringtone db is not exist, path=%{public}s", dbPath_.c_str());
        dbPath_ = backupPath + RINGTONE_LIBRARY_DB_PATH + "/rdb" + "/" + RINGTONE_LIBRARY_DB_NAME;
        if (!RingtoneFileUtils::IsFileExists(dbPath_)) {
            RINGTONE_ERR_LOG("ringtone db is not exist, path=%{public}s", dbPath_.c_str());
            return E_FAIL;
        }
    }
    backupPath_ = backupPath;
    int32_t err = RingtoneRestoreDbUtils::InitDb(restoreRdb_, RINGTONE_LIBRARY_DB_NAME, dbPath_,
        RINGTONE_BUNDLE_NAME, true);
    if (err != E_OK) {
        RINGTONE_ERR_LOG("ringtone rdb fail, err = %{public}d", err);
        return E_HAS_DB_ERROR;
    }
    if (RingtoneRestoreBase::Init(backupPath) != E_OK) {
        return E_FAIL;
    }

    RINGTONE_INFO_LOG("Init db successfully");
    return E_OK;
}

vector<FileInfo> RingtoneRestore::QueryFileInfos(int32_t offset)
{
    vector<FileInfo> result;
    string querySql = "SELECT * FROM " + RINGTONE_TABLE;
    if (offset != INVALID_QUERY_OFFSET) {
        querySql += " LIMIT " + to_string(offset) + ", " + to_string(QUERY_COUNT);
    }
    auto resultSet = restoreRdb_->QuerySql(querySql);
    if (resultSet == nullptr) {
        return {};
    }

    vector<shared_ptr<RingtoneMetadata>> metaDatas {};
    auto ret = resultSet->GoToFirstRow();
    while (ret == NativeRdb::E_OK) {
        auto metaData = make_unique<RingtoneMetadata>();
        if (PopulateMetadata(resultSet, metaData) != E_OK) {
            RINGTONE_INFO_LOG("read resultset error");
            continue;
        }
        metaDatas.push_back(std::move(metaData));
        ret = resultSet->GoToNextRow();
    };
    resultSet->Close();

    return ConvertToFileInfos(metaDatas);
}

void RingtoneRestore::UpdateSettingInfos()
{
    CHECK_AND_RETURN_LOG(restoreRdb_ != nullptr, "restoreRdb_ is null");
    string querySql = "SELECT * FROM " + SIMCARD_SETTING_TABLE;
    auto resultSet = restoreRdb_->QuerySql(querySql);
    CHECK_AND_RETURN_LOG(resultSet != nullptr, "resultSet is null");

    RingtoneFetchResult<SimcardSettingAsset> fetchResult;
    auto ret = resultSet->GoToFirstRow();
    while (ret == NativeRdb::E_OK) {
        auto rdbResult = std::dynamic_pointer_cast<NativeRdb::ResultSet>(resultSet);
        auto asset = fetchResult.GetObject(rdbResult);
        if (asset != nullptr) {
            UpdateSettingTable(*asset);
        }
        ret = resultSet->GoToNextRow();
    }
    resultSet->Close();
}

vector<FileInfo> RingtoneRestore::ConvertToFileInfos(vector<shared_ptr<RingtoneMetadata>> &metaDatas)
{
    vector<FileInfo> infos = {};
    for (auto meta : metaDatas) {
        infos.emplace_back(*meta);
    }
    return infos;
}

void RingtoneRestore::CheckRestoreFileInfos(vector<FileInfo> &infos)
{
    for (auto it = infos.begin(); it != infos.end();) {
        // at first, check backup file path
        string srcPath = backupPath_ + it->data;
        if (!RingtoneFileUtils::IsFileExists(srcPath)) {
            if (it->sourceType == SOURCE_TYPE_PRESET) {
                it->restorePath = it->data;
                CheckSetting(*it);
            }
            RINGTONE_INFO_LOG("warnning:backup file is not exist, path=%{private}s", srcPath.c_str());
            it = infos.erase(it);
        } else {
            it++;
        }
    }
}

int32_t RingtoneRestore::StartRestore()
{
    if (restoreRdb_ == nullptr || backupPath_.empty()) {
        return E_FAIL;
    }
    auto ret = RingtoneRestoreBase::StartRestore();
    if (ret != E_OK) {
        return ret;
    }
    CheckNotRingtoneRestore();
    UpdateSettingInfos();
    auto infos = QueryFileInfos(INVALID_QUERY_OFFSET);
    if ((!infos.empty()) && (infos.size() != 0)) {
        CheckRestoreFileInfos(infos);
        ret = InsertTones(infos);
    }
    
    FlushSettings();
    return ret;
}

void RingtoneRestore::UpdateRestoreFileInfo(FileInfo &info)
{
    info.displayName = RingtoneFileUtils::GetFileNameFromPath(info.restorePath);
    if (info.title == TITLE_DEFAULT) {
        info.title = RingtoneFileUtils::GetBaseNameFromPath(info.restorePath);
    }

    struct stat statInfo;
    if (stat(info.restorePath.c_str(), &statInfo) != 0) {
        RINGTONE_ERR_LOG("stat syscall err %{public}d", errno);
        return;
    }
    info.dateModified = static_cast<int64_t>(RingtoneFileUtils::Timespec2Millisecond(statInfo.st_mtim));
}

bool RingtoneRestore::OnPrepare(FileInfo &info, const std::string &destPath)
{
    if (!RingtoneFileUtils::IsFileExists(destPath)) {
        return false;
    }

    string fileName = RingtoneFileUtils::GetFileNameFromPath(info.data);
    if (fileName.empty()) {
        RINGTONE_ERR_LOG("src file name is null");
        return false;
    }
    string baseName = RingtoneFileUtils::GetBaseNameFromPath(info.data);
    if (baseName.empty()) {
        RINGTONE_ERR_LOG("src file base name is null");
        return false;
    }
    if (!RingtoneFileUtils::IsTargetExtension(info.data)) {
        RINGTONE_INFO_LOG("invalid target baseName:%{public}s", baseName.c_str());
        return false;
    }
    string extensionName = RingtoneFileUtils::GetExtensionFromPath(info.data);

    int32_t repeatCount = 1;
    string srcPath = backupPath_ + info.data;
    info.restorePath = destPath + "/" + fileName;
    while (RingtoneFileUtils::IsFileExists(info.restorePath)) {
        if (RingtoneFileUtils::IsSameFile(srcPath, info.restorePath)) {
            CheckSetting(info);
            RINGTONE_ERR_LOG("samefile: srcPath=%{private}s, dstPath=%{private}s", srcPath.c_str(),
                info.restorePath.c_str());
            return false;
        }
        info.restorePath = destPath + "/" + baseName + "(" + to_string(repeatCount++) + ")" + "." + extensionName;
    }

    if (!RingtoneRestoreBase::MoveFile(srcPath, info.restorePath)) {
        return false;
    }

    UpdateRestoreFileInfo(info);

    return true;
}

void RingtoneRestore::OnFinished(vector<FileInfo> &infos)
{
    if (!RingtoneFileUtils::RemoveDirectory(backupPath_)) {
        RINGTONE_ERR_LOG("cleanup backup dir failed, restorepath=%{public}s, err: %{public}s",
            backupPath_.c_str(), strerror(errno));
    }
}

void RingtoneRestore::CheckNotRingtoneRestore()
{
    // SIMCARD_MODE_1 ring no ringtone
    if (RingtoneRestoreBase::DetermineNoRingtone(RINGTONE_COLUMN_RING_TONE_TYPE,
        RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, RING_TONE_TYPE_SIM_CARD_1, RING_TONE_TYPE_SIM_CARD_BOTH, restoreRdb_) &&
        RingtoneRestoreBase::NeedCommitSetting(RINGTONE_COLUMN_RING_TONE_TYPE,
            RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, RING_TONE_TYPE_SIM_CARD_1, RING_TONE_TYPE_SIM_CARD_BOTH)) {
        RINGTONE_INFO_LOG("no ringtone sound for ringtone sim card 1");
        RingtoneRestoreBase::SetNotRingtone(RINGTONE_COLUMN_RING_TONE_TYPE,
            RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, SIMCARD_MODE_1);
    }
    // SIMCARD_MODE_2 ring no ringtone
    if (RingtoneRestoreBase::DetermineNoRingtone(RINGTONE_COLUMN_RING_TONE_TYPE,
        RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, RING_TONE_TYPE_SIM_CARD_2, RING_TONE_TYPE_SIM_CARD_BOTH, restoreRdb_) &&
        RingtoneRestoreBase::NeedCommitSetting(RINGTONE_COLUMN_RING_TONE_TYPE,
            RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, RING_TONE_TYPE_SIM_CARD_2, RING_TONE_TYPE_SIM_CARD_BOTH)) {
        RINGTONE_INFO_LOG("no ringtone sound for ringtone sim card 2");
        RingtoneRestoreBase::SetNotRingtone(RINGTONE_COLUMN_RING_TONE_TYPE,
            RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, SIMCARD_MODE_2);
    }
    // SIMCARD_MODE_1 shot no ringtone
    if (RingtoneRestoreBase::DetermineNoRingtone(RINGTONE_COLUMN_SHOT_TONE_TYPE,
        RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SHOT_TONE_TYPE_SIM_CARD_1, SHOT_TONE_TYPE_SIM_CARD_BOTH, restoreRdb_) &&
        RingtoneRestoreBase::NeedCommitSetting(RINGTONE_COLUMN_SHOT_TONE_TYPE,
            RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SHOT_TONE_TYPE_SIM_CARD_1, SHOT_TONE_TYPE_SIM_CARD_BOTH)) {
        RINGTONE_INFO_LOG("no shot sound for shot sim card 1");
        RingtoneRestoreBase::SetNotRingtone(RINGTONE_COLUMN_SHOT_TONE_TYPE,
            RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SIMCARD_MODE_1);
    }
    // SIMCARD_MODE_2 shot no ringtone
    if (RingtoneRestoreBase::DetermineNoRingtone(RINGTONE_COLUMN_SHOT_TONE_TYPE,
        RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SHOT_TONE_TYPE_SIM_CARD_2, SHOT_TONE_TYPE_SIM_CARD_BOTH, restoreRdb_) &&
        RingtoneRestoreBase::NeedCommitSetting(RINGTONE_COLUMN_SHOT_TONE_TYPE,
            RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SHOT_TONE_TYPE_SIM_CARD_2, SHOT_TONE_TYPE_SIM_CARD_BOTH)) {
        RINGTONE_INFO_LOG("no shot sound for shot sim card 2");
        RingtoneRestoreBase::SetNotRingtone(RINGTONE_COLUMN_SHOT_TONE_TYPE,
            RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SIMCARD_MODE_2);
    }
    // notification
    if (RingtoneRestoreBase::DetermineNoRingtone(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE,
        RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, NOTIFICATION_TONE_TYPE, NOTIFICATION_TONE_TYPE, restoreRdb_) &&
        RingtoneRestoreBase::NeedCommitSetting(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE,
            RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, NOTIFICATION_TONE_TYPE, NOTIFICATION_TONE_TYPE)) {
        RINGTONE_INFO_LOG("no notification sound for notification");
        RingtoneRestoreBase::SetNotRingtone(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE,
            RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, SIMCARD_MODE_1);
    }
}
} // namespace Media
} // namespace OHOS
