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
#ifdef USE_MEDIA_LIBRARY
#include "fetch_result.h"
#include "file_asset.h"
#include "userfile_manager_types.h"
#endif

namespace OHOS {
namespace Media {
using namespace std;

constexpr int STORAGE_MANAGER_MANAGER_ID = 5003;
static const string DUALFWK_SOUND_CONF_XML = "backup";

static std::shared_ptr<DataShare::DataShareHelper> CreateMediaDataShare(int32_t systemAbilityId)
{
    RINGTONE_INFO_LOG("CreateDataShareHelper::CreateFileExtHelper ");
    auto saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        RINGTONE_ERR_LOG("CreateFileExtHelper Get system ability mgr failed.");
        return nullptr;
    }
    auto remoteObj = saManager->GetSystemAbility(systemAbilityId);
    if (remoteObj == nullptr) {
        RINGTONE_ERR_LOG("CreateDataShareHelper GetSystemAbility Service Failed.");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObj, MEDIALIBRARY_DATA_URI);
}

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

    mediaDataShare_ = CreateMediaDataShare(STORAGE_MANAGER_MANAGER_ID);
    if (mediaDataShare_ == nullptr) {
        RINGTONE_ERR_LOG("mediaDataShareHelper fail");
        return E_FAIL;
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

#ifdef USE_MEDIA_LIBRARY
static void MediaUriAppendKeyValue(string &uri, const string &key, const string &value)
{
    string uriKey = key + '=';
    if (uri.find(uriKey) != string::npos) {
        return;
    }

    char queryMark = (uri.find('?') == string::npos) ? '?' : '&';
    string append = queryMark + key + '=' + value;

    size_t posJ = uri.find('#');
    if (posJ == string::npos) {
        uri += append;
    } else {
        uri.insert(posJ, append);
    }
}
#endif

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

#ifdef USE_MEDIA_LIBRARY
static void AssetToFileInfo(std::shared_ptr<FileInfo> infoPtr, const std::unique_ptr<FileAsset> &asset)
{
    infoPtr->toneId = asset->GetId();
    infoPtr->data = asset->GetPath();
    infoPtr->size = asset->GetSize();
    infoPtr->displayName = asset->GetDisplayName();
    infoPtr->title = asset->GetTitle();
    infoPtr->mimeType = asset->GetMimeType();
    infoPtr->mediaType = RINGTONE_MEDIA_TYPE_AUDIO;
    infoPtr->sourceType = SOURCE_TYPE_CUSTOMISED;
    infoPtr->dateAdded = asset->GetDateAdded();
    infoPtr->dateModified = asset->GetDateModified();
    infoPtr->dateTaken = asset->GetDateTaken();
    infoPtr->duration = asset->GetDuration();
}

int32_t RingtoneDualFwkRestore::QueryMediaLibForFileInfo(const std::vector<std::string> &names,
    std::map<std::string, std::shared_ptr<FileInfo>> &infoMap,
    const std::string &queryFileUriBase, const std::string &predicateColumn)
{
    if (mediaDataShare_ == nullptr || names.empty()) {
        RINGTONE_ERR_LOG("argument errr, return nullptr");
        return E_ERR;
    }
    vector<string> columns;
    DataShare::DataSharePredicates predicates;
    string whereClause = MakeBatchQueryWhereClause(names, predicateColumn) + " AND " +
        MediaColumn::MEDIA_TYPE + "=" + std::to_string(MEDIA_TYPE_AUDIO);
    RINGTONE_INFO_LOG("Querying media_library where %{public}s", whereClause.c_str());
    predicates.SetWhereClause(whereClause);

    std::string queryFileUri = queryFileUriBase;
    MediaUriAppendKeyValue(queryFileUri, KEY_API_VERSION, to_string(MEDIA_API_VERSION_V10));
    shared_ptr<DataShare::DataShareResultSet> resultSet = nullptr;
    Uri uri(queryFileUri);

    resultSet = mediaDataShare_->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        RINGTONE_WARN_LOG("resultset for %{public}s is null", whereClause.c_str());
        return E_FAIL;
    }
    int count = 0;
    resultSet->GetRowCount(count);
    RINGTONE_INFO_LOG("Querying %{public}s where %{public}s, got %{public}d records",
        queryFileUri.c_str(), whereClause.c_str(), count);

    if (count <= 0) {
        RINGTONE_WARN_LOG("resultset for %{public}s is empty", whereClause.c_str());
        return E_SUCCESS;
    }

    std::unique_ptr<FetchResult<FileAsset>> fetchFileResult = make_unique<FetchResult<FileAsset>>(
        move(resultSet));

    for (int i = 0; i < count; i++) {
        std::unique_ptr<FileAsset> asset = fetchFileResult->GetNextObject();
        auto infoPtr = std::make_shared<FileInfo>();
        AssetToFileInfo(infoPtr, asset);
        if (predicateColumn == MediaColumn::MEDIA_TITLE) {
            infoMap[asset->GetTitle()] = infoPtr;
        } else {
            infoMap[asset->GetDisplayName()] = infoPtr;
        }
        RINGTONE_INFO_LOG("new info found in media_lib: %{public}s", infoPtr->toString().c_str()); // debug
    }
    return E_SUCCESS;
}
#endif

int32_t RingtoneDualFwkRestore::QueryRingToneDbForFileInfo(std::shared_ptr<NativeRdb::RdbStore> rdbStore,
    const std::vector<std::string> &names, std::map<std::string, std::shared_ptr<FileInfo>> &infoMap,
    const std::string &predicateColumn)
{
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("rdb_ is nullptr, Maybe init failed.");
        return E_FAIL;
    }
    string whereClause = MakeBatchQueryWhereClause(names, predicateColumn);
    std::string queryCountSql = "SELECT * FROM " + RINGTONE_TABLE +
        " WHERE " + whereClause +  " AND " +
        RINGTONE_COLUMN_MEDIA_TYPE + "=" + std::to_string(RINGTONE_MEDIA_TYPE_AUDIO) + ";";
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
            infoMap[infoPtr->title] = infoPtr;
        } else {
            infoMap[infoPtr->displayName] = infoPtr;
        }
        
        RINGTONE_INFO_LOG("new info found in ringtone_lib: %{public}s", infoPtr->toString().c_str()); // debug
    }

    return E_SUCCESS;
}

static void AddSettingsToFileInfo(const DualFwkSettingItem &setting, FileInfo &info)
{
    switch (setting.settingType) {
        case TONE_SETTING_TYPE_ALARM:
            info.toneType = TONE_TYPE_ALARM;
            info.alarmToneType = setting.toneType;
            info.alarmToneSourceType = SOURCE_TYPE_CUSTOMISED;
            break;
        case TONE_SETTING_TYPE_RINGTONE:
            info.toneType = TONE_TYPE_RINGTONE;
            info.ringToneType = setting.toneType;
            info.ringToneSourceType = SOURCE_TYPE_CUSTOMISED;
            break;
        case TONE_SETTING_TYPE_SHOT:
            info.toneType = TONE_TYPE_NOTIFICATION;
            info.shotToneType = setting.toneType;
            info.shotToneSourceType = SOURCE_TYPE_CUSTOMISED;
            break;
        case TONE_SETTING_TYPE_NOTIFICATION:
            info.toneType = TONE_TYPE_NOTIFICATION;
            info.notificationToneType = setting.toneType;
            info.notificationToneSourceType = SOURCE_TYPE_CUSTOMISED;
            break;
        default:
            break;
    }
}

static std::shared_ptr<FileInfo> MergeQueries(const DualFwkSettingItem &setting,
    std::map<std::string, std::shared_ptr<FileInfo>> resultFromMediaByDisplayName,
    std::map<std::string, std::shared_ptr<FileInfo>> resultFromMediaByTitle,
    std::map<std::string, std::shared_ptr<FileInfo>> resultFromRingtoneByDisplayName,
    bool &doInsert)
{
    std::shared_ptr<FileInfo> infoPtr;
    doInsert = true;
    auto keyName = setting.toneFileName;
    auto KeyToneType = setting.toneType;
    if (resultFromMediaByDisplayName.find(keyName) != resultFromMediaByDisplayName.end()) {
        infoPtr = resultFromMediaByDisplayName[keyName];
        RINGTONE_INFO_LOG("found %{public}s in media_lib", keyName.c_str());
    } else if (resultFromMediaByTitle.find(keyName) != resultFromMediaByTitle.end()) {
        infoPtr = resultFromMediaByTitle[keyName];
        RINGTONE_INFO_LOG("found %{public}s in media_lib", keyName.c_str());
    } else if (resultFromRingtoneByDisplayName.find(keyName) != resultFromRingtoneByDisplayName.end()) {
        infoPtr = resultFromRingtoneByDisplayName.at(keyName);
        if (infoPtr == nullptr) {
            RINGTONE_ERR_LOG("infoPtr from ringtone by display name is nullptr");
            return nullptr;
        }
        if ((infoPtr->sourceType == SourceType::SOURCE_TYPE_PRESET) && ((infoPtr->toneType ==
            ToneType::TONE_TYPE_RINGTONE) || (infoPtr->toneType == ToneType::TONE_TYPE_NOTIFICATION))) {
            if (KeyToneType == infoPtr->toneType) {
                infoPtr = resultFromRingtoneByDisplayName[keyName];
                RINGTONE_INFO_LOG("found %{public}s in ringtone db", keyName.c_str());
                doInsert = false;
            } else {
                RINGTONE_INFO_LOG("%{public}s is invalid", keyName.c_str());
                return nullptr;
            }
        } else {
            infoPtr = resultFromRingtoneByDisplayName[keyName];
            RINGTONE_INFO_LOG("found %{public}s in ringtone db", keyName.c_str());
            doInsert = false;
        }
    } else if (setting.isTitle) {
        keyName = RingtoneUtils::ReplaceAll(keyName + ".ogg", " ", "_");
        if (resultFromRingtoneByDisplayName.find(keyName) != resultFromRingtoneByDisplayName.end()) {
            infoPtr = resultFromRingtoneByDisplayName[keyName];
            RINGTONE_INFO_LOG("found %{public}s in ringtone db", keyName.c_str());
            doInsert = false;
        }
    } else {
        RINGTONE_INFO_LOG("failed to find %{public}s", keyName.c_str());
    }
    return infoPtr;
}

std::vector<FileInfo> RingtoneDualFwkRestore::BuildFileInfo()
{
    std::vector<FileInfo> result;
    std::vector<std::string> fileNames = dualFwkSetting_->GetFileNames();
    std::map<std::string, std::shared_ptr<FileInfo>> resultFromMediaByDisplayName;
    std::map<std::string, std::shared_ptr<FileInfo>> resultFromMediaByTitle;

    std::vector<std::string> displayNames = dualFwkSetting_->GetDisplayNames();
    std::map<std::string, std::shared_ptr<FileInfo>> resultFromRingtoneByDisplayName;

#ifdef USE_MEDIA_LIBRARY
    QueryMediaLibForFileInfo(fileNames, resultFromMediaByDisplayName, UFM_QUERY_AUDIO, "display_name");
    QueryMediaLibForFileInfo(fileNames, resultFromMediaByTitle, UFM_QUERY_AUDIO, "title");
#endif
    QueryRingToneDbForFileInfo(GetBaseDb(), displayNames, resultFromRingtoneByDisplayName, "display_name");

    for (const auto& setting : dualFwkSetting_->GetSettings()) {
        bool doInsert = true;
        auto infoPtr = MergeQueries(setting, resultFromMediaByDisplayName, resultFromMediaByTitle,
            resultFromRingtoneByDisplayName, doInsert);
        if (infoPtr == nullptr) {
            continue;
        }
        FileInfo info = *infoPtr;
        info.doInsert = doInsert;
        AddSettingsToFileInfo(setting, info);
        result.push_back(info);

        RINGTONE_INFO_LOG("push back into results -----> %{private}s", info.toString().c_str());
    }
    return result;
}

int32_t RingtoneDualFwkRestore::StartRestore()
{
    if (dualFwkSetting_ == nullptr || mediaDataShare_ == nullptr) {
        RINGTONE_ERR_LOG("dualfwk restrore is not initialized successfully");
        return E_ERR;
    }
    auto ret = RingtoneRestoreBase::StartRestore();
    if (ret != E_OK) {
        return ret;
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
    RINGTONE_INFO_LOG("DupToneFile from %{private}s to %{private}s", info.data.c_str(), info.restorePath.c_str());
    std::string absDstPath = info.restorePath;
    RINGTONE_INFO_LOG("converted dst path from %{private}s to realpath %{private}s", info.restorePath.c_str(),
        absDstPath.c_str());

    std::string absSrcPath = info.data;
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
            CheckSetting(info);
            RINGTONE_ERR_LOG("samefile: srcPath=%{private}s, dstPath=%{private}s", info.data.c_str(),
                info.restorePath.c_str());
            return false;
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
