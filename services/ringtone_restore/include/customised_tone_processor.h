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

namespace OHOS {
namespace Media {
const std::string RINGTONE_EXTERNAL_BASE_PATH = "/storage/emulated/0";
const std::string RINGTONE_EXTERNAL_MUSIC_PATH = RINGTONE_EXTERNAL_BASE_PATH + "/Music";
const std::string FILE_MANAGER_BASE_PATH = "/storage/media/local/files/Docs";
const std::string FILE_MANAGER_UPDATE_BACKUP_PATH = FILE_MANAGER_BASE_PATH + "/UpdateBackup";

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
};
} // namespace Media
} // namespace OHOS
#endif // CUSTOMISED_TONE_PROCESSOR_H