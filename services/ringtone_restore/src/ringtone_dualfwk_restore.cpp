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

#define MLOG_TAG "RingtoneDualFwkRestore"

#include "ringtone_dualfwk_restore.h"

#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sstream>

#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "directory_ex.h"
#include "dualfwk_conf_parser.h"
#include "dualfwk_conf_loader.h"
#include "dualfwk_sound_setting.h"
#include "iservice_registry.h"
#include "result_set_utils.h"
#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_restore_db_utils.h"
#include "ringtone_restore_base.h"
#include "ringtone_restore_type.h"
#include "ringtone_type.h"
#include "ringtone_utils.h"
#include "customised_tone_processor.h"
#ifdef CORE_SERVICE_ENABLE
#include "core_service_client.h"
#include "telephony_errors.h"
#endif
#ifdef USE_MEDIA_LIBRARY
#include "media_library_manager.h"
#endif

namespace OHOS {
namespace Media {
using namespace std;

static const string DUALFWK_SOUND_CONF_XML = "backup";
const int32_t SIM_LABLE_INDEX_TOW = 2;

int32_t RingtoneDualFwkRestore::LoadDualFwkConf(const std::string &backupPath)
{
    DualFwkConfLoader confLoader;
    if (confLoader.Init() != E_OK) {
        RINGTONE_ERR_LOG("Failed to initialize DualFwkConfLoader.");
        return E_FAIL;
    }
    DualFwkConf conf;
    confLoader.Load(conf, RESTORE_SCENE_TYPE_DUAL_UPGRADE, backupPath);
    confLoader.ShowConf(conf);

    dualFwkSetting_ = std::make_unique<DualFwkSoundSetting>();
    if (dualFwkSetting_ == nullptr) {
        RINGTONE_ERR_LOG("Create DualFwkSoundSetting Failed.");
        return E_FAIL;
    }

    dualFwkSetting_->ProcessConf(conf);
    return E_SUCCESS;
}

int32_t RingtoneDualFwkRestore::ParseDualFwkConf(const string &xml)
{
    auto parser = std::make_unique<DualFwkConfParser>(xml);
    if (parser == nullptr) {
        RINGTONE_ERR_LOG("Create DualFwkConfParser Failed.");
        return E_FAIL;
    }

    dualFwkSetting_ = std::make_unique<DualFwkSoundSetting>();
    if (dualFwkSetting_ == nullptr) {
        RINGTONE_ERR_LOG("Create DualFwkSoundSetting Failed.");
        return E_FAIL;
    }

    if (parser->Parse() != E_SUCCESS) {
        RINGTONE_ERR_LOG("parse dualfwk-sound-setting-xml Failed.");
        return E_FAIL;
    }

    parser->ConfTraval([this](std::unique_ptr<DualFwkConfRow> &conf) -> void {
        dualFwkSetting_->ProcessConfRow(conf);
    });

    return E_SUCCESS;
}

int32_t RingtoneDualFwkRestore::Init(const std::string &backupPath)
{
    RINGTONE_INFO_LOG("Init db start");
    if (backupPath.empty()) {
        RINGTONE_ERR_LOG("error: backup path is null");
        return E_INVALID_ARGUMENTS;
    }

    if (LoadDualFwkConf(backupPath + "/" + DUALFWK_SOUND_CONF_XML) != E_SUCCESS) {
        return E_FAIL;
    }

    if (RingtoneRestoreBase::Init(backupPath) != E_OK) {
        return E_FAIL;
    }

    RINGTONE_INFO_LOG("Init db successfully");
    return E_OK;
}

static const string KEY_API_VERSION = "API_VERSION";
static std::string MakeBatchQueryWhereClause(const std::vector<std::string> &names,
    const std::string &predicateColumn)
{
    std::stringstream prefixSs;
    prefixSs << predicateColumn << " in (";
    bool start = true;
    for (const auto& name: names) {
        if (start) {
            start = false;
        } else {
            prefixSs << ",";
        }
        prefixSs << "\"" << name << "\"";
    }
    prefixSs << ")";
    return prefixSs.str();
}

int32_t RingtoneDualFwkRestore::QueryRingToneDbForFileInfo(std::shared_ptr<NativeRdb::RdbStore> rdbStore,
    const std::vector<std::string> &names, std::map<std::string, std::vector<std::shared_ptr<FileInfo>>> &infoMap,
    const std::string &predicateColumn)
{
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("rdb_ is nullptr, Maybe init failed.");
        return E_FAIL;
    }
    string whereClause = MakeBatchQueryWhereClause(names, predicateColumn);
    std::string queryCountSql = "SELECT * FROM " + RINGTONE_TABLE +
        " WHERE " + whereClause +  " AND " +
        RINGTONE_COLUMN_MEDIA_TYPE + "!=" + std::to_string(RINGTONE_MEDIA_TYPE_INVALID) + ";";
    RINGTONE_INFO_LOG("Querying ringtonedb where %{public}s", whereClause.c_str());

    auto resultSet = rdbStore->QuerySql(queryCountSql);
    if (resultSet == nullptr) {
        RINGTONE_WARN_LOG("resultset for %{public}s is null", whereClause.c_str());
        return E_FAIL;
    }
    int count = 0;
    resultSet->GetRowCount(count);
    RINGTONE_INFO_LOG("Querying ringtonedb where %{public}s, got %{public}d records",
        whereClause.c_str(), count);

    if (count <= 0) {
        RINGTONE_WARN_LOG("resultset for %{public}s is empty", whereClause.c_str());
        return E_SUCCESS;
    }

    for (int i = 0; i < count; i++) {
        resultSet->GoToNextRow();
        auto metaData = std::make_unique<RingtoneMetadata>();
        if (PopulateMetadata(resultSet, metaData) != E_OK) {
            return E_FAIL;
        }
        auto infoPtr = std::make_shared<FileInfo>(*metaData);
        infoPtr->doInsert = false;
        if (predicateColumn == RINGTONE_COLUMN_TITLE) {
            infoMap[infoPtr->title].emplace_back(infoPtr);
        } else {
            infoMap[infoPtr->displayName].emplace_back(infoPtr);
        }
        
        RINGTONE_INFO_LOG("new info found in ringtone_lib: %{public}s", infoPtr->toString().c_str()); // debug
    }

    for (const auto& results : infoMap) {
        if (!results.second.empty()) {
            for (const auto& infoPtr : results.second) {
                RINGTONE_INFO_LOG("key: %{public}s, value: %{public}s", results.first.c_str(),
                    infoPtr->toString().c_str());
            }
        } else {
            RINGTONE_INFO_LOG("key: %{public}s, value is nullptr", results.first.c_str());
        }
    }

    return E_SUCCESS;
}

static void AddSettingsToFileInfo(const DualFwkSettingItem &setting, FileInfo &info)
{
    int32_t sourceType = setting.toneSetting.toneSourceType == SOURCE_TYPE_INVALID ?
        SOURCE_TYPE_INVALID : SOURCE_TYPE_CUSTOMISED;
    info.simcard = setting.toneSetting.simcard;
    info.vibrateInfo = setting.vibrateSetting;
    switch (setting.toneSetting.settingType) {
        case TONE_SETTING_TYPE_ALARM:
            info.toneType = TONE_TYPE_ALARM;
            info.alarmToneType = setting.toneSetting.toneType;
            info.alarmToneSourceType = sourceType;
            break;
        case TONE_SETTING_TYPE_RINGTONE:
            info.toneType = TONE_TYPE_RINGTONE;
            info.ringToneType = setting.toneSetting.toneType;
            info.ringToneSourceType = sourceType;
            break;
        case TONE_SETTING_TYPE_SHOT:
            info.toneType = TONE_TYPE_NOTIFICATION;
            info.shotToneType = setting.toneSetting.toneType;
            info.shotToneSourceType = sourceType;
            break;
        case TONE_SETTING_TYPE_NOTIFICATION:
            info.toneType = TONE_TYPE_NOTIFICATION;
            info.notificationToneType = setting.toneSetting.toneType;
            info.notificationToneSourceType = sourceType;
            break;
        default:
            break;
    }
}

bool IsRingtoneConsistent(const std::shared_ptr<FileInfo> &ringtoneInfo, const DualFwkSettingItem &setting)
{
    auto keyToneType = setting.toneSetting.settingType;
    return ((ringtoneInfo->toneType == ToneType::TONE_TYPE_ALARM &&
        keyToneType == ToneSettingType::TONE_SETTING_TYPE_ALARM) ||
        (ringtoneInfo->toneType == ToneType::TONE_TYPE_RINGTONE &&
        keyToneType == ToneSettingType::TONE_SETTING_TYPE_RINGTONE) ||
        (ringtoneInfo->toneType == ToneType::TONE_TYPE_NOTIFICATION &&
        keyToneType == ToneSettingType::TONE_SETTING_TYPE_NOTIFICATION) ||
        (ringtoneInfo->toneType == ToneType::TONE_TYPE_NOTIFICATION &&
        keyToneType == ToneSettingType::TONE_SETTING_TYPE_SHOT));
}

std::shared_ptr<FileInfo> GetRingtonebyDisplayName(std::vector<std::shared_ptr<FileInfo>>& results,
    const DualFwkSettingItem &setting, bool &doInsert)
{
    auto keyName = setting.toneSetting.tonePath;
    std::shared_ptr<FileInfo> infoPtr;
    for (const auto& ringtoneInfo : results) {
        if (ringtoneInfo == nullptr) {
            RINGTONE_ERR_LOG("ringtoneInfo from ringtone by display name is nullptr");
            continue;
        }
        if (ringtoneInfo->sourceType == SourceType::SOURCE_TYPE_PRESET) {
            if (IsRingtoneConsistent(ringtoneInfo, setting)) {
                infoPtr = ringtoneInfo;
                RINGTONE_INFO_LOG("found %{public}s in ringtone db", keyName.c_str());
                doInsert = false;
                break;
            } else {
                RINGTONE_INFO_LOG("%{public}s is invalid", keyName.c_str());
                continue;
            }
        } else {
            infoPtr = ringtoneInfo;
            RINGTONE_INFO_LOG("found %{public}s in ringtone db", keyName.c_str());
            doInsert = false;
            break;
        }
    }
    return infoPtr;
}

std::shared_ptr<FileInfo> GetRingtoneInfo(std::vector<std::shared_ptr<FileInfo>>& results,
    const DualFwkSettingItem &setting, bool &doInsert)
{
    auto keyName = setting.toneSetting.tonePath;
    std::shared_ptr<FileInfo> infoPtr;
    for (const auto& ringtoneInfo : results) {
        if (ringtoneInfo == nullptr) {
            RINGTONE_ERR_LOG("ringtoneInfo from ringtone by display name is nullptr");
            continue;
        }
        infoPtr = ringtoneInfo;
        RINGTONE_INFO_LOG("found %{public}s in ringtone db", keyName.c_str());
        doInsert = false;
        break;
    }
    return infoPtr;
}

static std::shared_ptr<FileInfo> MergeQueries(const DualFwkSettingItem &setting,
    std::map<std::string, std::shared_ptr<FileInfo>> resultFromFileMgr,
    std::map<std::string, std::vector<std::shared_ptr<FileInfo>>> resultFromRingtoneByDisplayName,
    bool &doInsert)
{
    std::shared_ptr<FileInfo> infoPtr;
    std::vector<std::shared_ptr<FileInfo>> results;
    doInsert = true;
    auto keyName = setting.toneSetting.tonePath;
    if (resultFromFileMgr.find(keyName) != resultFromFileMgr.end()) {
        bool hasSameTone = resultFromRingtoneByDisplayName.find(keyName) != resultFromRingtoneByDisplayName.end();
        if (hasSameTone) {
            results = resultFromRingtoneByDisplayName.at(keyName);
            GetRingtoneInfo(results, setting, doInsert);
        }

        infoPtr = resultFromFileMgr[keyName];
        RINGTONE_INFO_LOG("found %{public}s in filemgr", keyName.c_str());
    } else if (resultFromRingtoneByDisplayName.find(keyName) != resultFromRingtoneByDisplayName.end()) {
        results = resultFromRingtoneByDisplayName.at(keyName);
        if (results.empty()) {
            RINGTONE_ERR_LOG("query ringtone from ringtoneDb by display name is null");
            return nullptr;
        }
        infoPtr = GetRingtonebyDisplayName(results, setting, doInsert);
    } else if (setting.isTitle) {
        keyName = RingtoneUtils::ReplaceAll(keyName + ".ogg", " ", "_");
        if (resultFromRingtoneByDisplayName.find(keyName) != resultFromRingtoneByDisplayName.end()) {
            results = resultFromRingtoneByDisplayName.at(keyName);
            if (results.empty()) {
                RINGTONE_ERR_LOG("iquery ringtone from ringtoneDb by display name is null");
                return nullptr;
            }
            infoPtr = GetRingtoneInfo(results, setting, doInsert);
        }
    } else {
        RINGTONE_INFO_LOG("failed to find %{public}s", keyName.c_str());
    }
    return infoPtr;
}

std::vector<FileInfo> RingtoneDualFwkRestore::BuildFileInfo()
{
    std::vector<FileInfo> result;
    std::map<std::string, std::shared_ptr<FileInfo>> resultFromFileMgr;
    CustomisedToneProcessor customisedToneProcessor;

    int32_t limit = GetRingtoneLimit(RINGTONE_MEDIA_TYPE_VIDEO);
    customisedToneProcessor.SetVideoToneLimit(limit);
    customisedToneProcessor.QueryFileMgrForFileInfo(resultFromFileMgr);

    std::vector<std::string> displayNames = dualFwkSetting_->GetDisplayNames();
    std::map<std::string, std::vector<std::shared_ptr<FileInfo>>> resultFromRingtoneByDisplayName;

    QueryRingToneDbForFileInfo(GetBaseDb(), displayNames, resultFromRingtoneByDisplayName, "display_name");

    for (const auto& setting : dualFwkSetting_->GetSettings()) {
        bool doInsert = true;
        auto infoPtr = MergeQueries(setting, resultFromFileMgr, resultFromRingtoneByDisplayName, doInsert);
        if (infoPtr == nullptr) {
            continue;
        }
        FileInfo info = *infoPtr;
        info.doInsert = doInsert;
        if (IsRingtoneSet(setting.toneSetting.settingType, setting.toneSetting.simcard)) {
            info.doInsert = false;
            info.skipSetting = true;
        }

        AddSettingsToFileInfo(setting, info);
        result.push_back(info);

        RINGTONE_INFO_LOG("push back into results -----> %{private}s", info.toString().c_str());
    }
    return result;
}

int32_t RingtoneDualFwkRestore::StartRestore()
{
    if (dualFwkSetting_ == nullptr) {
        RINGTONE_ERR_LOG("dualfwk restrore is not initialized successfully");
        return E_ERR;
    }
    auto ret = RingtoneRestoreBase::StartRestore();
    if (ret != E_OK) {
        return ret;
    }

    ret = HandleUpgradeWithESim();
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("HandleUpgradeWithESim failed, ret=%{public}d", ret);
    }

    std::vector<FileInfo> infos = BuildFileInfo();

    if ((!infos.empty()) && (infos.size() != 0)) {
        ret = InsertTones(infos);
    }
    FlushSettings();
    return ret;
}

int32_t RingtoneDualFwkRestore::DupToneFile(FileInfo &info)
{
    if (RingtoneFileUtils::IsFileExists(info.restorePath)) {
        return E_SUCCESS;
    }

    RINGTONE_INFO_LOG("DupToneFile from %{private}s to %{private}s", info.data.c_str(), info.restorePath.c_str());
    std::string absDstPath = info.restorePath;
    RINGTONE_INFO_LOG("converted dst path from %{private}s to realpath %{private}s", info.restorePath.c_str(),
        absDstPath.c_str());

    std::string absSrcPath = info.data;

    std::string::size_type isCustomisedToneFile = absSrcPath.find(FILE_MANAGER_BASE_PATH);
    if (isCustomisedToneFile != std::string::npos) {
        if (RingtoneFileUtils::CopyFileUtil(absSrcPath, absDstPath)) {
            return E_SUCCESS;
        }
        RINGTONE_ERR_LOG("copy file fail, src: %{public}s, dest: %{public}s", absSrcPath.c_str(), absDstPath.c_str());
    }

    std::string sub = "cloud";
    std::string replacement = "media/local";
    auto found = absSrcPath.find(sub);
    if (found != string::npos) {
        absSrcPath.replace(found, sub.size(), replacement);
        RINGTONE_INFO_LOG("converted src path from %{public}s to realpath %{public}s",
            info.data.c_str(), absSrcPath.c_str());

        if (!RingtoneFileUtils::CopyFileUtil(absSrcPath, absDstPath)) {
            RINGTONE_ERR_LOG("copy-file failed, src: %{public}s, err: %{public}s", absSrcPath.c_str(),
                strerror(errno));
            return E_FAIL;
        }
    } else {
        RINGTONE_INFO_LOG("no need to copy file.");
        info.restorePath = absSrcPath;
    }

    return E_SUCCESS;
}

void RingtoneDualFwkRestore::UpdateRestoreFileInfo(FileInfo &info)
{
    struct stat statInfo;
    if (stat(info.restorePath.c_str(), &statInfo) != 0) {
        RINGTONE_ERR_LOG("stat syscall err %{public}d", errno);
        return;
    }
    info.dateModified = static_cast<int64_t>(RingtoneFileUtils::Timespec2Millisecond(statInfo.st_mtim));
    info.displayName = RingtoneFileUtils::GetFileNameFromPath(info.restorePath);
}

bool RingtoneDualFwkRestore::OnPrepare(FileInfo &info, const std::string &dstPath)
{
    if (!RingtoneFileUtils::IsFileExists(dstPath)) {
        RINGTONE_ERR_LOG("dst path is not existing, dst path=%{public}s", dstPath.c_str());
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

    string extensionName = RingtoneFileUtils::GetExtensionFromPath(info.data);
    int32_t repeatCount = 1;
    info.restorePath = dstPath + "/" + fileName;
    while (RingtoneFileUtils::IsFileExists(info.restorePath)) {
        struct stat dstStatInfo {};
        if (stat(info.restorePath.c_str(), &dstStatInfo) != 0) {
            RINGTONE_ERR_LOG("Failed to get file %{private}s StatInfo, err=%{public}d",
                info.restorePath.c_str(), errno);
            return false;
        }
        if (info.size == dstStatInfo.st_size) {
            RINGTONE_INFO_LOG("samefile: srcPath=%{private}s, dstPath=%{private}s", info.data.c_str(),
                info.restorePath.c_str());
            break;
        }
        info.restorePath = dstPath + "/" + baseName + "(" + to_string(repeatCount++) + ")" + "." + extensionName;
    }

    if (DupToneFile(info) != E_SUCCESS) {
        return false;
    }

    UpdateRestoreFileInfo(info);

    return true;
}

void RingtoneDualFwkRestore::OnFinished(vector<FileInfo> &infos)
{
    RINGTONE_ERR_LOG("ringtone dualfwk restore finished");
}

CardScenarioType RingtoneDualFwkRestore::AnalyzeCardScenario(bool hasSim1, bool hasSim2, bool hasESim1, bool hasESim2)
{
    int32_t cardCount = static_cast<int32_t>(hasSim1) + static_cast<int32_t>(hasSim2) +
        static_cast<int32_t>(hasESim1) + static_cast<int32_t>(hasESim2);
    RINGTONE_INFO_LOG("Card scenario analysis: sim1=%{public}d, sim2=%{public}d, esim1=%{public}d, esim2=%{public}d, "
        "total=%{public}d", hasSim1, hasSim2, hasESim1, hasESim2, cardCount);

    if (cardCount == 0) {
        RINGTONE_INFO_LOG("No active SIM cards found");
        return SCENARIO_INVALID;
    }

    if (cardCount == 1) {
        if (hasESim1) return SCENARIO_ESIM1_ONLY;
        if (hasESim2) return SCENARIO_ESIM2_ONLY;
        if (hasSim1) return SCENARIO_SIM1_ONLY;
        if (hasSim2) return SCENARIO_SIM2_ONLY;
    } else if (cardCount == SIM_LABLE_INDEX_TOW) {
        if (hasSim1 && hasSim2) return SCENARIO_SIM1_SIM2;
        if (hasSim1 && hasESim1) return SCENARIO_SIM1_ESIM1;
        if (hasSim1 && hasESim2) return SCENARIO_SIM1_ESIM2;
        if (hasSim2 && hasESim1) return SCENARIO_SIM2_ESIM1;
        if (hasSim2 && hasESim2) return SCENARIO_SIM2_ESIM2;
        if (hasESim1 && hasESim2) return SCENARIO_ESIM1_ESIM2;
    }

    return SCENARIO_INVALID;
}

void RingtoneDualFwkRestore::MigrateToneType(int32_t fromCardMask, int32_t toCardMask)
{
    RINGTONE_INFO_LOG("Migrating tone type from mask=%{public}d to mask=%{public}d", fromCardMask, toCardMask);
    auto rdbStore = GetBaseDb();
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("GetBaseDb failed, cannot migrate tone type");
        return;
    }

    std::string updateShotToneSql =
        "UPDATE " + RINGTONE_TABLE + " SET " +
        RINGTONE_COLUMN_SHOT_TONE_TYPE + " = " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " | ?" +
        " WHERE " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " & ? != 0";
    int32_t ret = rdbStore->ExecuteSql(updateShotToneSql,
        { NativeRdb::ValueObject(toCardMask), NativeRdb::ValueObject(fromCardMask) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("Update shot_tone_type failed, ret=%{public}d", ret);
    }

    std::string updateRingToneSql =
        "UPDATE " + RINGTONE_TABLE + " SET " +
        RINGTONE_COLUMN_RING_TONE_TYPE + " = " + RINGTONE_COLUMN_RING_TONE_TYPE + " | ?" +
        " WHERE " + RINGTONE_COLUMN_RING_TONE_TYPE + " & ? != 0";
    ret = rdbStore->ExecuteSql(updateRingToneSql,
        { NativeRdb::ValueObject(toCardMask), NativeRdb::ValueObject(fromCardMask) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("Update ring_tone_type failed, ret=%{public}d", ret);
    }
}

void RingtoneDualFwkRestore::MigrateSimCardSetting(int32_t fromMode, int32_t toMode)
{
    RINGTONE_INFO_LOG("Migrating SimCardSetting from mode=%{public}d to mode=%{public}d", fromMode, toMode);
    auto rdbStore = GetBaseDb();
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("GetBaseDb failed, cannot migrate SimCardSetting");
        return;
    }
    std::string migrateSql =
        "INSERT OR REPLACE INTO " + SIMCARD_SETTING_TABLE + " (" +
        SIMCARD_SETTING_COLUMN_MODE + ", " +
        SIMCARD_SETTING_COLUMN_RINGTONE_TYPE + ", " +
        SIMCARD_SETTING_COLUMN_TONE_FILE + ", " +
        SIMCARD_SETTING_COLUMN_VIBRATE_FILE + ", " +
        SIMCARD_SETTING_COLUMN_VIBRATE_MODE + ", " +
        SIMCARD_SETTING_COLUMN_RING_MODE + ") " +
        "SELECT ?, " +
        SIMCARD_SETTING_COLUMN_RINGTONE_TYPE + ", " +
        SIMCARD_SETTING_COLUMN_TONE_FILE + ", " +
        SIMCARD_SETTING_COLUMN_VIBRATE_FILE + ", " +
        SIMCARD_SETTING_COLUMN_VIBRATE_MODE + ", " +
        SIMCARD_SETTING_COLUMN_RING_MODE + " " +
        "FROM " + SIMCARD_SETTING_TABLE + " " +
        "WHERE " + SIMCARD_SETTING_COLUMN_MODE + " = ?";
    int32_t ret = rdbStore->ExecuteSql(migrateSql,
        { NativeRdb::ValueObject(toMode), NativeRdb::ValueObject(fromMode) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("Migrate SimCardSetting failed, ret=%{public}d", ret);
    }
}

void RingtoneDualFwkRestore::ApplyMigration(int32_t fromCardMask, int32_t toCardMask)
{
    MigrateToneType(fromCardMask, toCardMask);
    MigrateSimCardSetting(GetSimcardModeFromCardMask(fromCardMask), GetSimcardModeFromCardMask(toCardMask));
}

int32_t RingtoneDualFwkRestore::HandleUpgradeWithESim()
{
    RINGTONE_INFO_LOG("HandleUpgradeWithESim start");
#ifdef CORE_SERVICE_ENABLE
    std::vector<Telephony::IccAccountInfo> telIccAccountInfoList;
    int32_t ret = Telephony::CoreServiceClient::GetInstance().GetActiveSimAccountInfoList(telIccAccountInfoList);
    if ((ret != Telephony::TELEPHONY_ERR_SUCCESS) && (ret != Telephony::TELEPHONY_ERR_NO_SIM_CARD)) {
        RINGTONE_ERR_LOG("GetActiveSimAccountInfoList error, ret = %{public}d.", ret);
        return ret;
    }

    bool hasSim1 = false;
    bool hasSim2 = false;
    bool hasESim1 = false;
    bool hasESim2 = false;
    for (const auto &telInfo : telIccAccountInfoList) {
        if (telInfo.isEsim) {
            if (telInfo.simLabelIndex == 1) {
                hasESim1 = true;
            } else if (telInfo.simLabelIndex == SIM_LABLE_INDEX_TOW) {
                hasESim2 = true;
            }
        } else {
            if (telInfo.simLabelIndex == 1) {
                hasSim1 = true;
            } else if (telInfo.simLabelIndex == SIM_LABLE_INDEX_TOW) {
                hasSim2 = true;
            }
        }
    }

    auto scenario = AnalyzeCardScenario(hasSim1, hasSim2, hasESim1, hasESim2);
    RINGTONE_INFO_LOG("Card scenario type: %{public}d", scenario);
 
    if (scenario == SCENARIO_INVALID) {
        RINGTONE_ERR_LOG("Unknown card scenario");
        return E_FAIL;
    }

    for (const auto &config : MIGRATION_CONFIG_TABLE) {
        if (config.scenario == scenario) {
            RINGTONE_INFO_LOG("Applying migration for scenario=%{public}d, "
                "fromCardMask=%{public}d, toCardMask=%{public}d",
                scenario, config.fromCardMask, config.toCardMask);
            ApplyMigration(config.fromCardMask, config.toCardMask);
            break;
        }
    }
#else
    RINGTONE_WARN_LOG("CORE_SERVICE_ENABLE not defined, skip HandleUpgradeWithESim");
#endif
 
    RINGTONE_INFO_LOG("HandleUpgradeWithESim end");
    return E_OK;
}

int32_t RingtoneDualFwkRestoreClone::LoadDualFwkConf(const std::string &backupPath)
{
    DualFwkConfLoader confLoader;
    if (confLoader.Init() != E_OK) {
        RINGTONE_ERR_LOG("Failed to initialize DualFwkConfLoader.");
        return E_FAIL;
    }
    DualFwkConf conf;
    confLoader.Load(conf, RESTORE_SCENE_TYPE_DUAL_CLONE, backupPath);
    confLoader.ShowConf(conf);

    dualFwkSetting_ = std::make_unique<DualFwkSoundSetting>();
    if (dualFwkSetting_ == nullptr) {
        RINGTONE_ERR_LOG("Create DualFwkSoundSetting Failed.");
        return E_FAIL;
    }

    dualFwkSetting_->ProcessConf(conf);
    return E_SUCCESS;
}
} // namespace Media
} // namespace OHOS
