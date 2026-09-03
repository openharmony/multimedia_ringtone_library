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
#include "ringtone_rdbstore.h"
#include "ringtone_scanner_utils.h"

namespace OHOS {
namespace Media {
using namespace std;
static const int32_t QUERY_COUNT = 500;
static const int32_t INVALID_QUERY_OFFSET = -1;
static const std::string RINGTONE_PREFIX_STR = "/audio/";

int32_t RingtoneRestore::Init(const std::string &backupPath)
{
    RINGTONE_INFO_LOG("Init db start");
    if (backupPath.empty()) {
        RINGTONE_ERR_LOG("error: backup path is null");
        return E_INVALID_ARGUMENTS;
    }
    // 优先使用el1路径下的备份DB，若不存在则回退到非el1路径
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
    // 以只读模式打开备份DB，用于查询源端ToneFiles和SimCardSetting数据
    int32_t err = RingtoneRestoreDbUtils::InitDb(restoreRdb_, RINGTONE_LIBRARY_DB_NAME, dbPath_,
        RINGTONE_BUNDLE_NAME, true);
    if (err != E_OK) {
        RINGTONE_ERR_LOG("ringtone rdb fail, err = %{public}d", err);
        return E_HAS_DB_ERROR;
    }
    // 初始化本地DB（目标端铃音库数据库）和RingtoneSettingManager
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
    RINGTONE_INFO_LOG("QueryFileInfos source records Num: %{public}zu", metaDatas.size());
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
        // 跳过ring_mode为NULL的行，这些行没有有效的设置数据
        bool isNull = false;
        int columnIndex = -1;
        resultSet->GetColumnIndex(SIMCARD_SETTING_COLUMN_RING_MODE, columnIndex);
        resultSet->IsColumnNull(columnIndex, isNull);
        if (isNull) {
            RINGTONE_DEBUG_LOG("skip null column");
            ret = resultSet->GoToNextRow();
            continue;
        }

        auto rdbResult = std::dynamic_pointer_cast<NativeRdb::ResultSet>(resultSet);
        if (rdbResult != nullptr) {
            auto asset = fetchResult.GetObject(rdbResult);
            if (asset != nullptr) {
                // 将备份DB的SimCardSetting行更新到本地DB，forceUpdate默认false
                // 当forceUpdate=false时，只更新vibrate_mode IS NULL的行
                UpdateSettingTable(*asset);
            }
        } else {
            RINGTONE_ERR_LOG("failed to cast resultSet to NativeRdb::ResultSet");
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

void RingtoneRestore::CustomizedRingToneHandle(FileInfo& fileInfo)
{
    if (!RingtoneFileUtils::IsFileExists(fileInfo.data)) {
        RINGTONE_INFO_LOG("source path does not exist, srcPath=%{public}s",
            RingtoneScannerUtils::GetSafePath(fileInfo.data).c_str());
        size_t pos = fileInfo.data.find(RINGTONE_PREFIX_STR);
        if (pos != std::string::npos) {
            // 提取"/audio/"之后的相对路径（如 "alarms/xxx.mp3"）
            string dataPath = fileInfo.data.substr(pos);
            auto rdbStore = RingtoneRdbStore::GetInstance();
            CHECK_AND_RETURN_LOG(rdbStore != nullptr, "rdbstore is nullptr");
            auto rawRdb = rdbStore->GetRaw();
            CHECK_AND_RETURN_LOG(rawRdb != nullptr, "rawRdb is nullptr");
            // 在本地DB中查找source_type=1(preset)且路径后缀匹配的铃声记录
            string sql = "SELECT " + VIBRATE_COLUMN_DATA + " FROM " +
                RINGTONE_TABLE + " WHERE " + VIBRATE_COLUMN_DATA + " LIKE ?" +
                " AND " +  RINGTONE_COLUMN_SOURCE_TYPE + " = 1";
            std::vector<NativeRdb::ValueObject> bindArgs;
            bindArgs.push_back(NativeRdb::ValueObject("%" + dataPath));
            auto resultSet = rawRdb->QuerySql(sql, bindArgs);
            CHECK_AND_RETURN_LOG(resultSet != nullptr, "resultSet is nullptr");
            if (resultSet->GoToFirstRow() != NativeRdb::E_OK) {
                resultSet->Close();
                RINGTONE_INFO_LOG("Query operation failed, no resultSet");
                return;
            }
            // 将fileInfo.data更新为本地DB中preset铃声的实际路径
            string originDataPath = GetStringVal(VIBRATE_COLUMN_DATA, resultSet);
            fileInfo.data = originDataPath;
        }
    }
}

void RingtoneRestore::CheckRestoreFileInfos(vector<FileInfo> &infos)
{
    int32_t videoRingtoneLimit = GetRingtoneLimit(RINGTONE_MEDIA_TYPE_VIDEO);
    RINGTONE_INFO_LOG("%{public}d in %{public}zu available video ringtone to restore",
        videoRingtoneLimit, infos.size());
    int32_t videoRingtoneCnt = 0;
    for (auto it = infos.begin(); it != infos.end();) {
        // 拼接备份文件的完整路径并检查是否存在
        string srcPath = backupPath_ + it->data;
        bool toneExists = RingtoneFileUtils::IsFileExists(srcPath); // 可判断出预置
        bool toneExceedLimit = toneExists && (it->mediaType == RINGTONE_MEDIA_TYPE_VIDEO &&
            ++videoRingtoneCnt > videoRingtoneLimit);
        // 处理自定义铃声的路径映射
        RINGTONE_DEBUG_LOG("CheckRestoreFileInfos: %{public}s, vibrateInfo: soundMode=%{public}d, toneType=%{public}d, "
            "vibrateMode=%{public}d, toneExists=%{public}d, toneExceedLimit=%{public}d, srcPath=%{public}s",
            it->toString().c_str(), it->vibrateInfo.soundMode, it->vibrateInfo.toneType, it->vibrateInfo.vibrateMode,
            toneExists, toneExceedLimit, srcPath.c_str());
        CustomizedRingToneHandle(*it);
        if (!toneExists || toneExceedLimit) {
            if (it->sourceType == SOURCE_TYPE_PRESET) { // TODO 是否过滤 .json
                // preset铃声文件不存在时，不需要恢复文件，但需要保留设置
                // restorePath指向本地preset路径，CheckSetting通过settingMgr_提交设置
                it->restorePath = it->data;
                CheckSetting(*it);
            }
            RINGTONE_DEBUG_LOG("backup file is not exist, path=%{private}s, mediaType=%{public}d",
                srcPath.c_str(), it->mediaType);
            it = infos.erase(it);
        } else {
            it++;
        }
    }
}

int32_t RingtoneRestore::StartRestore()
{
    sceneType_ = RESTORE_SCENE_TYPE_SINGLE_CLONE;
    if (restoreRdb_ == nullptr || backupPath_.empty()) {
        return E_FAIL;
    }
    // 基类StartRestore：确保本地扫描器完成，初始化MimeType映射
    auto ret = RingtoneRestoreBase::StartRestore();
    if (ret != E_OK) {
        return ret;
    }
    // 检查备份DB中哪些铃声类型处于"无铃声"状态，同步到本地D
    CheckNotRingtoneRestore();
    // 从备份DB读取SimCardSetting表，更新到本地DB的SimCardSetting表
    UpdateSettingInfos();
    // 从备份DB读取ToneFiles表所有记录
    auto infos = QueryFileInfos(INVALID_QUERY_OFFSET);
    if ((!infos.empty()) && (infos.size() != 0)) {
        // 过滤无法恢复的记录，保留preset铃声的设置信息
        CheckRestoreFileInfos(infos);
        // 将有效记录插入本地DB，并在插入过程中通过CheckSetting提交设置
        ret = InsertTones(infos);
    }
    // 刷新设置管理器，将CommitSetting中缓存的设置操作持久化到DB
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
    // 处理目标路径同名文件冲突
    while (RingtoneFileUtils::IsFileExists(info.restorePath)) {
        if (RingtoneFileUtils::IsSameFile(srcPath, info.restorePath)) {
            // 目标路径已存在相同文件，无需移动，但需保留设置信息
            CheckSetting(info);
            RINGTONE_ERR_LOG("samefile: srcPath=%{private}s, dstPath=%{private}s", srcPath.c_str(),
                info.restorePath.c_str());
            return false;
        }
        // 同名但不同文件，自动重命名（如 "Alarm(1).mp3"）
        info.restorePath = destPath + "/" + baseName + "(" + to_string(repeatCount++) + ")" + "." + extensionName;
    }

    // 将备份文件移动到目标路径（先尝试rename，失败则copy+delete）
    if (!RingtoneRestoreBase::MoveFile(srcPath, info.restorePath)) {
        return false;
    }

    // 更新文件属性以反映目标路径下的实际状态
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
    // SIM卡1来电铃声：检查备份DB中SIM卡1来电是否无铃声，同步到本地DB
    if (RingtoneRestoreBase::IsDetermineNoRingtone(RINGTONE_COLUMN_RING_TONE_TYPE,
        RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, RING_TONE_TYPE_SIM_CARD_1, restoreRdb_) &&
        RingtoneRestoreBase::NeedCommitSetting(RINGTONE_COLUMN_RING_TONE_TYPE,
        RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, RING_TONE_TYPE_SIM_CARD_1)) {
        RINGTONE_INFO_LOG("no ringtone sound for ringtone sim card 1");
        RingtoneRestoreBase::SetNotRingtoneForRingtone(RING_TONE_TYPE_SIM_CARD_1);
    }
    // SIM卡2来电铃声：检查备份DB中SIM卡2来电是否无铃声，同步到本地DB
    if (RingtoneRestoreBase::IsDetermineNoRingtone(RINGTONE_COLUMN_RING_TONE_TYPE,
        RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, RING_TONE_TYPE_SIM_CARD_2, restoreRdb_) &&
        RingtoneRestoreBase::NeedCommitSetting(RINGTONE_COLUMN_RING_TONE_TYPE,
            RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, RING_TONE_TYPE_SIM_CARD_2)) {
        RINGTONE_INFO_LOG("no ringtone sound for ringtone sim card 2");
        RingtoneRestoreBase::SetNotRingtoneForRingtone(RING_TONE_TYPE_SIM_CARD_2);
    }
    // SIM卡1短信铃声：检查备份DB中SIM卡1短信是否无铃声，同步到本地DB
    if (RingtoneRestoreBase::IsDetermineNoRingtone(RINGTONE_COLUMN_SHOT_TONE_TYPE,
        RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SHOT_TONE_TYPE_SIM_CARD_1, restoreRdb_) &&
        RingtoneRestoreBase::NeedCommitSetting(RINGTONE_COLUMN_SHOT_TONE_TYPE,
            RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SHOT_TONE_TYPE_SIM_CARD_1)) {
        RINGTONE_INFO_LOG("no shot sound for shot sim card 1");
        RingtoneRestoreBase::SetNotRingtoneForShot(SHOT_TONE_TYPE_SIM_CARD_1);
    }
    // SIM卡2短信铃声：检查备份DB中SIM卡2短信是否无铃声，同步到本地DB
    if (RingtoneRestoreBase::IsDetermineNoRingtone(RINGTONE_COLUMN_SHOT_TONE_TYPE,
        RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SHOT_TONE_TYPE_SIM_CARD_2, restoreRdb_) &&
        RingtoneRestoreBase::NeedCommitSetting(RINGTONE_COLUMN_SHOT_TONE_TYPE,
            RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SHOT_TONE_TYPE_SIM_CARD_2)) {
        RINGTONE_INFO_LOG("no shot sound for shot sim card 2");
        RingtoneRestoreBase::SetNotRingtoneForShot(SHOT_TONE_TYPE_SIM_CARD_2);
    }
    // 通知铃声：检查备份DB中通知铃声是否无铃声，同步到本地DB
    // 通知铃声无卡区分，type和allSetType都使用NOTIFICATION_TONE_TYPE
    if (RingtoneRestoreBase::IsDetermineNoRingtoneForNotification(restoreRdb_) &&
        RingtoneRestoreBase::NeedCommitSettingForNotification()) {
        RINGTONE_INFO_LOG("no notification sound for notification");
        RingtoneRestoreBase::SetNotRingtoneForNotification();
    }
}
} // namespace Media
} // namespace OHOS
