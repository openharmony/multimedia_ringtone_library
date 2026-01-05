/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#define MLOG_TAG "CustomisedToneProcessor"

#include "customised_tone_processor.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>

#include "ringtone_type.h"
#include "directory_ex.h"
#include "avmetadatahelper.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "dualfwk_conf_loader.h"
#include "ringtone_file_utils.h"
#include "ringtone_db_const.h"
#include "ringtone_scanner_utils.h"
#include "ringtone_mimetype_utils.h"
#include "ringtone_tracer.h"
#include "ringtone_utils.h"
#ifdef USE_MEDIA_LIBRARY
#include "userfilemgr_uri.h"
#include "medialibrary_db_const.h"
#include "fetch_result.h"
#include "base_data_uri.h"
#endif
#include "ringtone_restore_base.h"
#include "iservice_registry.h"
#include "ringtone_restore_db_utils.h"

namespace OHOS {
namespace Media {
using namespace std;

CustomisedToneProcessor::CustomisedToneProcessor()
{
#ifdef USE_MEDIA_LIBRARY
    manager_ = Media::MediaLibraryManager::GetMediaLibraryManager();
    CHECK_AND_RETURN_LOG(manager_ != nullptr, "get media library manager failed");
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG(sam != nullptr, "get samgr failed");
    auto remoteObj = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    CHECK_AND_RETURN_LOG(remoteObj != nullptr, "get system ability failed");
    manager_->InitMediaLibraryManager(remoteObj);
#endif
}

static int32_t GetFileFd(const std::string &path, int &fd, struct stat64 &st)
{
    std::error_code ec;
    char realPath[PATH_MAX] = "";

    if (realpath(path.c_str(), realPath) == nullptr) {
        RINGTONE_ERR_LOG("Realpath input file failed");
        return E_FAIL;
    }

    string filePath(realPath);
    std::filesystem::exists(filePath, ec);
    if (ec.value() != E_SUCCESS) {
        RINGTONE_ERR_LOG("GetFileTitle normalized fd failed");
        return E_FAIL;
    }

    int mode = O_RDONLY;
    fd = open(realPath, mode);
    CHECK_AND_RETURN_RET_LOG(fd > 0, E_FAIL, "open fail path: %{private}s", realPath);

    int32_t ret = fstat64(fd, &st);
    if (ret != 0) {
        RINGTONE_ERR_LOG("file stat is err, %{public}d, fd: %{public}d", ret, fd);
        close(fd);
        return E_FAIL;
    }

    return E_OK;
}

static int32_t GetFileTitleAndDuration(const std::string &path, FileInfo &fileInfo)
{
    RingtoneTracer tracer;
    tracer.Start("CustomisedToneProcessor::GetFileTitleAndDuration");

    int fd;
    struct stat64 st;
    if (GetFileFd(path, fd, st) != E_OK) {
        return E_FAIL;
    }

    shared_ptr<AVMetadataHelper> avMetadataHelper = AVMetadataHelperFactory::CreateAVMetadataHelper();
    if (avMetadataHelper == nullptr) {
        close(fd);
        return 0;
    }
    int64_t fileLength = static_cast<int64_t>(st.st_size);
    int32_t ret = avMetadataHelper->SetSource(fd, 0, fileLength, AV_META_USAGE_META_ONLY);
    close(fd);
    if (ret != 0) {
        RINGTONE_ERR_LOG("av set source is err");
        return E_FAIL;
    }

    std::shared_ptr<Meta> avMeta = avMetadataHelper->GetAVMetadata();
    CHECK_AND_RETURN_RET_LOG(avMeta != nullptr, 0, "avMeta is nullptr");
    if (!avMeta->GetData(Tag::MEDIA_TITLE, fileInfo.title) || fileInfo.title.empty()) {
        RINGTONE_ERR_LOG("get file title fail");
        // If the title can not be obtained, use the prefix of displayname
        std::string::size_type pos = fileInfo.displayName.find_last_of('.');
        fileInfo.title = (pos == std::string::npos) ? fileInfo.displayName : fileInfo.displayName.substr(0, pos);
    }

    std::string duration;
    if (!avMeta->GetData(Tag::MEDIA_DURATION, duration) || !RingtoneUtils::IsNumber(duration)) {
        RINGTONE_ERR_LOG("get file duration fail");
        return E_FAIL;
    }

    fileInfo.duration = atoi(duration.c_str());
    return E_SUCCESS;
}

int32_t CustomisedToneProcessor::GetCustomisedAudioPath(DualFwkConf &dualFwkConf)
{
    DualFwkConfLoader confLoader;
    if (confLoader.Init() != E_OK) {
        RINGTONE_ERR_LOG("Failed to initialize dualFwkConfLoader");
        return E_FAIL;
    }

    dualFwkConf.ringtonePath = confLoader.GetConf("ringtone_path");
    dualFwkConf.ringtone2Path = confLoader.GetConf("ringtone2_path");
    dualFwkConf.messagePath = confLoader.GetConf("message_path");
    dualFwkConf.messageSub1 = confLoader.GetConfPath("messageSub1");
    dualFwkConf.notificationSoundPath = confLoader.GetConf("notification_sound_path");
    dualFwkConf.alarmAlertPath = confLoader.GetConf("alarm_alert_path");

    return E_OK;
}

std::string CustomisedToneProcessor::ConvertCustomisedAudioPath(const std::string &ringtonePath)
{
    CHECK_AND_RETURN_RET_LOG(!ringtonePath.empty(), "", "customised audio path is empty");
    auto pos = ringtonePath.find(RINGTONE_EXTERNAL_BASE_PATH);
    CHECK_AND_RETURN_RET_LOG(pos != std::string::npos, "",
        "customised audio file not in the directory: %{private}s", RINGTONE_EXTERNAL_BASE_PATH.c_str());

    std::filesystem::path userSubPath = ringtonePath.substr(RINGTONE_EXTERNAL_BASE_PATH.length());
    userSubPath = userSubPath.relative_path();
    auto iter = userSubPath.begin();
    CHECK_AND_RETURN_RET_LOG(iter != userSubPath.end(), "",
        "invalid subpath: %{private}s", userSubPath.c_str());

    std::filesystem::path subPath = RingtoneUtils::IsNumber(*iter) ?
        std::filesystem::relative(userSubPath, *iter) : userSubPath;
    bool isMusicPath = subPath.string().find(RINGTONE_EXTERNAL_SUB_DIR_MUSIC) != std::string::npos;
    std::filesystem::path fileMgrBasePath = FILE_MANAGER_BASE_PATH;
    std::filesystem::path searchPath1 = fileMgrBasePath / FILE_MANAGER_SUB_DIR_DOCS;
    std::filesystem::path searchPath2 = fileMgrBasePath / FILE_MANAGER_SUB_DIR_UPDATEBACKUP;
    std::string strSearchPath1 = (searchPath1 / subPath).lexically_normal().string();
    std::string strSearchPath2 =  (searchPath2 / subPath).lexically_normal().string();
    RINGTONE_INFO_LOG("searchPath1:%{private}s, searchPath2:%{private}s",
        strSearchPath1.c_str(), strSearchPath2.c_str());

    std::string tmpPath;
    CHECK_AND_RETURN_RET(!(isMusicPath && PathToRealPath(strSearchPath2, tmpPath)), strSearchPath2);
    CHECK_AND_RETURN_RET(!(!isMusicPath && PathToRealPath(strSearchPath1, tmpPath)), strSearchPath1);
    CHECK_AND_RETURN_RET(!(!isMusicPath && PathToRealPath(strSearchPath2, tmpPath)), strSearchPath2);
    CHECK_AND_RETURN_RET(!(isMusicPath && PathToRealPath(strSearchPath1, tmpPath)), strSearchPath1);
    RINGTONE_INFO_LOG("No rule can convert ringtonePath: %{private}s", ringtonePath.c_str());
    return "";
}

int32_t CustomisedToneProcessor::BuildFileInfo(const std::string &dualFilePath, int32_t toneType, int32_t ringtoneType,
    int32_t shotToneType, std::vector<FileInfo> &fileInfos)
{
    CHECK_AND_RETURN_RET_LOG(!dualFilePath.empty(), E_FAIL, "empty input path");
    const std::string oldUri{dualFilePath};
    CHECK_AND_RETURN_RET_LOG(CanBuildFileInfo(oldUri), E_FAIL,
        "can not add file info of tone:%{public}d", toneType);

    std::string customisedPath = ConvertCustomisedAudioPath(oldUri);
    std::string newUri = customisedPath.empty() ? GetNewUri(toneType, oldUri) : customisedPath;
    std::string filePath = newUri != oldUri ? newUri : customisedPath;
    CHECK_AND_RETURN_RET_LOG(!filePath.empty(), E_FAIL, "invalid customized path");

    FileInfo fileInfo;
    fileInfo.toneType = toneType;
    fileInfo.data = filePath;
    fileInfo.displayName = RingtoneFileUtils::GetFileNameFromPath(fileInfo.data);
    fileInfo.ringToneType = ringtoneType;
    fileInfo.shotToneType = shotToneType;
    fileInfo.sourceType = SOURCE_TYPE_CUSTOMISED;

    const std::string ext = RingtoneFileUtils::GetExtensionFromPath(filePath);
    std::string mimeType = RingtoneMimeTypeUtils::GetMimeTypeFromExtension(ext);
    RingtoneMediaType mediaType = RingtoneMimeTypeUtils::GetMediaTypeFromMimeType(mimeType);
    fileInfo.mimeType = mimeType;
    fileInfo.mediaType = mediaType;

    fileInfo.dateAdded = RingtoneFileUtils::UTCTimeMilliSeconds();
    GetFileTitleAndDuration(filePath, fileInfo);

    struct stat statInfo {};
    if (stat(filePath.c_str(), &statInfo) != 0) {
        RINGTONE_ERR_LOG("fail to get file %{private}s statInfo, err: %{public}d", filePath.c_str(), errno);
        return E_FAIL;
    }

    fileInfo.size = statInfo.st_size;
    fileInfos.emplace_back(fileInfo);
    return E_OK;
}

void CustomisedToneProcessor::SetVideoToneLimit(int32_t limit)
{
    RINGTONE_INFO_LOG("limit set to %{public}d", limit);
    videoToneLimit_ = limit;
}

bool CustomisedToneProcessor::CanBuildFileInfo(const std::string &dualFilePath)
{
    bool ret = true;
    const std::string ext = RingtoneFileUtils::GetExtensionFromPath(dualFilePath);
    std::string mimeType = RingtoneMimeTypeUtils::GetMimeTypeFromExtension(ext);
    RingtoneMediaType mediaType = RingtoneMimeTypeUtils::GetMediaTypeFromMimeType(mimeType);
    if (videoToneLimit_.load() >= 0 && mediaType == RINGTONE_MEDIA_TYPE_VIDEO) {
        if (videoToneLimit_.load() == 0) {
            RINGTONE_ERR_LOG("video file info can not build");
            return false;
        } else {
            videoToneLimit_--;
        }
    }

    return ret;
}

std::string CustomisedToneProcessor::GetNewUri(int32_t toneType, const std::string &oldUri)
{
    std::string newUri{oldUri};
#ifdef USE_MEDIA_LIBRARY
    const std::string ext = RingtoneFileUtils::GetExtensionFromPath(oldUri);
    std::string mimeType = RingtoneMimeTypeUtils::GetMimeTypeFromExtension(ext);
    RingtoneMediaType mediaType = RingtoneMimeTypeUtils::GetMediaTypeFromMimeType(mimeType);
    CHECK_AND_RETURN_RET_LOG(manager_ != nullptr, newUri, "get media library manager failed");
    std::string fileUri;
    std::string filePath{oldUri};
    if (mediaType == RINGTONE_MEDIA_TYPE_VIDEO && ext == RINGTONE_CONTAINER_TYPE_VIDEO_MP4) {
        auto newUris = manager_->GetUrisByOldUris({oldUri});
        fileUri = newUris.count(oldUri) == 0 ? oldUri : newUris[oldUri];
        if (fileUri != oldUri) {
            filePath = GetFileAssetPathByUri(fileUri);
        }
    }
    RINGTONE_INFO_LOG("fileUri:%{public}s, old path:%{public}s, new path:%{public}s",
        fileUri.c_str(), oldUri.c_str(), filePath.c_str());
    if (!filePath.empty() && filePath != oldUri) {
        int32_t fd = manager_->OpenAsset(fileUri, RINGTONE_FILEMODE_READONLY);
        CHECK_AND_RETURN_RET_LOG(fd > 0, newUri, "get file fd failed");
        if (!RingtoneFileUtils::CheckFileSize(fd, MAX_TONE_FILE_SIZE)) {
            RINGTONE_ERR_LOG("file size exceed limit");
            manager_->CloseAsset(fileUri, fd);
            return newUri;
        }
        std::string target = RingtoneRestoreBase::GetRestoreDir(toneType) +
            "/" + RingtoneFileUtils::GetFileNameFromPath(oldUri);
        if (!RingtoneFileUtils::IsFileExists(target)) {
            int32_t ret = RingtoneFileUtils::CopyFileFromFd(fd, target);
            manager_->CloseAsset(fileUri, fd);
            CHECK_AND_RETURN_RET_LOG(ret == E_OK, newUri, "copy file failed");
        } else {
            manager_->CloseAsset(fileUri, fd);
        }
        newUri = target;
        RINGTONE_INFO_LOG("new Uri:%{public}s", newUri.c_str());
    }
#endif
    return newUri;
}

#ifdef USE_MEDIA_LIBRARY
std::string CustomisedToneProcessor::GetFileAssetPathByUri(const std::string &uri)
{
    std::string ret;
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(sam != nullptr, ret, "get samgr failed");
    auto remoteObj = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    CHECK_AND_RETURN_RET_LOG(remoteObj != nullptr, ret, "get system ability failed");
    auto dataShareHelper = DataShare::DataShareHelper::Creator(remoteObj, MEDIALIBRARY_DATA_URI);
    CHECK_AND_RETURN_RET_LOG(dataShareHelper != nullptr, ret, "create media datashare failed");

    std::string fileName = RingtoneFileUtils::GetFileNameFromPath(uri);
    CHECK_AND_RETURN_RET_LOG(!fileName.empty(), ret, "get name from uri failed");

    std::vector<std::string> columns{};
    Uri queryUri(PAH_QUERY_PHOTO);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(MEDIA_DATA_DB_NAME, fileName);
    auto queryResultSet = dataShareHelper->Query(queryUri, predicates, columns);
    auto fetchResult = std::make_shared<FetchResult<FileAsset>>(queryResultSet);
    if (fetchResult == nullptr) {
        dataShareHelper->Release();
        return ret;
    }

    auto asset = fetchResult->GetFirstObject();
    if (asset != nullptr) {
        ret = asset->GetPath();
    }
    dataShareHelper->Release();
    return ret;
}
#endif

std::vector<FileInfo> CustomisedToneProcessor::BuildFileInfos()
{
    std::vector<FileInfo> result;
    DualFwkConf dualFwkConf;
    GetCustomisedAudioPath(dualFwkConf);
    BuildFileInfo(dualFwkConf.ringtonePath, TONE_TYPE_RINGTONE, RING_TONE_TYPE_SIM_CARD_1, SHOT_TONE_TYPE_NOT, result);
    BuildFileInfo(dualFwkConf.ringtone2Path, TONE_TYPE_RINGTONE, RING_TONE_TYPE_SIM_CARD_2, SHOT_TONE_TYPE_NOT, result);
    BuildFileInfo(dualFwkConf.notificationSoundPath, TONE_TYPE_NOTIFICATION, RING_TONE_TYPE_NOT, SHOT_TONE_TYPE_NOT,
        result);
    BuildFileInfo(dualFwkConf.alarmAlertPath, TONE_TYPE_ALARM, RING_TONE_TYPE_NOT, SHOT_TONE_TYPE_NOT, result);
    BuildFileInfo(dualFwkConf.messagePath, TONE_TYPE_NOTIFICATION, SHOT_TONE_TYPE_NOT, RING_TONE_TYPE_SIM_CARD_1,
        result);
    BuildFileInfo(dualFwkConf.messageSub1, TONE_TYPE_NOTIFICATION, SHOT_TONE_TYPE_NOT, RING_TONE_TYPE_SIM_CARD_2,
        result);
    return result;
}

int32_t CustomisedToneProcessor::QueryFileMgrForFileInfo(std::map<std::string, std::shared_ptr<FileInfo>> &infoMap)
{
    std::vector<FileInfo> fileInfos = BuildFileInfos();
    for (auto fileInfo : fileInfos) {
        std::shared_ptr<FileInfo> infoPtr = std::make_shared<FileInfo>(fileInfo);
        infoMap[fileInfo.displayName] = infoPtr;
        RINGTONE_INFO_LOG("new info found in file_manager: %{public}s", infoPtr->toString().c_str());
    }
    return E_SUCCESS;
}
} // namespace Media
} // namespace OHOS