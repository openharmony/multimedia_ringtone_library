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

#define MLOG_TAG "RingtoneRestoreBase"

#include "datashare_ext_ability.h"
#include "datashare_ext_ability_context.h"
#include "rdb_helper.h"
#include "rdb_sql_utils.h"
#include "result_set.h"
#include "ringtone_restore_type.h"
#include "ringtone_restore_db_utils.h"
#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_rdb_transaction.h"
#include "ringtone_restore_base.h"

namespace OHOS {
namespace Media {
using namespace std;
int32_t RingtoneRestoreBase::Init(const string &backupPath)
{
    if (localRdb_ != nullptr) {
        return E_OK;
    }
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        RINGTONE_ERR_LOG("Failed to get context");
        return E_FAIL;
    }
    int32_t errCode = 0;
    string realPath = NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(context->GetDatabaseDir(),
        RINGTONE_LIBRARY_DB_NAME, errCode);
    int32_t err = RingtoneRestoreDbUtils::InitDb(localRdb_, RINGTONE_LIBRARY_DB_NAME, realPath,
        context->GetBundleName(), true);
    if (err != E_OK) {
        RINGTONE_ERR_LOG("medialibrary rdb fail, err = %{public}d", err);
        return E_FAIL;
    }
    settingMgr_ = make_unique<RingtoneSettingManager>(localRdb_);
    if (settingMgr_ == nullptr) {
        RINGTONE_ERR_LOG("create ringtone setting manager failed");
        return E_FAIL;
    }

    return E_OK;
}

int32_t RingtoneRestoreBase::MoveFile(const std::string &srcFile, const std::string &dstFile)
{
    if (RingtoneFileUtils::MoveFile(srcFile, dstFile)) {
        return E_OK;
    }

    if (!RingtoneFileUtils::CopyFileUtil(srcFile, dstFile)) {
        RINGTONE_ERR_LOG("CopyFile failed, filePath: %{private}s, errmsg: %{public}s", srcFile.c_str(),
            strerror(errno));
        return E_FAIL;
    }
    (void)RingtoneFileUtils::DeleteFile(srcFile);
    return E_OK;
}

void RingtoneRestoreBase::CheckSetting(FileInfo &info)
{
    if ((info.shotToneType > SHOT_TONE_TYPE_NOT) && (info.shotToneType < SHOT_TONE_TYPE_MAX) &&
            (info.shotToneSourceType == SOURCE_TYPE_CUSTOMISED)) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_SHOT, info.shotToneType,
                info.shotToneSourceType);
    }
    if (info.ringToneType > RING_TONE_TYPE_NOT && info.ringToneType < RING_TONE_TYPE_MAX &&
            info.ringToneSourceType == SOURCE_TYPE_CUSTOMISED) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_RINGTONE, info.ringToneType,
                info.ringToneSourceType);
    }
    if (info.notificationToneType == NOTIFICATION_TONE_TYPE &&
            info.notificationToneSourceType == SOURCE_TYPE_CUSTOMISED) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_NOTIFICATION,
                info.notificationToneType, info.notificationToneSourceType);
    }
    if (info.alarmToneType == ALARM_TONE_TYPE && info.alarmToneSourceType == SOURCE_TYPE_CUSTOMISED) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_ALARM, info.alarmToneType,
                info.alarmToneSourceType);
    }
}

void RingtoneRestoreBase::InsertTones(std::vector<FileInfo> &fileInfos)
{
    if (localRdb_ == nullptr) {
        RINGTONE_ERR_LOG("localRdb_ is null");
        return;
    }
    if (fileInfos.empty()) {
        RINGTONE_ERR_LOG("fileInfos are empty");
        return;
    }
    vector<NativeRdb::ValuesBucket> values = MakeInsertValues(fileInfos);
    int64_t rowNum = 0;
    int32_t errCode = BatchInsert(RINGTONE_TABLE, values, rowNum);
    if (errCode != E_OK) {
        return;
    }

    settingMgr_->FlushSettings();

    OnFinished(fileInfos);
}

int32_t RingtoneRestoreBase::BatchInsert(const std::string &tableName, std::vector<NativeRdb::ValuesBucket> &values,
    int64_t &rowNum)
{
    int32_t errCode = E_ERR;
    RingtoneRdbTransaction transactionOprn(localRdb_);
    errCode = transactionOprn.Start();
    if (errCode != E_OK) {
        RINGTONE_ERR_LOG("can not get rdb before batch insert");
        return errCode;
    }
    errCode = localRdb_->BatchInsert(rowNum, tableName, values);
    if (errCode != E_OK) {
        RINGTONE_ERR_LOG("InsertSql failed, errCode: %{public}d, rowNum: %{public}ld.", errCode, (long)rowNum);
        return errCode;
    }
    transactionOprn.Finish();
    return errCode;
}

string RingtoneRestoreBase::GetRestoreDir(const int32_t toneType)
{
    string path = {};
    if (toneType == ToneType::TONE_TYPE_ALARM) {
         path = RINGTONE_RESTORE_DIR + "/" + "alarms";
    } else if (toneType == TONE_TYPE_RINGTONE) {
         path = RINGTONE_RESTORE_DIR + "/" + "ringtones";
    } else if (toneType == TONE_TYPE_NOTIFICATION) {
         path = RINGTONE_RESTORE_DIR + "/" + "notifications";
    } else {
        path = {};
    }
    return path;
}

vector<NativeRdb::ValuesBucket> RingtoneRestoreBase::MakeInsertValues(std::vector<FileInfo> &fileInfos)
{
    vector<NativeRdb::ValuesBucket> values;
    for (auto it = fileInfos.begin(); it != fileInfos.end(); it++) {
        auto destDir = GetRestoreDir(it->toneType);
        if (destDir.empty() || !OnPrepare(*it, destDir)) {
            continue;
        }

        NativeRdb::ValuesBucket value = SetInsertValue(*it);
        if (value.IsEmpty()) {
            continue;
        }
        values.emplace_back(value);
        CheckSetting(*it);
    }
    return values;
}

NativeRdb::ValuesBucket RingtoneRestoreBase::SetInsertValue(const FileInfo &fileInfo)
{
    if (fileInfo.restorePath.empty() || fileInfo.data.empty()) {
        return {};
    }
    NativeRdb::ValuesBucket values;
    values.PutString(RINGTONE_COLUMN_DATA, fileInfo.restorePath);
    values.PutInt(RINGTONE_COLUMN_SIZE, fileInfo.size);
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, fileInfo.displayName);
    values.PutString(RINGTONE_COLUMN_TITLE, fileInfo.title);
    values.PutInt(RINGTONE_COLUMN_MEDIA_TYPE, fileInfo.mediaType);
    values.PutInt(RINGTONE_COLUMN_TONE_TYPE, fileInfo.toneType);
    values.PutString(RINGTONE_COLUMN_MIME_TYPE, fileInfo.mimeType);
    values.PutInt(RINGTONE_COLUMN_SOURCE_TYPE, fileInfo.sourceType);
    values.PutLong(RINGTONE_COLUMN_DATE_ADDED, fileInfo.dateAdded);
    values.PutLong(RINGTONE_COLUMN_DATE_MODIFIED, fileInfo.dateModified);
    values.PutLong(RINGTONE_COLUMN_DATE_TAKEN, fileInfo.dateTaken);
    values.PutInt(RINGTONE_COLUMN_DURATION, fileInfo.duration);
    // ringtone setting infos
    values.PutInt(RINGTONE_COLUMN_SHOT_TONE_TYPE, SHOT_TONE_TYPE_DEFAULT);
    values.PutInt(RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SHOT_TONE_SOURCE_TYPE_DEFAULT);
    values.PutInt(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE, NOTIFICATION_TONE_TYPE_DEFAULT);
    values.PutInt(RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, NOTIFICATION_TONE_SOURCE_TYPE_DEFAULT);
    values.PutInt(RINGTONE_COLUMN_RING_TONE_TYPE, RING_TONE_TYPE_DEFAULT);
    values.PutInt(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, RING_TONE_SOURCE_TYPE_DEFAULT);
    values.PutInt(RINGTONE_COLUMN_ALARM_TONE_TYPE, ALARM_TONE_TYPE_DEFAULT);
    values.PutInt(RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE, ALARM_TONE_SOURCE_TYPE_DEFAULT);

    return values;
}

int32_t RingtoneRestoreBase::MoveDirectory(const std::string &srcDir, const std::string &dstDir)
{
    if (!RingtoneFileUtils::CreateDirectory(dstDir)) {
        RINGTONE_ERR_LOG("Create dstDir %{private}s failed", dstDir.c_str());
        return E_FAIL;
    }
    for (const auto &dirEntry : std::filesystem::directory_iterator{ srcDir }) {
        std::string srcFilePath = dirEntry.path();
        std::string tmpFilePath = srcFilePath;
        std::string dstFilePath = tmpFilePath.replace(0, srcDir.length(), dstDir);
        if (MoveFile(srcFilePath, dstFilePath) != E_OK) {
            RINGTONE_ERR_LOG("Move file from %{private}s to %{private}s failed", srcFilePath.c_str(), dstFilePath.c_str());
            return E_FAIL;
        }
    }
    return E_OK;
}
} // namespace Media
} // namespace OHOS
