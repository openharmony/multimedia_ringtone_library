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

#define MLOG_TAG "RingtoneDualfwRestore"

#include "ringtone_dualfw_restore.h"

#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "directory_ex.h"
#include "dualfw_conf_parser.h"
#include "dualfw_sound_setting.h"
#include "file_asset.h"
#include "fetch_result.h"
#include "iservice_registry.h"
#include "medialibrary_db_const.h"
#include "result_set_utils.h"
#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_restore_db_utils.h"
#include "ringtone_restore_base.h"
#include "ringtone_restore_type.h"
#include "ringtone_type.h"
#include "userfile_manager_types.h"

namespace OHOS {
namespace Media {
using namespace std;
static const mode_t MODE_RWX_USR_GRP = 02771;
constexpr int STORAGE_MANAGER_MANAGER_ID = 5003;
static const string DUALFW_SOUND_CONF_XML = "setting_system.xml";
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

int32_t RingtoneDualfwRestore::ParseDualfwConf(const string &xml)
{
    auto parser = std::make_unique<DualfwConfParser>(xml);
    if (parser == nullptr) {
        RINGTONE_ERR_LOG("Create DualfwConfParser Failed.");
        return E_FAIL;
    }

    dualfwSetting_ = std::make_unique<DualfwSoundSetting>();
    if (dualfwSetting_ == nullptr) {
        RINGTONE_ERR_LOG("Create DualfwSoundSetting Failed.");
        return E_FAIL;
    }

    if (parser->Parse() != E_SUCCESS) {
        RINGTONE_ERR_LOG("parse dualfw-sound-setting-xml Failed.");
        return E_FAIL;
    }

    parser->ConfTraval([this](std::unique_ptr<DualFwConfRow> &conf) -> void {
        dualfwSetting_->ProcessConfRow(conf);
    });

    return E_SUCCESS;
}

int32_t RingtoneDualfwRestore::Init(const std::string &backupPath)
{
    RINGTONE_INFO_LOG("Init db start");
    if (backupPath.empty()) {
        RINGTONE_ERR_LOG("error: backup path is null");
        return E_INVALID_ARGUMENTS;
    }
    dualfwConf_ = backupPath + "/" + DUALFW_SOUND_CONF_XML;

    if (!RingtoneFileUtils::IsFileExists(dualfwConf_)) {
        RINGTONE_ERR_LOG("dualfw-conf-xml is not exist, path=%{public}s", dualfwConf_.c_str());
        return E_FAIL;
    }

    mediaDataShare_ = CreateMediaDataShare(STORAGE_MANAGER_MANAGER_ID);
    if (mediaDataShare_ == nullptr) {
        RINGTONE_ERR_LOG("mediaDataShareHelper fail");
        return E_FAIL;
    }

    if (ParseDualfwConf(dualfwConf_) != E_SUCCESS) {
        return E_FAIL;
    }

    if (RingtoneRestoreBase::Init(backupPath) != E_OK) {
        return E_FAIL;
    }

    RINGTONE_INFO_LOG("Init db successfully");
    return E_OK;
}

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

static const string KEY_API_VERSION = "API_VERSION";
static unique_ptr<FileAsset> QueryMediaFileAsset(std::shared_ptr<DataShare::DataShareHelper> &mediaDataShare,
    DualfwSettingItem &item)
{
    if (mediaDataShare == nullptr || item.toneFileName.empty() || item.toneFileName == "") {
        RINGTONE_ERR_LOG("argument errr, return nullptr");
        return nullptr;
    }

    vector<string> columns;
    DataShare::DataSharePredicates predicates;
    string prefix = MEDIA_DATA_DB_NAME + " = \"" + item.toneFileName + "\"";
    predicates.SetWhereClause(prefix);
    string queryFileUri = UFM_QUERY_AUDIO;
    MediaUriAppendKeyValue(queryFileUri, KEY_API_VERSION, to_string(MEDIA_API_VERSION_V10));
    shared_ptr<DataShare::DataShareResultSet> resultSet = nullptr;
    Uri uri(queryFileUri);
    resultSet = mediaDataShare->Query(uri, predicates, columns);
    if (resultSet == nullptr) {
        RINGTONE_INFO_LOG("query resultset is null");
        return nullptr;
    }

    unique_ptr<FetchResult<FileAsset>> fetchFileResult = make_unique<FetchResult<FileAsset>>(move(resultSet));
    if (fetchFileResult->GetCount() < 0) {
        return nullptr;
    }
    unique_ptr<FileAsset> fileAsset = fetchFileResult->GetFirstObject();

    return fileAsset;
}

static int32_t BuildFileInfo(DualfwSettingItem &item, unique_ptr<FileAsset> &asset, FileInfo &info)
{
    if (asset == nullptr) {
        return E_ERR;
    }

    switch (item.settingType) {
        case TONE_SETTING_TYPE_ALARM:
            info.toneType = TONE_TYPE_ALARM;
            info.alarmToneType = item.toneType;
            info.alarmToneSourceType = SOURCE_TYPE_CUSTOMISED;
            break;
        case TONE_SETTING_TYPE_RINGTONE:
            info.toneType = TONE_TYPE_RINGTONE;
            info.ringToneType = item.toneType;
            info.ringToneSourceType = SOURCE_TYPE_CUSTOMISED;
            break;
        case TONE_SETTING_TYPE_SHOT:
            info.toneType = TONE_TYPE_NOTIFICATION;
            info.shotToneType = item.toneType;
            info.shotToneSourceType = SOURCE_TYPE_CUSTOMISED;
            break;
        case TONE_SETTING_TYPE_NOTIFICATION:
            info.toneType = TONE_TYPE_NOTIFICATION;
            info.notificationToneType = item.toneType;
            info.notificationToneSourceType = SOURCE_TYPE_CUSTOMISED;
            break;
        default:
            break;
    }

    info.toneId = asset->GetId();
    info.data = asset->GetPath();
    info.size = asset->GetSize();
    info.displayName = asset->GetDisplayName();
    info.title = asset->GetTitle();
    info.mimeType = asset->GetMimeType();
    info.mediaType = RINGTONE_MEDIA_TYPE_AUDIO;
    info.sourceType = SOURCE_TYPE_CUSTOMISED;
    info.dateAdded = asset->GetDateAdded();
    info.dateModified = asset->GetDateModified();
    info.dateTaken = asset->GetDateTaken();
    info.duration = asset->GetDuration();

    return E_SUCCESS;
}

void RingtoneDualfwRestore::StartRestore()
{
    if (dualfwSetting_ == nullptr || mediaDataShare_ == nullptr) {
        RINGTONE_ERR_LOG("dualfw restrore is not initialized successfully");
        return;
    }

    vector<FileInfo> infos = {};
    dualfwSetting_->SettingsTraval([&](DualfwSettingItem &item) -> void {
        auto asset = QueryMediaFileAsset(mediaDataShare_, item);
        FileInfo info = {0};
        auto ret = BuildFileInfo(item, asset, info);
        if (ret == E_SUCCESS) {
            infos.push_back(info);
        }
        RINGTONE_INFO_LOG("***************************************************");
        RINGTONE_INFO_LOG("toneName               = %{public}s", info.data.c_str());
        RINGTONE_INFO_LOG("settingType            = %{public}d", item.settingType);
        RINGTONE_INFO_LOG("toneType               = %{public}d", item.toneType);
        RINGTONE_INFO_LOG("defaultSysSet          = %{public}d", item.defaultSysSet);
        RINGTONE_INFO_LOG("setFlag                = %{public}d", item.setFlag);
    });

    if ((!infos.empty()) && (infos.size() != 0)) {
        InsertTones(infos);
    }
}

int32_t RingtoneDualfwRestore::DupToneFile(FileInfo &info)
{
    string uriStr = MEDIALIBRARY_DATA_URI + "/" + MEDIA_AUDIOOPRN + "/" + to_string(info.toneId);

    Uri openFileUri(uriStr);
    int32_t srcFd = mediaDataShare_->OpenFile(openFileUri, "r");
    if (srcFd < 0) {
        return E_ERR;
    }

    string absFilePath;
    if (!PathToRealPath(info.restorePath, absFilePath)) {
        RINGTONE_ERR_LOG("info.restorePath is not real path, file path: %{private}s", info.restorePath.c_str());
        return E_ERR;
    }
    if (absFilePath.empty()) {
        RINGTONE_ERR_LOG("Failed to obtain the canonical path for source path: %{private}s %{public}d",
            absFilePath.c_str(), errno);
        return E_ERR;
    }

    int32_t dstFd = open(absFilePath.c_str(), O_WRONLY | O_CREAT, MODE_RWX_USR_GRP);
    if (dstFd < 0) {
        RINGTONE_ERR_LOG("Open file failed, errno:%{public}d", errno);
        close(srcFd);
        return E_ERR;
    }
    struct stat fst {};
    if (fstat(srcFd, &fst) == 0) {
        // Copy file content
        if (sendfile(dstFd, srcFd, nullptr, fst.st_size) <= 0) {
            RINGTONE_ERR_LOG("copy file failed errno:%{public}d", errno);
        }
    }
    close(srcFd);
    close(dstFd);

    return E_SUCCESS;
}

void RingtoneDualfwRestore::UpdateRestoreFileInfo(FileInfo &info)
{
    struct stat statInfo;
    if (stat(info.restorePath.c_str(), &statInfo) != 0) {
        RINGTONE_ERR_LOG("stat syscall err %{public}d", errno);
        return;
    }
    info.dateModified = static_cast<int64_t>(RingtoneFileUtils::Timespec2Millisecond(statInfo.st_mtim));
    info.displayName = RingtoneFileUtils::GetFileNameFromPath(info.restorePath);
}

bool RingtoneDualfwRestore::OnPrepare(FileInfo &info, const std::string &dstPath)
{
    if (!RingtoneFileUtils::IsFileExists(dstPath)) {
        RINGTONE_ERR_LOG("dst path is not existing, dst path=%{public}s", dstPath.c_str());
        return false;
    }
    string fileName = RingtoneFileUtils::GetFileNameFromPath(info.data);
    if (fileName.empty() || fileName == "") {
        RINGTONE_ERR_LOG("src file name is null");
        return false;
    }
    string baseName = RingtoneFileUtils::GetBaseNameFromPath(info.data);
    if (baseName.empty() || baseName == "") {
        RINGTONE_ERR_LOG("src file base name is null");
        return false;
    }

    string extensionName = RingtoneFileUtils::GetExtensionFromPath(info.data);
    int32_t repeatCount = 1;
    while (RingtoneFileUtils::IsFileExists(dstPath + "/" + fileName)) {
        struct stat dstStatInfo {};
        string dstName = dstPath + "/" + fileName;
        if (stat(dstName.c_str(), &dstStatInfo) != 0) {
            RINGTONE_ERR_LOG("Failed to get file %{private}s StatInfo, err=%{public}d", dstName.c_str(), errno);
            return false;
        }
        if (info.size == dstStatInfo.st_size) {
            RINGTONE_ERR_LOG("samefile: srcPath=%{public}s, dstPath=%{public}s", info.data.c_str(),
                (dstPath + "/" + fileName).c_str());
            return false;
        }
        fileName = baseName + "(" + to_string(repeatCount++) + ")" + "." + extensionName;
    }
    info.restorePath = dstPath + "/" + fileName;

    if (DupToneFile(info) != E_SUCCESS) {
        return false;
    }

    UpdateRestoreFileInfo(info);

    return true;
}

void RingtoneDualfwRestore::OnFinished(vector<FileInfo> &infos)
{
    RINGTONE_ERR_LOG("ringtone dualfw restore finished");
}
} // namespace Media
} // namespace OHOS
