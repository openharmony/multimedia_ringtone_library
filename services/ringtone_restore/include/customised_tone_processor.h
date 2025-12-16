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
#ifndef CUSTOMISED_TONE_PROCESSOR_H
#define CUSTOMISED_TONE_PROCESSOR_H

#include <cstdint>
#include <string>

#include "dualfwk_conf_loader.h"
#ifdef USE_MEDIA_LIBRARY
#include "media_library_manager.h"
#endif

namespace OHOS {
namespace Media {
const std::string RINGTONE_EXTERNAL_BASE_PATH = "/storage/emulated/";
const std::string RINGTONE_EXTERNAL_SUB_DIR_MUSIC = "Music/";
const std::string FILE_MANAGER_BASE_PATH = "/storage/media/";
const std::string FILE_MANAGER_SUB_DIR_DOCS = "local/files/Docs/";
const std::string FILE_MANAGER_SUB_DIR_UPDATEBACKUP = FILE_MANAGER_SUB_DIR_DOCS + "UpdateBackup/";

constexpr int STORAGE_MANAGER_MANAGER_ID = 5003;
static constexpr uint64_t MAX_TONE_FILE_SIZE = 200 * 1024 * 1024;

class CustomisedToneProcessor {
public:
    CustomisedToneProcessor();
    ~CustomisedToneProcessor() {};
    int32_t QueryFileMgrForFileInfo(std::map<std::string, std::shared_ptr<FileInfo>> &infoMap);

private:
    int32_t GetCustomisedAudioPath(DualFwkConf &dualFwkConf);
    std::string ConvertCustomisedAudioPath(const std::string &ringtonePath);
    int32_t BuildFileInfo(const std::string &dualFilePath, int32_t toneType, int32_t ringtoneType,
        int32_t shotToneType, std::vector<FileInfo> &fileInfo);
    std::vector<FileInfo> BuildFileInfos();
    std::string GetNewUri(int32_t toneType, const std::string &oldUri);
#ifdef USE_MEDIA_LIBRARY
    std::string GetFileAssetPathByUri(const std::string &uri);
    MediaLibraryManager *manager_ = nullptr;
#endif
};
} // namespace Media
} // namespace OHOS
#endif // CUSTOMISED_TONE_PROCESSOR_H