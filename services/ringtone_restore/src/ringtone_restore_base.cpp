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

#include "ringtone_restore_base.h"

#include "datashare_ext_ability.h"
#include "datashare_ext_ability_context.h"
#include "rdb_helper.h"
#include "rdb_sql_utils.h"
#include "result_set.h"
#include "result_set_utils.h"
#include "ringtone_restore_type.h"
#include "ringtone_restore_db_utils.h"
#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_rdb_transaction.h"
#include "ringtone_rdbstore.h"
#include "ringtone_scanner_manager.h"
#include "preferences_helper.h"
#include "dfx_const.h"

namespace OHOS {
namespace Media {
using namespace std;

static const char RINGTONE_PARAMETER_SCANNER_COMPLETED_KEY[] = "ringtone.scanner.completed";
static const int RINGTONE_PARAMETER_SCANNER_COMPLETED_TRUE = 1;
static const int RINGTONE_PARAMETER_SCANNER_COMPLETED_FALSE = 0;

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
    
    auto rdbStore = RingtoneRdbStore::GetInstance(context);
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("rdbStore initialization failed");
        return E_RDB;
    }

    int32_t errCode = 0;
    string realPath = NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(RINGTONE_LIBRARY_DB_PATH,
        RINGTONE_LIBRARY_DB_NAME, errCode);
    int32_t err = RingtoneRestoreDbUtils::InitDb(localRdb_, RINGTONE_LIBRARY_DB_NAME, realPath,
        RINGTONE_BUNDLE_NAME, true);
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

int32_t RingtoneRestoreBase::StartRestore()
{
    RingtoneFileUtils::AccessRingtoneDir();
    int32_t errCode;
    shared_ptr<NativePreferences::Preferences> prefs =
        NativePreferences::PreferencesHelper::GetPreferences(DFX_COMMON_XML, errCode);
    if (!prefs) {
        RINGTONE_ERR_LOG("get preferences error: %{public}d", errCode);
        return E_FAIL;
    }
    int isCompleted = prefs->GetInt(RINGTONE_PARAMETER_SCANNER_COMPLETED_KEY,
        RINGTONE_PARAMETER_SCANNER_COMPLETED_FALSE);
    if (!isCompleted) {
        RingtoneScannerManager::GetInstance()->Start(true);
        prefs->PutInt(RINGTONE_PARAMETER_SCANNER_COMPLETED_KEY, RINGTONE_PARAMETER_SCANNER_COMPLETED_TRUE);
        prefs->FlushSync();
    }
    return E_OK;
}

bool RingtoneRestoreBase::MoveFile(const std::string &src, const std::string &dst)
{
    if (RingtoneFileUtils::MoveFile(src, dst)) {
        return true;
    }

    if (!RingtoneFileUtils::CopyFileUtil(src, dst)) {
        RINGTONE_ERR_LOG("copy-file failed, src: %{public}s, err: %{public}s", src.c_str(), strerror(errno));
        return false;
    }

    if (!RingtoneFileUtils::DeleteFile(src)) {
        RINGTONE_ERR_LOG("remove-file failed, filePath: %{public}s, err: %{public}s", src.c_str(), strerror(errno));
    }

    return true;
}

bool RingtoneRestoreBase::NeedCommitSetting(const std::string &typeColumn, const std::string &sourceColumn,
    int type, int allSetType)
{
    auto type2Str = std::to_string(type);
    auto allSetTypeStr = std::to_string(allSetType);
    // allSetType is for those settings with both sim cards
    string queryCountSql = "SELECT count(1) as count FROM " + RINGTONE_TABLE + " WHERE " +  sourceColumn +
        " = 2 AND (" + typeColumn + " = " + type2Str + " OR " + typeColumn + " = " + allSetTypeStr + " );";
    RINGTONE_INFO_LOG("queryCountSql: %{public}s", queryCountSql.c_str());

    int32_t count = RingtoneRestoreDbUtils::QueryInt(localRdb_, queryCountSql, "count");
    RINGTONE_INFO_LOG("got count = %{public}d", count);

    return !(count>0);
}

void RingtoneRestoreBase::CheckSetting(FileInfo &info)
{
    RINGTONE_INFO_LOG("checking setting: %{public}s", info.toString().c_str());
    if ((info.shotToneType > SHOT_TONE_TYPE_NOT) && (info.shotToneType < SHOT_TONE_TYPE_MAX) &&
            (info.shotToneSourceType == SOURCE_TYPE_CUSTOMISED) && NeedCommitSetting(RINGTONE_COLUMN_SHOT_TONE_TYPE,
            RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, info.shotToneType, SHOT_TONE_TYPE_SIM_CARD_BOTH)) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_SHOT, info.shotToneType,
            info.shotToneSourceType);
        RINGTONE_INFO_LOG("commit as shottone");
    }
    if (info.ringToneType > RING_TONE_TYPE_NOT && info.ringToneType < RING_TONE_TYPE_MAX &&
            info.ringToneSourceType == SOURCE_TYPE_CUSTOMISED && NeedCommitSetting(RINGTONE_COLUMN_RING_TONE_TYPE,
            RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, info.ringToneType, RING_TONE_TYPE_SIM_CARD_BOTH)) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_RINGTONE, info.ringToneType,
            info.ringToneSourceType);
        RINGTONE_INFO_LOG("commit as ringtone");
    }
    if (info.notificationToneType == NOTIFICATION_TONE_TYPE &&
            info.notificationToneSourceType == SOURCE_TYPE_CUSTOMISED &&
            NeedCommitSetting(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE,
            RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, info.notificationToneType, NOTIFICATION_TONE_TYPE)) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_NOTIFICATION,
            info.notificationToneType, info.notificationToneSourceType);
        RINGTONE_INFO_LOG("commit as notificationTone");
    }
    if (info.alarmToneType == ALARM_TONE_TYPE && info.alarmToneSourceType == SOURCE_TYPE_CUSTOMISED &&
            NeedCommitSetting(RINGTONE_COLUMN_ALARM_TONE_TYPE, RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE,
            info.alarmToneType, ALARM_TONE_TYPE)) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_ALARM, info.alarmToneType,
            info.alarmToneSourceType);
        RINGTONE_INFO_LOG("commit as alarmTone");
    }
}

int32_t RingtoneRestoreBase::InsertTones(std::vector<FileInfo> &fileInfos)
{
    if (localRdb_ == nullptr) {
        RINGTONE_ERR_LOG("localRdb_ is null");
        return E_FAIL;
    }
    if (fileInfos.empty()) {
        RINGTONE_ERR_LOG("fileInfos are empty, not need restore");
        return E_OK;
    }
    vector<NativeRdb::ValuesBucket> values = MakeInsertValues(fileInfos);
    int64_t rowNum = 0;
    int32_t errCode = BatchInsert(RINGTONE_TABLE, values, rowNum);
    if (errCode != E_OK) {
        RINGTONE_ERR_LOG("fail to batch insert");
        return errCode;
    }

    OnFinished(fileInfos);
    return E_OK;
}

void RingtoneRestoreBase::FlushSettings()
{
    if (settingMgr_ != nullptr) {
        settingMgr_->FlushSettings();
    } else {
        RINGTONE_ERR_LOG("ringtone setting mgr is nullptr");
    }
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
        if (it->doInsert) { // only when this value needs to be inserted
            values.emplace_back(value);
        }
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
        if (!MoveFile(srcFilePath, dstFilePath)) {
            RINGTONE_ERR_LOG("Move file from %{private}s to %{private}s failed", srcFilePath.c_str(),
                dstFilePath.c_str());
            return E_FAIL;
        }
    }
    return E_OK;
}

void RingtoneRestoreBase::ExtractMetaFromColumn(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<RingtoneMetadata> &metadata, const std::string &col)
{
    RingtoneResultSetDataType dataType = RingtoneResultSetDataType::DATA_TYPE_NULL;
    RingtoneMetadata::RingtoneMetadataFnPtr requestFunc = nullptr;
    auto itr = metadata->memberFuncMap_.find(col);
    if (itr != metadata->memberFuncMap_.end()) {
        dataType = itr->second.first;
        requestFunc = itr->second.second;
    } else {
        RINGTONE_ERR_LOG("column name invalid %{private}s", col.c_str());
        return;
    }

    std::variant<int32_t, std::string, int64_t, double> data =
        ResultSetUtils::GetValFromColumn<const shared_ptr<NativeRdb::ResultSet>>(col, resultSet, dataType);

    if (requestFunc != nullptr) {
        (metadata.get()->*requestFunc)(data);
    }
}

int32_t RingtoneRestoreBase::PopulateMetadata(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<RingtoneMetadata> &metaData)
{
    std::vector<std::string> columnNames;
    int32_t err = resultSet->GetAllColumnNames(columnNames);
    if (err != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get all column names");
        return E_RDB;
    }

    for (const auto &col : columnNames) {
        ExtractMetaFromColumn(resultSet, metaData, col);
    }

    return E_OK;
}
} // namespace Media
} // namespace OHOS
