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

namespace OHOS {
namespace Media {
using namespace std;

CustomisedToneProcessor::CustomisedToneProcessor() {}

static int32_t GetFileTitleAndDuration(const std::string &path, FileInfo &fileInfo)
{
    RingtoneTracer tracer;
    tracer.Start("CustomisedToneProcessor::GetFileTitleAndDuration");

    std::error_code ec;
    std::string realPath = std::filesystem::weakly_canonical(realPath, ec);
    if (ec.value() != E_SUCCESS) {
        RINGTONE_ERR_LOG("GetFileTitle normalized realPath failed");
        return E_FAIL;
    }

    std::filesystem::exists(realPath, ec);
    if (ec.value() != E_SUCCESS) {
        RINGTONE_ERR_LOG("GetFileTitle normalized fd failed");
        return E_FAIL;
    }

    int mode = O_RDONLY;
    int fd = open(realPath.c_str(), mode);
    if (fd < 0) {
        RINGTONE_ERR_LOG("open fail path: %{private}s", realPath.c_str());
        return E_FAIL;
    }

    struct stat64 st;
    int32_t ret = fstat64(fd, &st);
    if (ret != 0) {
        RINGTONE_ERR_LOG("file stat is err, %{public}d, fd: %{public}d", ret, fd);
        close(fd);
        return E_FAIL;
    }

    shared_ptr<AVMetadataHelper> avMetadataHelper = AVMetadataHelperFactory::CreateAVMetadataHelper();
    CHECK_AND_RETURN_RET_LOG(avMetadataHelper != nullptr, 0, "avMetadataHelper is nullptr");
    int64_t fileLength = static_cast<int64_t>(st.st_size);
    ret = avMetadataHelper->SetSource(fd, 0, fileLength, AV_META_USAGE_META_ONLY);
    close(fd);
    if (ret != 0) {
        RINGTONE_ERR_LOG("av set source is err");
        return E_FAIL;
    }

    std::shared_ptr<Meta> avMeta = avMetadataHelper->GetAVMetadata();
    CHECK_AND_RETURN_RET_LOG(avMeta != nullptr, 0, "avMeta is nullptr");
    if (!avMeta->GetData(Tag::MEDIA_TITLE, fileInfo.title) || fileInfo.title.empty()) {
        RINGTONE_ERR_LOG("get file title fail");
        return E_FAIL;
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
    dualFwkConf.messageSub1 = confLoader.GetConf("messageSub1");
    dualFwkConf.notificationSoundPath = confLoader.GetConf("notification_sound_path");
    dualFwkConf.alarmAlertPath = confLoader.GetConf("alarm_alert_path");

    return E_OK;
}

std::string CustomisedToneProcessor::ConvertCustomisedAudioPath(const std::string &ringtonePath)
{
    if (ringtonePath.empty()) {
        RINGTONE_ERR_LOG("customised audio path is empty");
        return "";
    }

    std::string::size_type pos = ringtonePath.find(RINGTONE_EXTERNAL_BASE_PATH);
    if (pos == std::string::npos) {
        RINGTONE_ERR_LOG("customised audio file not in the directory: %{public}s", RINGTONE_EXTERNAL_BASE_PATH.c_str());
        return "";
    }

    std::string absPath = ringtonePath.substr(pos + RINGTONE_EXTERNAL_BASE_PATH.length());
    bool isMusicPath = ringtonePath.find(RINGTONE_EXTERNAL_MUSIC_PATH) != std::string::npos;
    std::string fileMgrAudioPath = "";
    std::string tmpPath;
    if (isMusicPath && PathToRealPath(FILE_MANAGER_UPDATE_BACKUP_PATH + absPath, tmpPath)) {
        fileMgrAudioPath = FILE_MANAGER_UPDATE_BACKUP_PATH + absPath;
        RINGTONE_INFO_LOG("convert %{public}s to %{public}s", ringtonePath.c_str(), fileMgrAudioPath.c_str());
        return fileMgrAudioPath;
    }

    if (!isMusicPath && PathToRealPath(FILE_MANAGER_BASE_PATH + absPath, tmpPath)) {
        fileMgrAudioPath = FILE_MANAGER_BASE_PATH + absPath;
        RINGTONE_INFO_LOG("convert %{public}s to %{public}s", ringtonePath.c_str(), fileMgrAudioPath.c_str());
        return fileMgrAudioPath;
    }

    if (!isMusicPath && PathToRealPath(FILE_MANAGER_UPDATE_BACKUP_PATH + absPath, tmpPath)) {
        fileMgrAudioPath = FILE_MANAGER_UPDATE_BACKUP_PATH + absPath;
        RINGTONE_INFO_LOG("convert %{public}s to %{public}s", ringtonePath.c_str(), fileMgrAudioPath.c_str());
        return fileMgrAudioPath;
    }

    if (isMusicPath && PathToRealPath(FILE_MANAGER_BASE_PATH + absPath, tmpPath)) {
        fileMgrAudioPath = FILE_MANAGER_BASE_PATH + absPath;
        RINGTONE_INFO_LOG("convert %{public}s to %{public}s", ringtonePath.c_str(), fileMgrAudioPath.c_str());
        return fileMgrAudioPath;
    }

    RINGTONE_DEBUG_LOG("No rule can convert ringtonePath: %{public}s", ringtonePath.c_str());
    return fileMgrAudioPath;
}

int32_t CustomisedToneProcessor::BuildFileInfo(const std::string &dualFilePath, int32_t toneType, int32_t ringtoneType,
    int32_t shotToneType, std::vector<FileInfo> &fileInfos)
{
    if (dualFilePath.empty()) {
        return E_FAIL;
    }

    std::string customisedAudioPath = ConvertCustomisedAudioPath(dualFilePath);
    if (customisedAudioPath.empty()) {
        return E_FAIL;
    }

    FileInfo fileInfo;
    fileInfo.toneType = toneType;
    fileInfo.data = customisedAudioPath;
    fileInfo.displayName = RingtoneFileUtils::GetFileNameFromPath(fileInfo.data);
    fileInfo.ringToneType = ringtoneType;
    fileInfo.shotToneType = shotToneType;
    fileInfo.sourceType = SOURCE_TYPE_CUSTOMISED;
    fileInfo.mediaType = RINGTONE_MEDIA_TYPE_AUDIO;

    GetFileTitleAndDuration(customisedAudioPath, fileInfo);
    std::string extension = RingtoneScannerUtils::GetFileExtension(customisedAudioPath);
    fileInfo.mimeType = RingtoneMimeTypeUtils::GetMimeTypeFromExtension(extension);

    struct stat statInfo {};
    if (stat(customisedAudioPath.c_str(), &statInfo) != 0) {
        RINGTONE_ERR_LOG("fail to get file %{private}s statInfo, err: %{public}d", customisedAudioPath.c_str(), errno);
        return E_FAIL;
    }

    fileInfo.size = statInfo.st_size;
    fileInfos.emplace_back(fileInfo);
    return E_OK;
}

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