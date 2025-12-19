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
#include "ringtone_utils.h"
#include "ringtone_log.h"
#include "ringtone_mimetype_utils.h"
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
    string realPath = NativeRdb::RdbSqlUtils::GetDefaultDatabasePath(RINGTONE_LIBRARY_DB_PATH_EL1,
        RINGTONE_LIBRARY_DB_NAME, errCode);
    int32_t err = RingtoneRestoreDbUtils::InitDb(localRdb_, RINGTONE_LIBRARY_DB_PATH_EL1, realPath,
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
    int32_t errCode = RingtoneMimeTypeUtils::InitMimeTypeMap();
    if (errCode != E_OK) {
        RINGTONE_ERR_LOG("get mine type map error: %{public}d", errCode);
        return errCode;
    }
    shared_ptr<NativePreferences::Preferences> prefs =
        NativePreferences::PreferencesHelper::GetPreferences(COMMON_XML_EL1, errCode);
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
    if (count > 0) {
        return false;
    }
    return !DetermineNoRingtone(typeColumn, sourceColumn, type, allSetType, localRdb_);
}

bool RingtoneRestoreBase::DetermineNoRingtone(const std::string &typeColumn,
    const std::string &sourceColumn, int type, int allSetType, const std::shared_ptr<NativeRdb::RdbStore> &rdbStore)
{
    string queryCountSql = "SELECT count(1) as count FROM " + RINGTONE_TABLE + " WHERE " + sourceColumn +
        " > 0 AND ( " + typeColumn + " = " + to_string(type) + " OR " + typeColumn + " = " +
        to_string(allSetType) + " );";
    RINGTONE_INFO_LOG("queryCountSql: %{public}s", queryCountSql.c_str());
    int32_t count = RingtoneRestoreDbUtils::QueryInt(rdbStore, queryCountSql, "count");
    RINGTONE_INFO_LOG("got no ringtone count = %{public}d", count);
    if (count == 0) {
        RINGTONE_INFO_LOG("no ringtone sound");
        return true;
    }
    return false;
}

void RingtoneRestoreBase::CheckShotSetting(FileInfo &info)
{
    if (info.shotToneType == SHOT_TONE_TYPE_SIM_CARD_BOTH && info.shotToneSourceType == SOURCE_TYPE_CUSTOMISED) {
        if (NeedCommitSetting(RINGTONE_COLUMN_SHOT_TONE_TYPE, RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE,
            SHOT_TONE_TYPE_SIM_CARD_1, SHOT_TONE_TYPE_SIM_CARD_BOTH)) {
            settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_SHOT,
                SHOT_TONE_TYPE_SIM_CARD_1, info.shotToneSourceType);
            RINGTONE_INFO_LOG("commit as card1 shottone");
        }
        if (NeedCommitSetting(RINGTONE_COLUMN_SHOT_TONE_TYPE, RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE,
            SHOT_TONE_TYPE_SIM_CARD_2, SHOT_TONE_TYPE_SIM_CARD_BOTH)) {
            settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_SHOT,
                SHOT_TONE_TYPE_SIM_CARD_2, info.shotToneSourceType);
            RINGTONE_INFO_LOG("commit as card2 shottone");
        }
    } else if ((info.shotToneType > SHOT_TONE_TYPE_NOT) && (info.shotToneType < SHOT_TONE_TYPE_SIM_CARD_BOTH) &&
            (info.shotToneSourceType == SOURCE_TYPE_CUSTOMISED) && NeedCommitSetting(RINGTONE_COLUMN_SHOT_TONE_TYPE,
            RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, info.shotToneType, SHOT_TONE_TYPE_SIM_CARD_BOTH)) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_SHOT, info.shotToneType,
            info.shotToneSourceType);
        RINGTONE_INFO_LOG("commit as shottone");
    } else if (info.shotToneType == SHOT_TONE_TYPE_NOT &&
            info.shotToneSourceType == SOURCE_TYPE_INVALID) {
        SetNotRingtone(RINGTONE_COLUMN_SHOT_TONE_TYPE, RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, info.simcard);
    }
}

void RingtoneRestoreBase::CheckRingtoneSetting(FileInfo &info)
{
    if (info.ringToneType == RING_TONE_TYPE_SIM_CARD_BOTH && info.ringToneSourceType == SOURCE_TYPE_CUSTOMISED) {
        if (NeedCommitSetting(RINGTONE_COLUMN_RING_TONE_TYPE, RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE,
            RING_TONE_TYPE_SIM_CARD_1, RING_TONE_TYPE_SIM_CARD_BOTH)) {
            settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_RINGTONE,
                RING_TONE_TYPE_SIM_CARD_1, info.ringToneSourceType);
            RINGTONE_INFO_LOG("commit as card1 ringtone");
        }
        if (NeedCommitSetting(RINGTONE_COLUMN_RING_TONE_TYPE, RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE,
            RING_TONE_TYPE_SIM_CARD_2, RING_TONE_TYPE_SIM_CARD_BOTH)) {
            settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_RINGTONE,
                RING_TONE_TYPE_SIM_CARD_2, info.ringToneSourceType);
            RINGTONE_INFO_LOG("commit as card2 ringtone");
        }
    } else if (info.ringToneType > RING_TONE_TYPE_NOT && info.ringToneType < RING_TONE_TYPE_SIM_CARD_BOTH &&
            info.ringToneSourceType == SOURCE_TYPE_CUSTOMISED && NeedCommitSetting(RINGTONE_COLUMN_RING_TONE_TYPE,
            RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, info.ringToneType, RING_TONE_TYPE_SIM_CARD_BOTH)) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_RINGTONE, info.ringToneType,
            info.ringToneSourceType);
        RINGTONE_INFO_LOG("commit as ringtone");
    } else if (info.ringToneType == RING_TONE_TYPE_NOT &&
            info.ringToneSourceType == SOURCE_TYPE_INVALID) {
        SetNotRingtone(RINGTONE_COLUMN_RING_TONE_TYPE, RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, info.simcard);
    }
}

void RingtoneRestoreBase::CheckSetting(FileInfo &info)
{
    RINGTONE_INFO_LOG("checking setting: %{public}s", info.toString().c_str());
    CheckUpdateVibrateSetting(info);
    CheckShotSetting(info);
    CheckRingtoneSetting(info);

    if (info.notificationToneType == NOTIFICATION_TONE_TYPE &&
            info.notificationToneSourceType == SOURCE_TYPE_CUSTOMISED &&
            NeedCommitSetting(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE,
            RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, info.notificationToneType, NOTIFICATION_TONE_TYPE)) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_NOTIFICATION,
            info.notificationToneType, info.notificationToneSourceType);
        RINGTONE_INFO_LOG("commit as notificationTone");
    } else if (info.notificationToneType == NOTIFICATION_TONE_TYPE_NOT &&
            info.notificationToneSourceType == SOURCE_TYPE_INVALID) {
        SetNotRingtone(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE,
            RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, info.simcard);
    }

    if (info.alarmToneType == ALARM_TONE_TYPE && info.alarmToneSourceType == SOURCE_TYPE_CUSTOMISED &&
            NeedCommitSetting(RINGTONE_COLUMN_ALARM_TONE_TYPE, RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE,
            info.alarmToneType, ALARM_TONE_TYPE)) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_ALARM, info.alarmToneType,
            info.alarmToneSourceType);
        RINGTONE_INFO_LOG("commit as alarmTone");
    } else if (info.alarmToneType == ALARM_TONE_TYPE_NOT &&
            info.alarmToneSourceType == SOURCE_TYPE_INVALID) {
        SetNotRingtone(RINGTONE_COLUMN_ALARM_TONE_TYPE, RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE, info.simcard);
    }
}

void RingtoneRestoreBase::CheckUpdateVibrateSetting(const FileInfo &info)
{
    CHECK_AND_RETURN_LOG(info.vibrateInfo.vibrateMode != VIBRATE_PLAYMODE_INVALID, "vibrate mode is invalid");
    const std::string ext = RingtoneFileUtils::GetExtensionFromPath(info.vibrateInfo.displayName);
    std::string vibrateName = info.vibrateInfo.vibrateMode == VIBRATE_PLAYMODE_SYNC ?
        RingtoneUtils::ReplaceAll(info.displayName, ext, VIBRATE_FILE_SUFFIX) :
        info.vibrateInfo.displayName;
    RINGTONE_INFO_LOG("ringtoneFile=%{public}s, vibrateFile=%{public}s, vibrateMode=%{public}d",
        info.displayName.c_str(), vibrateName.c_str(), info.vibrateInfo.vibrateMode);

    std::string toneData = info.data;
    if (toneData.empty()) {
        RINGTONE_INFO_LOG("incoming data is empty, get from db");
        NativeRdb::AbsRdbPredicates absRdbPredicates1(RINGTONE_TABLE);
        absRdbPredicates1.EqualTo(RINGTONE_COLUMN_TONE_TYPE, info.vibrateInfo.toneType);
        absRdbPredicates1.And();
        absRdbPredicates1.NotEqualTo(RINGTONE_COLUMN_MEDIA_TYPE, RINGTONE_MEDIA_TYPE_INVALID);
        absRdbPredicates1.And();
        absRdbPredicates1.BeginsWith(RINGTONE_COLUMN_DISPLAY_NAME, info.displayName);
        toneData = QuerySingleColumn(RINGTONE_COLUMN_DATA, absRdbPredicates1);
    }

    std::string vibrateData{};
    if (info.vibrateInfo.vibrateMode != VIBRATE_PLAYMODE_NONE) {
        auto queryVibrateFunc = [&](const std::string &displayName) {
            NativeRdb::AbsRdbPredicates absRdbPredicates2(VIBRATE_TABLE);
            absRdbPredicates2.BeginsWith(VIBRATE_COLUMN_DISPLAY_NAME, displayName);
            absRdbPredicates2.And();
            absRdbPredicates2.EqualTo(VIBRATE_COLUMN_PLAY_MODE, VIBRATE_PLAYMODE_CLASSIC);
            absRdbPredicates2.And();
            absRdbPredicates2.In(VIBRATE_COLUMN_VIBRATE_TYPE, {VIBRATE_TYPE_STANDARD, VIBRATE_TYPE_SALARM,
                VIBRATE_TYPE_SRINGTONE, VIBRATE_TYPE_SNOTIFICATION});
            vibrateData = QuerySingleColumn(VIBRATE_COLUMN_DATA, absRdbPredicates2);
        };
        queryVibrateFunc(vibrateName);
        if (vibrateData.empty()) {
            RINGTONE_INFO_LOG("invalid vibrate:%{public}s, searching for classic vibrate",
                vibrateName.c_str());
            queryVibrateFunc(VIBRATE_FILE_NAME_STD);
        }
        CHECK_AND_RETURN_LOG(!vibrateData.empty(), "vibrateData is empty");
    }
    SimcardSettingAsset asset;
    asset.SetMode(info.vibrateInfo.simcard);
    asset.SetRingMode(info.vibrateInfo.vibrateMode);
    asset.SetVibrateFile(vibrateData);
    asset.SetRingtoneType(info.vibrateInfo.settingType);
    asset.SetToneFile(toneData);
    UpdateSettingTable(asset);
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
        path = RINGTONE_CUSTOMIZED_ALARM_PATH;
    } else if (toneType == TONE_TYPE_RINGTONE) {
        path = RINGTONE_CUSTOMIZED_RINGTONE_PATH;
    } else if (toneType == TONE_TYPE_NOTIFICATION) {
        path = RINGTONE_CUSTOMIZED_NOTIFICATIONS_PATH;
    } else if (toneType == TONE_TYPE_APP_NOTIFICATION) {
        path = RINGTONE_CUSTOMIZED_APP_NOTIFICATIONS_PATH;
    } else {
        path = {};
    }

    // check ringtone dir
    if (!path.empty() && access(path.c_str(), F_OK) != 0) {
        if (!RingtoneFileUtils::CreateDirectory(path)) {
            RINGTONE_ERR_LOG("Create customised tone dir: %{private}s failed!", path.c_str());
        }
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
    values.PutInt(RINGTONE_COLUMN_SCANNER_FLAG, fileInfo.scannerFlag);
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

void RingtoneRestoreBase::SetNotRingtone(const string &columnType, const string &columnSourceType, int32_t simCard)
{
    CHECK_AND_RETURN_LOG(simCard != SIMCARD_MODE_INVALID, "invalid simcard");
    CHECK_AND_RETURN_LOG(localRdb_ != nullptr, "localRdb_ is null, can not set not ringtone");
    RINGTONE_INFO_LOG("columnType:%{public}s, columnSourceType:%{public}s, sim:%{public}d",
        columnType.c_str(), columnSourceType.c_str(), simCard);
    int32_t changeRows = 0;
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutInt(columnType, RING_TONE_TYPE_NOT);
    valuesBucket.PutInt(columnSourceType, SOURCE_TYPE_INVALID);
    NativeRdb::AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
    string whereClause = columnType + "= ? AND " + columnSourceType + " = 1";
    vector<string> whereArgs;
    whereArgs.push_back(to_string(simCard));
    absRdbPredicates.SetWhereClause(whereClause);
    absRdbPredicates.SetWhereArgs(whereArgs);
    localRdb_->Update(changeRows, valuesBucket, absRdbPredicates);
    RINGTONE_INFO_LOG("update end changeRows = %{public}d", changeRows);

    CHECK_AND_RETURN_LOG(columnType != RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE, "set ok for notification tone");
    if (simCard == SIMCARD_MODE_1) {
        simCard = SIMCARD_MODE_2;
    } else {
        simCard = SIMCARD_MODE_1;
    }
    changeRows = 0;
    NativeRdb::ValuesBucket valuesBucketBoth;
    valuesBucketBoth.PutInt(columnType, simCard);
    valuesBucketBoth.PutInt(columnSourceType, SOURCE_TYPE_INVALID);
    NativeRdb::AbsRdbPredicates absRdbPredicatesBoth(RINGTONE_TABLE);
    string whereClauseBoth = columnType + "= ? AND " + columnSourceType + " = 1";
    vector<string> whereArgsBoth;
    whereArgsBoth.push_back(to_string(SIMCARD_MODE_BOTH));
    absRdbPredicatesBoth.SetWhereClause(whereClauseBoth);
    absRdbPredicatesBoth.SetWhereArgs(whereArgsBoth);
    localRdb_->Update(changeRows, valuesBucketBoth, absRdbPredicatesBoth);
    RINGTONE_INFO_LOG("update both end changeRows = %{public}d", changeRows);
}

int32_t RingtoneRestoreBase::GetRingtoneLimit(RingtoneMediaType mediaType)
{
    std::string querySql = "SELECT count(1) AS count FROM " + RINGTONE_TABLE + " WHERE " +  RINGTONE_COLUMN_MEDIA_TYPE +
        " = " + std::to_string(mediaType);
    int32_t count = RingtoneRestoreDbUtils::QueryInt(GetBaseDb(), querySql, "count");
    int32_t limit = RINGTONE_VIDEO_MAX_COUNT;
    if (count >= 0) {
        limit = std::max(RINGTONE_VIDEO_MAX_COUNT - count, 0);
    }
    return limit;
}

void RingtoneRestoreBase::UpdateSettingTable(const SimcardSettingAsset &asset, bool forceUpdate)
{
    CHECK_AND_RETURN_LOG(localRdb_ != nullptr, "localRdb_ is null");
    int32_t changeRows = 0;
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutInt(SIMCARD_SETTING_COLUMN_RING_MODE, asset.GetRingMode());
    valuesBucket.PutString(SIMCARD_SETTING_COLUMN_VIBRATE_FILE, asset.GetVibrateFile());
    if (!asset.GetToneFile().empty()) {
        valuesBucket.PutString(SIMCARD_SETTING_COLUMN_TONE_FILE, asset.GetToneFile());
    }
    valuesBucket.PutInt(SIMCARD_SETTING_COLUMN_VIBRATE_MODE, VIBRATE_TYPE_STANDARD);
    NativeRdb::AbsRdbPredicates absRdbPredicates(SIMCARD_SETTING_TABLE);
    string whereClause = SIMCARD_SETTING_COLUMN_MODE + "= ? AND " + SIMCARD_SETTING_COLUMN_RINGTONE_TYPE + " = ? ";
    if (!forceUpdate) {
        whereClause += " AND " + SIMCARD_SETTING_COLUMN_VIBRATE_MODE + " IS NULL ";
    }
    vector<string> whereArgs;
    whereArgs.push_back(to_string(asset.GetMode()));
    whereArgs.push_back(to_string(asset.GetRingtoneType()));
    absRdbPredicates.SetWhereClause(whereClause);
    absRdbPredicates.SetWhereArgs(whereArgs);
    localRdb_->Update(changeRows, valuesBucket, absRdbPredicates);
    RINGTONE_INFO_LOG("update end changeRows = %{public}d", changeRows);
}

std::string RingtoneRestoreBase::QuerySingleColumn(const std::string &columnName,
    const NativeRdb::AbsRdbPredicates &predicates)
{
    std::string ret{};
    CHECK_AND_RETURN_RET_LOG(localRdb_ != nullptr, ret, "localRdb_ is null");
    auto resultSet = localRdb_->Query(predicates, {columnName});
    CHECK_AND_RETURN_RET_LOG(resultSet != nullptr, ret, "resultSet is null");
    if (resultSet->GoToFirstRow() != NativeRdb::E_OK) {
        resultSet->Close();
        RINGTONE_INFO_LOG("resultSet is empty");
        return ret;
    }
    ret = GetStringVal(columnName, resultSet);
    resultSet->Close();
    return ret;
}

} // namespace Media
} // namespace OHOS
