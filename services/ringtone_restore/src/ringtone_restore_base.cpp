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
    settingMgr_->SetForceFlush(true);

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
    // 检查本地扫描器是否已完成，未完成则触发一次同步扫描
    shared_ptr<NativePreferences::Preferences> prefs =
        NativePreferences::PreferencesHelper::GetPreferences(COMMON_XML_EL1, errCode);
    if (!prefs) {
        RINGTONE_ERR_LOG("get preferences error: %{public}d", errCode);
        return E_FAIL;
    }
    int isCompleted = prefs->GetInt(RINGTONE_PARAMETER_SCANNER_COMPLETED_KEY,
        RINGTONE_PARAMETER_SCANNER_COMPLETED_FALSE);
    if (!isCompleted) {
        // 首次恢复时需要扫描本地preset铃声，确保ToneFiles表有初始数据
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

std::string GetCardBitTypeSqlString(const std::vector<int32_t> &types)
{
    if (types.empty()) {
        return "(0)";
    }
    std::string result = "(";
    for (size_t i = 0; i < types.size(); i++) {
        if (i > 0) {
            result += ", ";
        }
        result += std::to_string(types[i]);
    }
    result += ")";
    return result;
}

bool RingtoneRestoreBase::HasCustomisedSetting(const std::string &typeColumn, const std::string &sourceColumn,
    int cardBitType)
{
    string queryCountSql = "SELECT count(1) as count FROM " + RINGTONE_TABLE + " WHERE " + sourceColumn +
        " = 2 AND " + typeColumn + " IN " + GetCardBitTypeSqlString(GetCombinationsForCard(cardBitType)) + ";";
    int32_t count = RingtoneRestoreDbUtils::QueryInt(localRdb_, queryCountSql, "count");
    RINGTONE_INFO_LOG("HasCustomisedSetting: typeColumn=%{public}s,"
        "count=%{public}d", typeColumn.c_str(), count);
    return count > 0;
}

bool RingtoneRestoreBase::NeedCommitSetting(const std::string &typeColumn, const std::string &sourceColumn,
    int cardBitType)
{
    if (HasCustomisedSetting(typeColumn, sourceColumn, cardBitType)) {
        return false;
    }
    return !IsDetermineNoRingtone(typeColumn, sourceColumn, cardBitType, localRdb_);
}

bool RingtoneRestoreBase::IsDetermineNoRingtone(const std::string &typeColumn,
    const std::string &sourceColumn, int cardBitType, const std::shared_ptr<NativeRdb::RdbStore> &rdbStore)
{
    string queryCountSql = "SELECT count(1) as count FROM " + RINGTONE_TABLE + " WHERE " + sourceColumn +
        " > 0 AND " + typeColumn + " IN " + GetCardBitTypeSqlString(GetCombinationsForCard(cardBitType)) + ";";
    RINGTONE_DEBUG_LOG("queryCountSql: %{public}s", queryCountSql.c_str());
    int32_t count = RingtoneRestoreDbUtils::QueryInt(rdbStore, queryCountSql, "count");
    RINGTONE_DEBUG_LOG("got no ringtone count = %{public}d", count);
    if (count == 0) {
        RINGTONE_INFO_LOG("no ringtone sound");
        return true;
    }
    return false;
}

bool RingtoneRestoreBase::HasCustomisedNotification()
{
    string queryCountSql = "SELECT count(1) as count FROM " + RINGTONE_TABLE + " WHERE " +
        RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " = 2 AND " +
        RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " = " + std::to_string(NOTIFICATION_TONE_TYPE) + ";";
    int32_t count = RingtoneRestoreDbUtils::QueryInt(localRdb_, queryCountSql, "count");
    RINGTONE_INFO_LOG("HasCustomisedNotification: count=%{public}d", count);
    return count > 0;
}

// 有铃音的找不到就是无铃音
bool RingtoneRestoreBase::IsDetermineNoRingtoneForNotification(const std::shared_ptr<NativeRdb::RdbStore> &rdbStore)
{
    string queryCountSql = "SELECT count(1) as count FROM " + RINGTONE_TABLE + " WHERE " +
        RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE +" > 0 AND " +
        RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " = " + to_string(NOTIFICATION_TONE_TYPE) +  ";";
    RINGTONE_INFO_LOG("queryCountSql: %{public}s", queryCountSql.c_str());
    int32_t count = RingtoneRestoreDbUtils::QueryInt(rdbStore, queryCountSql, "count");
    RINGTONE_INFO_LOG("got no notification count = %{public}d", count);
    if (count == 0) {
        RINGTONE_INFO_LOG("no notification sound");
        return true;
    }
    return false;
}

bool RingtoneRestoreBase::NeedCommitSettingForNotification()
{
    if (HasCustomisedNotification()) {
        return false;
    }
    return !IsDetermineNoRingtoneForNotification(localRdb_);
}

bool RingtoneRestoreBase::HasCustomisedAlarm()
{
    string queryCountSql = "SELECT count(1) as count FROM " + RINGTONE_TABLE + " WHERE " +
        RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE + " = 2 AND " +
        RINGTONE_COLUMN_ALARM_TONE_TYPE + " = " + std::to_string(ALARM_TONE_TYPE) + ";";
    int32_t count = RingtoneRestoreDbUtils::QueryInt(localRdb_, queryCountSql, "count");
    RINGTONE_INFO_LOG("HasCustomisedAlarm: count=%{public}d", count);
    return count > 0;
}

// 有铃音的找不到就是无铃音
bool RingtoneRestoreBase::IsDetermineNoRingtoneForAlarm(const std::shared_ptr<NativeRdb::RdbStore> &rdbStore)
{
    string queryCountSql = "SELECT count(1) as count FROM " + RINGTONE_TABLE + " WHERE " +
        RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE +" > 0 AND " +
        RINGTONE_COLUMN_ALARM_TONE_TYPE + " = " + to_string(NOTIFICATION_TONE_TYPE) +  ";";
    RINGTONE_INFO_LOG("queryCountSql: %{public}s", queryCountSql.c_str());
    int32_t count = RingtoneRestoreDbUtils::QueryInt(rdbStore, queryCountSql, "count");
    RINGTONE_INFO_LOG("got no alarm count = %{public}d", count);
    if (count == 0) {
        RINGTONE_INFO_LOG("no alarm sound");
        return true;
    }
    return false;
}

bool RingtoneRestoreBase::NeedCommitSettingForAlarm()
{
    if (HasCustomisedAlarm()) {
        return false;
    }
    return !IsDetermineNoRingtoneForAlarm(localRdb_);
}

void RingtoneRestoreBase::CommitCardSettings(FileInfo &info, int32_t toneTypeBits, int32_t toneSourceType,
    const std::string &typeColumn, const std::string &sourceColumn, ToneSettingType settingType)
{
    // 没有任何卡位设置，标记为NotRingtone（该铃声不作为任何卡的铃声）
    if (!HasAnyCardSet(toneTypeBits)) {
        return;
    }

    static const std::vector<int32_t> cardMasks = {
        SIM_CARD_1_MASK, SIM_CARD_2_MASK, ESIM_CARD_1_MASK, ESIM_CARD_2_MASK};
    static const std::vector<int32_t> shotCardToneTypes = {SHOT_TONE_TYPE_SIM_CARD_1, SHOT_TONE_TYPE_SIM_CARD_2,
                                                           SHOT_TONE_TYPE_ESIM_CARD_1, SHOT_TONE_TYPE_ESIM_CARD_2};
    static const std::vector<int32_t> ringCardToneTypes = {RING_TONE_TYPE_SIM_CARD_1, RING_TONE_TYPE_SIM_CARD_2,
                                                           RING_TONE_TYPE_ESIM_CARD_1, RING_TONE_TYPE_ESIM_CARD_2};
    const auto &cardToneTypes = (settingType == TONE_SETTING_TYPE_SHOT) ? shotCardToneTypes : ringCardToneTypes;

    int32_t cardCount = GetSimCardCount(toneTypeBits);
    RINGTONE_DEBUG_LOG("CommitCardSettings cardCount = %{public}d", cardCount);
    if (cardCount > 1) {
        CommitMultiCardSettings(info, toneTypeBits, toneSourceType, typeColumn, sourceColumn,
            cardToneTypes, cardMasks, settingType);
    } else {
        CommitSingleCardSettings(info, toneTypeBits, toneSourceType, typeColumn, sourceColumn,
            cardToneTypes, cardMasks, settingType);
    }
}

void RingtoneRestoreBase::CommitMultiCardSettings(FileInfo &info, int32_t toneTypeBits, int32_t toneSourceType,
    const std::string &typeColumn, const std::string &sourceColumn,
    const std::vector<int32_t> &cardToneTypes, const std::vector<int32_t> &cardMasks, ToneSettingType settingType)
{
    uint32_t bits = static_cast<uint32_t>(toneTypeBits);
    for (size_t i = 0; i < cardMasks.size(); i++) {
        uint32_t mask = static_cast<uint32_t>(cardMasks[i]);
        if (!(bits & mask)) {
            continue;
        }
        if (NeedCommitSetting(typeColumn, sourceColumn, cardToneTypes[i])) {
            settingMgr_->CommitSetting(info.toneId, info.restorePath, settingType, cardToneTypes[i], toneSourceType);
            RINGTONE_INFO_LOG("CommitMultiCardSettings %{public}s as card/%{public}d, mask=%{public}d",
                typeColumn.c_str(), static_cast<int>(i + 1), cardMasks[i]);
        }
    }
}

void RingtoneRestoreBase::CommitSingleCardSettings(FileInfo &info, int32_t toneTypeBits, int32_t toneSourceType,
    const std::string &typeColumn, const std::string &sourceColumn,
    const std::vector<int32_t> &cardToneTypes, const std::vector<int32_t> &cardMasks, ToneSettingType settingType)
{
    uint32_t bits = static_cast<uint32_t>(toneTypeBits);
    for (size_t i = 0; i < cardMasks.size(); i++) {
        uint32_t mask = static_cast<uint32_t>(cardMasks[i]);
        if (!(bits & mask)) {
            continue;
        }
        if (NeedCommitSetting(typeColumn, sourceColumn, cardToneTypes[i])) {
            settingMgr_->CommitSetting(info.toneId, info.restorePath, settingType, cardToneTypes[i], toneSourceType);
            RINGTONE_INFO_LOG("CommitSingleCardSettings %{public}s as card/%{public}d single",
                typeColumn.c_str(), static_cast<int>(i + 1));
        }
        break;
    }
}

void RingtoneRestoreBase::CheckShotSetting(FileInfo &info)
{
    CommitCardSettings(info, info.shotToneType, info.shotToneSourceType,
        RINGTONE_COLUMN_SHOT_TONE_TYPE, RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, TONE_SETTING_TYPE_SHOT);
}

void RingtoneRestoreBase::CheckRingtoneSetting(FileInfo &info)
{
    CommitCardSettings(info, info.ringToneType, info.ringToneSourceType,
        RINGTONE_COLUMN_RING_TONE_TYPE, RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, TONE_SETTING_TYPE_RINGTONE);
}

void RingtoneRestoreBase::CheckSetting(FileInfo &info)
{
    RINGTONE_DEBUG_LOG("checking setting: %{public}s, vibrateInfo: soundMode=%{public}d, toneType=%{public}d, "
        "vibrateMode=%{public}d", info.toString().c_str(), info.vibrateInfo.soundMode,
        info.vibrateInfo.toneType, info.vibrateInfo.vibrateMode);
    // 更新SimCardSetting表中的震动设置
    CheckUpdateVibrateSetting(info);
    // 提交短信铃声的卡位设置
    CheckShotSetting(info);
    // 提交来电铃声的卡位设置
    CheckRingtoneSetting(info);

    // 通知铃声处理：只处理CUSTOMISED和INVALID两种情况，PRESET未处理
    if (info.notificationToneType == NOTIFICATION_TONE_TYPE &&
            info.notificationToneSourceType == SOURCE_TYPE_CUSTOMISED &&
            NeedCommitSettingForNotification()) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_NOTIFICATION,
            info.notificationToneType, info.notificationToneSourceType);
        RINGTONE_INFO_LOG("commit %{public}s as notificationTone", info.displayName.c_str());
    } else if (info.notificationToneType == NOTIFICATION_TONE_TYPE_NOT &&
            info.notificationToneSourceType == SOURCE_TYPE_INVALID) {
        RINGTONE_INFO_LOG("clear %{public}s notificationTone", info.displayName.c_str());
        ClearLocalDBNotificationRecordTypeSetting(info.restorePath); // 清空当前记录的type sourcetype两个字段
    }

    // 闹钟铃声处理：只处理CUSTOMISED和INVALID两种情况，PRESET未处理
    if (info.alarmToneType == ALARM_TONE_TYPE && info.alarmToneSourceType == SOURCE_TYPE_CUSTOMISED &&
            NeedCommitSettingForAlarm()) {
        settingMgr_->CommitSetting(info.toneId, info.restorePath, TONE_SETTING_TYPE_ALARM, info.alarmToneType,
            info.alarmToneSourceType);
        RINGTONE_INFO_LOG("commit %{public}s as alarmTone", info.displayName.c_str());
    } else if (info.alarmToneType == ALARM_TONE_TYPE_NOT &&
            info.alarmToneSourceType == SOURCE_TYPE_INVALID) {
        RINGTONE_INFO_LOG("clear %{public}s alarmTone", info.displayName.c_str());
        ClearLocalDBAlarmRecordTypeSetting(info.restorePath); // 清空当前记录的type sourcetype两个字段
    }
}

void RingtoneRestoreBase::CheckUpdateVibrateSetting(const FileInfo &info)
{
    CHECK_AND_RETURN(info.vibrateInfo.vibrateMode != VIBRATE_PLAYMODE_INVALID);
    
    RINGTONE_INFO_LOG("CheckUpdateVibrateSetting start: simcard=%{public}d, soundMode=%{public}d, "
        "toneType=%{public}d, settingType=%{public}d, vibrateMode=%{public}d, displayName=%{public}s",
        info.vibrateInfo.simcard, info.vibrateInfo.soundMode, info.vibrateInfo.toneType,
        info.vibrateInfo.settingType, info.vibrateInfo.vibrateMode, info.vibrateInfo.displayName.c_str());

    std::string toneData = QueryToneData(info);
    std::string vibrateName = GetVibrateFileName(info);
    std::string vibrateData = QueryVibrateData(vibrateName, info.vibrateInfo.vibrateMode);
    
    int32_t ringtoneType = CalcRingtoneType(info.vibrateInfo);
    SimcardSettingAsset asset = BuildSimcardSettingAsset(info, toneData, vibrateData, ringtoneType);
    
    UpdateSettingTable(asset);
    RINGTONE_DEBUG_LOG("CheckUpdateVibrateSetting end");
}

std::string RingtoneRestoreBase::QueryToneData(const FileInfo &info)
{
    std::string toneData = info.data;
    if (!toneData.empty()) {
        return toneData;
    }
    
    RINGTONE_INFO_LOG("incoming data is empty, get from db");
    NativeRdb::AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
    absRdbPredicates.EqualTo(RINGTONE_COLUMN_TONE_TYPE, info.vibrateInfo.toneType);
    absRdbPredicates.And();
    absRdbPredicates.NotEqualTo(RINGTONE_COLUMN_MEDIA_TYPE, RINGTONE_MEDIA_TYPE_INVALID);
    absRdbPredicates.And();
    absRdbPredicates.BeginsWith(RINGTONE_COLUMN_DISPLAY_NAME, info.displayName);
    return QuerySingleColumn(RINGTONE_COLUMN_DATA, absRdbPredicates);
}

std::string RingtoneRestoreBase::QueryVibrateData(const std::string &displayName, VibratePlayMode vibrateMode)
{
    if (vibrateMode == VIBRATE_PLAYMODE_NONE) {
        return {};
    }
    
    auto queryVibrateFunc = [&](const std::string &name) {
        NativeRdb::AbsRdbPredicates absRdbPredicates(VIBRATE_TABLE);
        absRdbPredicates.BeginsWith(VIBRATE_COLUMN_DISPLAY_NAME, name);
        absRdbPredicates.And();
        absRdbPredicates.EqualTo(VIBRATE_COLUMN_PLAY_MODE, VIBRATE_PLAYMODE_CLASSIC);
        absRdbPredicates.And();
        absRdbPredicates.In(VIBRATE_COLUMN_VIBRATE_TYPE, {VIBRATE_TYPE_STANDARD, VIBRATE_TYPE_SALARM,
            VIBRATE_TYPE_SRINGTONE, VIBRATE_TYPE_SNOTIFICATION});
        return QuerySingleColumn(VIBRATE_COLUMN_DATA, absRdbPredicates);
    };
    
    std::string vibrateData = queryVibrateFunc(displayName);
    if (vibrateData.empty()) {
        RINGTONE_INFO_LOG("invalid vibrate:%{public}s, searching for classic vibrate", displayName.c_str());
        vibrateData = queryVibrateFunc(VIBRATE_FILE_NAME_STD);
    }
    CHECK_AND_RETURN_RET_LOG(!vibrateData.empty(), {}, "vibrateData is empty");
    return vibrateData;
}

std::string RingtoneRestoreBase::GetVibrateFileName(const FileInfo &info)
{
    const std::string ext = RingtoneFileUtils::GetExtensionFromPath(info.vibrateInfo.displayName);
    return info.vibrateInfo.vibrateMode == VIBRATE_PLAYMODE_SYNC ?
        RingtoneUtils::ReplaceAll(info.displayName, ext, VIBRATE_FILE_SUFFIX) :
        info.vibrateInfo.displayName;
}

int32_t RingtoneRestoreBase::CalcRingtoneType(const VibrateFileInfo &vibrateInfo)
{
    int32_t ringtoneType = vibrateInfo.settingType;
    if (vibrateInfo.toneType == TONE_TYPE_ALARM) {
        ringtoneType = CalcRingtoneTypeByMode(vibrateInfo.soundMode, TONE_SETTING_TYPE_ALARM);
    } else if (vibrateInfo.toneType == TONE_TYPE_NOTIFICATION) {
        ringtoneType = CalcRingtoneTypeByMode(vibrateInfo.soundMode, TONE_SETTING_TYPE_NOTIFICATION);
    } else if (vibrateInfo.toneType == TONE_TYPE_RINGTONE) {
        ringtoneType = CalcRingtoneTypeByMode(vibrateInfo.soundMode, TONE_SETTING_TYPE_RINGTONE);
    } else if (vibrateInfo.toneType == TONE_TYPE_SHOT) {
        ringtoneType = CalcRingtoneTypeByMode(vibrateInfo.soundMode, TONE_SETTING_TYPE_SHOT);
    } else {
        RINGTONE_ERR_LOG("Calc ringtoneType: soundMode=%{public}d, toneType=%{public}d → ringtoneType=%{public}d",
            vibrateInfo.soundMode, vibrateInfo.toneType, ringtoneType);
    }
    RINGTONE_INFO_LOG("Calc ringtoneType: soundMode=%{public}d, toneType=%{public}d → ringtoneType=%{public}d",
        vibrateInfo.soundMode, vibrateInfo.toneType, ringtoneType);
    return ringtoneType;
}

SimcardSettingAsset RingtoneRestoreBase::BuildSimcardSettingAsset(const FileInfo &info,
    const std::string &toneData, const std::string &vibrateData, int32_t ringtoneType)
{
    SimcardSettingAsset asset;
    asset.SetMode(info.vibrateInfo.simcard);
    asset.SetRingMode(info.vibrateInfo.vibrateMode);
    asset.SetVibrateFile(vibrateData);
    asset.SetRingtoneType(ringtoneType);
    asset.SetToneFile(toneData);
    RINGTONE_INFO_LOG("BuildSimcardSettingAsset: mode=%{public}d, vibrateMode=%{public}d, "
        "ringtoneType=%{public}d, vibrateFile=%{public}s, toneFile=%{public}s",
        info.vibrateInfo.simcard, info.vibrateInfo.vibrateMode, ringtoneType,
        vibrateData.c_str(), toneData.c_str());
    return asset;
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

static void CheckSameFilePath(FileInfo &fileInfo, const vector<NativeRdb::ValuesBucket> &values)
{
    for (const auto& insertValue : values) {
        NativeRdb::ValueObject tmpValue;
        string restorePath {};
        if (insertValue.GetObject(RINGTONE_COLUMN_DATA, tmpValue)) {
            tmpValue.GetString(restorePath);
        }
        if (restorePath == fileInfo.restorePath && !restorePath.empty()) {
            RINGTONE_INFO_LOG("find same file path: %{private}s", restorePath.c_str());
            fileInfo.doInsert = false;
            break;
        }
    }
}

vector<NativeRdb::ValuesBucket> RingtoneRestoreBase::MakeInsertValues(std::vector<FileInfo> &fileInfos)
{
    vector<NativeRdb::ValuesBucket> values;
    for (auto it = fileInfos.begin(); it != fileInfos.end(); it++) {
        // 根据toneType获取目标目录（alarm/ringtone/notification/app_notification）
        auto destDir = GetRestoreDir(it->toneType);
        // OnPrepare：移动文件到目标目录，处理同名冲突，设置restorePath
        if (destDir.empty() || !OnPrepare(*it, destDir)) {
            continue;
        }

        // SetInsertValue：构建插入ValuesBucket（设置列使用硬编码默认值，存在缺陷）
        NativeRdb::ValuesBucket value = SetInsertValue(*it);
        if (value.IsEmpty()) {
            continue;
        }
        // SINGLE_CLONE场景跳过重复路径检查（源端文件路径不与本地冲突）
        if (sceneType_ != RESTORE_SCENE_TYPE_SINGLE_CLONE) {
            CheckSameFilePath(*it, values);
        }
        if (it->doInsert) { // 文件未重复时才加入插入列表
            values.emplace_back(value);
        }
        // 通过settingMgr_路径提交设置（震动、短信、来电、通知、闹钟）
        if (!it->skipSetting) {
            CheckSetting(*it);  // 自定义的铃声判断是否要保存设置 没有设置过任何铃声 用户设置(2) 无铃声 特殊判断 系统设置(1)
        }
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
    // 铃声设置信息：全部使用硬编码默认值，未使用FileInfo中从备份DB读取的正确值
    // 这导致SINGLE_CLONE场景下shot_tone_source_type等字段克隆不正确
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
        RINGTONE_ERR_LOG("column name invalid %{public}s", col.c_str());
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

void RingtoneRestoreBase::ClearLocalDBNotificationRecordTypeSetting(std::string restorePath)
{
    CHECK_AND_RETURN_LOG(localRdb_ != nullptr, "localRdb_ is null, can not set not ringtone");
    int32_t changeRows = 0;
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutInt(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE, NOTIFICATION_TONE_TYPE_NOT);
    valuesBucket.PutInt(RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, SOURCE_TYPE_NOT_SET);
    NativeRdb::AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
    string whereClause = RINGTONE_COLUMN_DATA + "= ? AND " +
        RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " != 2";
    vector<string> whereArgs;
    whereArgs.push_back(restorePath);
    absRdbPredicates.SetWhereClause(whereClause);
    absRdbPredicates.SetWhereArgs(whereArgs);
    localRdb_->Update(changeRows, valuesBucket, absRdbPredicates);
    RINGTONE_INFO_LOG("ClearLocalDBNotificationRecord update end changeRows = %{public}d", changeRows);
}

void RingtoneRestoreBase::ClearLocalDBAlarmRecordTypeSetting(std::string restorePath)
{
    CHECK_AND_RETURN_LOG(localRdb_ != nullptr, "localRdb_ is null, can not set not ringtone");
    int32_t changeRows = 0;
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutInt(RINGTONE_COLUMN_ALARM_TONE_TYPE, ALARM_TONE_TYPE_NOT);
    valuesBucket.PutInt(RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE, SOURCE_TYPE_NOT_SET);
    NativeRdb::AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
    string whereClause = RINGTONE_COLUMN_DATA + "= ? AND " +
        RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE + " != 2";
    vector<string> whereArgs;
    whereArgs.push_back(restorePath);
    absRdbPredicates.SetWhereClause(whereClause);
    absRdbPredicates.SetWhereArgs(whereArgs);
    localRdb_->Update(changeRows, valuesBucket, absRdbPredicates);
    RINGTONE_INFO_LOG("ClearLocalDBAlarmRecordTypeSetting update end changeRows = %{public}d", changeRows);
}

void RingtoneRestoreBase::SetNotRingtoneForNotification()
{
    CHECK_AND_RETURN_LOG(localRdb_ != nullptr, "localRdb_ is null, can not set not ringtone");
    int32_t changeRows = 0;
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutInt(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE, NOTIFICATION_TONE_TYPE_NOT);
    valuesBucket.PutInt(RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, SOURCE_TYPE_INVALID);
    NativeRdb::AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
    string whereClause = RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + "= ? AND " +
        RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " = 1";
    vector<string> whereArgs;
    whereArgs.push_back(to_string(NOTIFICATION_TONE_TYPE));
    absRdbPredicates.SetWhereClause(whereClause);
    absRdbPredicates.SetWhereArgs(whereArgs);
    localRdb_->Update(changeRows, valuesBucket, absRdbPredicates);
    RINGTONE_INFO_LOG("SetNotRingtoneForNotification update end changeRows = %{public}d", changeRows);
}

void RingtoneRestoreBase::SetNotRingtoneForAlarm()
{
    CHECK_AND_RETURN_LOG(localRdb_ != nullptr, "localRdb_ is null, can not set not ringtone");
    int32_t changeRows = 0;
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutInt(RINGTONE_COLUMN_ALARM_TONE_TYPE, ALARM_TONE_TYPE_NOT);
    valuesBucket.PutInt(RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE, SOURCE_TYPE_INVALID);
    NativeRdb::AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
    string whereClause = RINGTONE_COLUMN_ALARM_TONE_TYPE + "= ? AND " +
        RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE + " = 1";
    vector<string> whereArgs;
    whereArgs.push_back(to_string(ALARM_TONE_TYPE));
    absRdbPredicates.SetWhereClause(whereClause);
    absRdbPredicates.SetWhereArgs(whereArgs);
    localRdb_->Update(changeRows, valuesBucket, absRdbPredicates);
    RINGTONE_INFO_LOG("SetNotRingtoneForAlarm update end changeRows = %{public}d", changeRows);
}


void RingtoneRestoreBase::SetNotRingtoneForRingtone(int32_t cardBit)
{
    CHECK_AND_RETURN_LOG(cardBit == RING_TONE_TYPE_SIM_CARD_1 || cardBit == RING_TONE_TYPE_SIM_CARD_2 ||
        cardBit == RING_TONE_TYPE_ESIM_CARD_1 || cardBit == RING_TONE_TYPE_ESIM_CARD_2,
        "invalid cardBit: %{public}d", cardBit);
    CHECK_AND_RETURN_LOG(localRdb_ != nullptr, "localRdb_ is null, can not set not ringtone");
    RINGTONE_INFO_LOG("SetNotRingtoneForRingtone cardBit:%{public}d", cardBit);
    // UPDATE ToneFiles SET ring_tone_type = ring_tone_type & ~cardBit
    // WHERE ring_tone_source_type = 1 AND (ring_tone_type & cardBit) != 0
    uint32_t clearBit = ~static_cast<uint32_t>(cardBit);
    string updateSql = "UPDATE " + RINGTONE_TABLE + " SET " + RINGTONE_COLUMN_RING_TONE_TYPE +
        " = " + RINGTONE_COLUMN_RING_TONE_TYPE + " & " + to_string(clearBit) +
        " WHERE " + RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE + " = 1 AND (" +
        RINGTONE_COLUMN_RING_TONE_TYPE + " & " + to_string(cardBit) + ") != 0;";
    RINGTONE_INFO_LOG("SetNotRingtoneForRingtone sql: %{public}s", updateSql.c_str());
    int32_t changeRows = 0;
    localRdb_->ExecuteSql(updateSql);
    RINGTONE_INFO_LOG("SetNotRingtoneForRingtone end changeRows = %{public}d", changeRows);
}

void RingtoneRestoreBase::SetNotRingtoneForShot(int32_t cardBit)
{
    CHECK_AND_RETURN_LOG(cardBit == SHOT_TONE_TYPE_SIM_CARD_1 || cardBit == SHOT_TONE_TYPE_SIM_CARD_2 ||
        cardBit == SHOT_TONE_TYPE_ESIM_CARD_1 || cardBit == SHOT_TONE_TYPE_ESIM_CARD_2,
        "invalid cardBit: %{public}d", cardBit);
    CHECK_AND_RETURN_LOG(localRdb_ != nullptr, "localRdb_ is null, can not set not ringtone");
    RINGTONE_INFO_LOG("SetNotRingtoneForShot cardBit:%{public}d", cardBit);
    // UPDATE ToneFiles SET shot_tone_type = shot_tone_type & ~cardBit
    // WHERE shot_tone_source_type = 1 AND (shot_tone_type & cardBit) != 0
    uint32_t clearBit = ~static_cast<uint32_t>(cardBit);
    string updateSql = "UPDATE " + RINGTONE_TABLE + " SET " + RINGTONE_COLUMN_SHOT_TONE_TYPE +
        " = " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " & " + to_string(clearBit) +
        " WHERE " + RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + " = 1 AND (" +
        RINGTONE_COLUMN_SHOT_TONE_TYPE + " & " + to_string(cardBit) + ") != 0;";
    RINGTONE_INFO_LOG("SetNotRingtoneForShot sql: %{public}s", updateSql.c_str());
    int32_t changeRows = 0;
    localRdb_->ExecuteSql(updateSql);
    RINGTONE_INFO_LOG("SetNotRingtoneForShot end changeRows = %{public}d", changeRows);
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

bool RingtoneRestoreBase::IsRingtoneSet(ToneSettingType toneType, SimcardMode simcard)
{
    bool ret = false;
    std::string sourceColumn = RingtoneRestoreDbUtils::GetSourceColumnName(toneType);
    std::string simcardColumn = RingtoneRestoreDbUtils::GetModeColumnName(toneType);
    CHECK_AND_RETURN_RET_LOG(!sourceColumn.empty() && !simcardColumn.empty(), ret, "get column name failed");

    std::string querySql = "SELECT count(1) AS count FROM " + RINGTONE_TABLE +
        " WHERE " +  sourceColumn + " = " + std::to_string(SOURCE_TYPE_CUSTOMISED) +
        " AND " + simcardColumn + " IN (" + std::to_string(simcard) + " , " +
        std::to_string(SIMCARD_MODE_BOTH) + " )";
    int32_t count = RingtoneRestoreDbUtils::QueryInt(GetBaseDb(), querySql, "count");
    ret = count > 0;
    return ret;
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
    RINGTONE_INFO_LOG("UpdateSettingTable: mode=%{public}d, ringtoneType=%{public}d, ringMode=%{public}d, "
        "vibrateFile=%{public}s, forceUpdate=%{public}d", asset.GetMode(), asset.GetRingtoneType(),
        asset.GetRingMode(), asset.GetVibrateFile().c_str(), forceUpdate);
    localRdb_->Update(changeRows, valuesBucket, absRdbPredicates);
    RINGTONE_INFO_LOG("UpdateSettingTable result: changeRows=%{public}d", changeRows);
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